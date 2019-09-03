### 内核中的传输控制块
各协议族传输层使用各自的传输控制块存放套接字所要求的信息。比较重要的有IP地址对（远端和本地IP地址）和端口对（远端和本地端口号）

#### sock_common
所有传输控制块的共有成员。
```
/// @file include/net/sock.h
  1 /*
  2  * INET     An implementation of the TCP/IP protocol suite for the LINUX
  3  *      operating system.  INET is implemented using the  BSD Socket
  4  *      interface as the means of communication with the user level.
  5  *
  6  *      Definitions for the AF_INET socket handler.
132 /**
154  *  This is the minimal network layer representation of sockets, the header
155  *  for struct sock and struct inet_timewait_sock.
156  */
157 struct sock_common {
158     /* skc_daddr and skc_rcv_saddr must be grouped on a 8 bytes aligned
159      * address on 64bit arches : cf INET_MATCH()
160      */
161     union { // IP地址
162         __addrpair             skc_addrpair;       // IP 地址对
163         struct {
164             __be32             skc_daddr;          // 远端 IP 地址
165             __be32             skc_rcv_saddr;      // 本地 IP 地址
166         };
167     };
168     union  { // 查找哈希值
169         unsigned int           skc_hash;           // 与各种协议查找表一起使用的哈希值
170         __u16                  skc_u16hashes[2];   // DUP 查找表的两个哈希值
171     };
172     /* skc_dport && skc_num must be grouped as well */
173     union {
174         __portpair             skc_portpair;       // 端口对
175         struct {
176             __be16             skc_dport;          // 原端端口号，inet_dport/tw_dport使用
177             __u16              skc_num;            // 本地端口号，inet_num/tw_num使用
178         };
179     };
180 
181     unsigned short              skc_family;        // 协议族
182     volatile unsigned char      skc_state;         // TCP 状态
183     unsigned char               skc_reuse:4;       // 保存 SO_REUSEADDR 设置
184     unsigned char               skc_reuseport:4;   // 保存 SO_REUSEPORT 设置
185     int                         skc_bound_dev_if;  // 如果不为 0，代表绑定的 device 序列号
186     union {
187         struct hlist_node       skc_bind_node;     // 用于绑定协议查找表
188         struct hlist_nulls_node skc_portaddr_node; // 用于绑定 UDP 的查找表
189     };
190     struct proto               *skc_prot;          // 绑定各自的接口（函数）
191 #ifdef CONFIG_NET_NS
192     struct net                 *skc_net;           // 所属网络命名空间
193 #endif
194 
195 #if IS_ENABLED(CONFIG_IPV6)
196     struct in6_addr             skc_v6_daddr;      // IPv6 远端地址
197     struct in6_addr             skc_v6_rcv_saddr;  // IPv6 本地地址
198 #endif
199 
200     /*
201      * fields between dontcopy_begin/dontcopy_end
202      * are not copied in sock_copy()
203      */
204     /* private: */
205     int                         skc_dontcopy_begin[0];
206     /* public: */
207     union {
208         struct hlist_node       skc_node;
209         struct hlist_nulls_node skc_nulls_node;
210     };
211     int                         skc_tx_queue_mapping; // tx queue 值？？
212     atomic_t                    skc_refcnt;           // 引用计数
213     /* private: */
214     int                         skc_dontcopy_end[0];
215     /* public: */
216 };
```

#### sock
比较通用的网络层描述块，构成传输控制块的基础，与具体的协议无关。添加了接收队列和发送队列，接受队列和发送队列的大小。以及状态变化、可读、可写等的回调
```
/// @file include/net/sock.h
219 /**
220   * struct sock - network layer representation of sockets
295  */
296 struct sock { 
297     /*
298      * Now struct inet_timewait_sock also uses sock_common, so please just
299      * don't add nothing before this first member (__sk_common) --acme
300      */
301     struct sock_common      __sk_common;
302 #define sk_node             __sk_common.skc_node // 重命名
303 #define sk_nulls_node       __sk_common.skc_nulls_node
304 #define sk_refcnt           __sk_common.skc_refcnt
305 #define sk_tx_queue_mapping __sk_common.skc_tx_queue_mapping
306 
307 #define sk_dontcopy_begin   __sk_common.skc_dontcopy_begin
308 #define sk_dontcopy_end     __sk_common.skc_dontcopy_end
309 #define sk_hash             __sk_common.skc_hash
310 #define sk_portpair         __sk_common.skc_portpair
311 #define sk_num              __sk_common.skc_num
312 #define sk_dport            __sk_common.skc_dport
313 #define sk_addrpair         __sk_common.skc_addrpair
314 #define sk_daddr            __sk_common.skc_daddr
315 #define sk_rcv_saddr        __sk_common.skc_rcv_saddr
316 #define sk_family           __sk_common.skc_family
317 #define sk_state            __sk_common.skc_state
318 #define sk_reuse            __sk_common.skc_reuse
319 #define sk_reuseport        __sk_common.skc_reuseport
320 #define sk_bound_dev_if     __sk_common.skc_bound_dev_if
321 #define sk_bind_node        __sk_common.skc_bind_node
322 #define sk_prot             __sk_common.skc_prot
323 #define sk_net              __sk_common.skc_net
324 #define sk_v6_daddr         __sk_common.skc_v6_daddr
325 #define sk_v6_rcv_saddr     __sk_common.skc_v6_rcv_saddr
326 
327     socket_lock_t             sk_lock;            // 用于同步
328     struct sk_buff_head       sk_receive_queue;   // 接收队列
329     /*
330      * The backlog queue is special, it is always used with
331      * the per-socket spinlock held and requires low latency
332      * access. Therefore we special case it's implementation.
333      * Note : rmem_alloc is in this structure to fill a hole
334      * on 64bit arches, not because its logically part of
335      * backlog.
336      */
337     struct {
338         atomic_t              rmem_alloc;
339         int                   len;
340         struct sk_buff       *head;
341         struct sk_buff       *tail;
342     } sk_backlog; // 后备队列
343 #define sk_rmem_alloc sk_backlog.rmem_alloc
344     int                       sk_forward_alloc;
345 #ifdef CONFIG_RPS
346     __u32                     sk_rxhash;
347 #endif
348 #ifdef CONFIG_NET_RX_BUSY_POLL
349     unsigned int              sk_napi_id;
350     unsigned int              sk_ll_usec;
351 #endif
352     atomic_t                  sk_drops;
353     int                       sk_rcvbuf;          // 接收缓冲区大小
354 
355     struct sk_filter __rcu   *sk_filter;
356     struct socket_wq __rcu   *sk_wq;
357 
358 #ifdef CONFIG_NET_DMA
359     struct sk_buff_head       sk_async_wait_queue; // DMA
360 #endif
361 
362 #ifdef CONFIG_XFRM
363     struct xfrm_policy       *sk_policy[2];
364 #endif
365     unsigned long             sk_flags;           // SO_LINGER, SO_BROADCAST, SO_KEEPALIVE, 
                                                      // SO_OOBINLINE, SO_TIMESTAMPING
366     struct dst_entry         *sk_rx_dst;
367     struct dst_entry __rcu   *sk_dst_cache;       // 目的路由缓存
368     spinlock_t                sk_dst_lock;        // 目的路由缓存操作锁
369     atomic_t                  sk_wmem_alloc;      // 为发送而分配SKB的总大小
370     atomic_t                  sk_omem_alloc;      // 分配辅助缓冲区的总大小
371     int                       sk_sndbuf;          // 发送缓冲区大小
372     struct sk_buff_head       sk_write_queue;     // 发送队列
373     kmemcheck_bitfield_begin(flags);
374     unsigned int              sk_shutdown  : 2,   // RCV_SHUTDOWN
                                                      // SEND_SHUTDOWN
                                                      // SHUTDOWN_MASK
375                               sk_no_check_tx : 1, // SO_NO_CHECK设置
376                               sk_no_check_rx : 1,
377                               sk_userlocks : 4,   // SOCK_SNDBUF_LOCK：设置了发送缓冲区大小
                                                      // SOCK_RCVBUF_LOCK：设置了接受缓冲区大小
                                                      // SOCK_BINDADDR_LOCK：绑定了本地端口
                                                      // SOCK_BINDPORT_LOCK：绑定了本地地址
378                               sk_protocol  : 8,   // 所属协议
379                               sk_type      : 16;  // 套接字类型
380 #define SK_PROTOCOL_MAX U8_MAX
381     kmemcheck_bitfield_end(flags);
382     int                       sk_wmem_queued;     // 发送队列中数据总长度
383     gfp_t                     sk_allocation;
384     u32                       sk_pacing_rate;
385     u32                       sk_max_pacing_rate;
386     netdev_features_t         sk_route_caps;
387     netdev_features_t         sk_route_nocaps;
388     int                       sk_gso_type;
389     unsigned int              sk_gso_max_size;
390     u16                       sk_gso_max_segs;
391     int                       sk_rcvlowat;        // 接收低水位 SO_RCVLOWAT 设置
392     unsigned long             sk_lingertime;      // SO_LINGER 设置
393     struct sk_buff_head       sk_error_queue;     // 错误队列
394     struct proto             *sk_prot_creator;
395     rwlock_t                  sk_callback_lock;
396     int                       sk_err,
397                               sk_err_soft;
398     unsigned short            sk_ack_backlog;     // 当前已建立的连接数
399     unsigned short            sk_max_ack_backlog; // 已建立连接数的上限
400     __u32                     sk_priority;        // SO_PRIORITY 设置
401 #if IS_ENABLED(CONFIG_CGROUP_NET_PRIO)
402     __u32                     sk_cgrp_prioidx;
403 #endif
404     struct pid               *sk_peer_pid;
405     const struct cred        *sk_peer_cred;
406     long                      sk_rcvtimeo;        // 接收超时，SO_RCVTIMEO 设置
407     long                      sk_sndtimeo;        // 发送超时，SO_SNDTIMEO 设置
408     void                     *sk_protinfo;        // 存放私有数据
409     struct timer_list         sk_timer;           // 计时器链表
410     ktime_t                   sk_stamp;
411     struct socket            *sk_socket;
412     void                     *sk_user_data;
413     struct page_frag          sk_frag;
414     struct sk_buff           *sk_send_head;      // 发送队列
415     __s32                     sk_peek_off;       // 当前 peek_offset 值
416     int                       sk_write_pending;  // 标识有数据即将写入套接字
417 #ifdef CONFIG_SECURITY
418     void                     *sk_security;
419 #endif
420     __u32                    sk_mark;
421     u32                      sk_classid;
422     struct cg_proto          *sk_cgrp;
423     void            (*sk_state_change)(struct sock *sk); // 状态变化回调（唤醒等待进程）
424     void            (*sk_data_ready)(struct sock *sk);   // 可读回调
425     void            (*sk_write_space)(struct sock *sk);  // 可写回调
426     void            (*sk_error_report)(struct sock *sk); // 出错回调
427     int             (*sk_backlog_rcv)(struct sock *sk,
428                           struct sk_buff *skb);          // 后备队列处理回调
429     void            (*sk_destruct)(struct sock *sk);     // 析构函数回调，释放 sock 时调用
430 };
```

#### inet_sock
比较通用的 IPv4 协议簇描述块，包含 IPv4 协议簇基础传输层，即 UDP、TCP 以及 RAW 控制块共有的信息。添加了 IPv4 协议专用的属性，比如 TTL
```
/// @file include/net/inet_sock.h
132 /** struct inet_sock - representation of INET sockets
151  */
152 struct inet_sock {
153     /* sk and pinet6 has to be the first two members of inet_sock */
154     struct sock                  sk;
155 #if IS_ENABLED(CONFIG_IPV6)
156     struct ipv6_pinfo           *pinet6;
157 #endif
158     /* Socket demultiplex comparisons on incoming packets. */
159 #define inet_daddr               sk.__sk_common.skc_daddr     // 绑定的目的 IP 地址
160 #define inet_rcv_saddr           sk.__sk_common.skc_rcv_saddr // 绑定的本地 IP 地址
161 #define inet_dport               sk.__sk_common.skc_dport     // 目的端口号
162 #define inet_num                 sk.__sk_common.skc_num       // 本地端口号
163 
164     __be32                       inet_saddr;           // 发送的实际本地 IP 地址
165     __s16                        uc_ttl;               // 单播 TTL，默认-1        
166     __u16                        cmsg_flags;
167     __be16                       inet_sport;           // 发送的实际本地端口号
168     __u16                        inet_id;              // IP 首部标识 ID
169 
170     struct ip_options_rcu __rcu *inet_opt;             // IP 首部的选项指针
171     int                          rx_dst_ifindex;
172     __u8                         tos;                  // IP 首部 TOS（区分服务）
173     __u8                         min_ttl;
174     __u8                         mc_ttl;               // 多播 TTL
175     __u8                         pmtudisc;
176     __u8                         recverr:1,            // IP_RECVERR 设置
177                                  is_icsk:1,            // 是否是 inet_connection_sock
178                                  freebind:1,           // IP_FREEBIND设 置
179                                  hdrincl:1,
180                                  mc_loop:1,            // 组播是否发向回路
181                                  transparent:1,
182                                  mc_all:1,
183                                  nodefrag:1;
184     __u8                         rcv_tos;
185     int                          uc_index;             // 单播的网络设备索引号，为0表示任意
186     int                          mc_index;             // 多播的网络设备索引号，为0表示任意
187     __be32                       mc_addr;              // 多播源地址
188     struct ip_mc_socklist __rcu *mc_list;              // 加入的组播组
189     struct inet_cork_full        cork;
190 };
191 
```

#### inet_connection_sock
面向连接特性的描述块，在 inet_sock 的基础上增加了连接、确认和重传等成员
```
/// @file include/net/inet_connection_sock.h 
 68 /** inet_connection_sock - INET connection oriented sock
 87  */
 88 struct inet_connection_sock {
 89     /* inet_sock has to be the first member! */
 90     struct inet_sock           icsk_inet;
91     struct request_sock_queue   icsk_accept_queue; // 存放已建立的连接的队列
 92     struct inet_bind_bucket   *icsk_bind_hash;    // 执行绑定的本地端口信息
 93     unsigned long              icsk_timeout;      // 超时重传时间
 94     struct timer_list          icsk_retransmit_timer; // 超时重传计时器
 95     struct timer_list          icsk_delack_timer; // 延迟发送 ACK 计时器
 96     __u32                      icsk_rto;          // 超时重传时间，处置为 TCP_TIMEOUT_INIT
 97     __u32                      icsk_pmtu_cookie;  // 最后一次更新的最大传输单元（MTU）
 98     const struct tcp_congestion_ops          *icsk_ca_ops; // 指向某个拥塞控制算法
 99     const struct inet_connection_sock_af_ops *icsk_af_ops; // TCP 的一个操作接口集
100     unsigned int          (*icsk_sync_mss)(struct sock *sk, u32 pmtu); // 根据 PMTU 同步本地 MSS 函数指针
101     __u8                       icsk_ca_state;     // 拥塞控制状态
102     __u8                       icsk_retransmits;  // 超时重传次数
103     __u8                       icsk_pending;      // 标识预定的定时器事件
                                             // ICSK_TIME_RETRANS：重传计数器
                                             // ICSK_TIME_DACK：延迟ACK计时器
                                             // ICSK_TIME_PROBE0：零窗口探测计时器
                                             // ICSK_TIME_KEEPOPEN：保活计时器
104     __u8                       icsk_backoff;      // 计时器的指数退避算法的指数
105     __u8                       icsk_syn_retries;  // 最多重传 SYN 的次数，TCP_SYNCNT 设置
106     __u8                       icsk_probes_out;   // 零窗口探测或保活时发送但为确认的 TCP 分节数
107     __u16                      icsk_ext_hdr_len;  // IP 首部选项长度
108     struct { // 延迟确认控制块
109         __u8                   pending;           // ACK状态
                                             // ICSK_ACK_SCHED：
                                             // ICSK_ACK_TIMER：
                                             // ICSK_ACK_PUSHED：
                                             // ICSK_ACK_PUSHED2：
110         __u8                   quick;             // ？？
111         __u8                   pingpong;          // 快速确认（0）或延迟确认（1）
112         __u8                   blocked;  
113         __u32                  ato;               // 用来计算延迟 ACK 的估值
114         unsigned long          timeout;           // 当前的延迟 ACK 超时时间
115         __u32                  lrcvtime;          // 最后一次收到数据的时间
116         __u16                  last_seg_size;     // 最后收到的数据的长度
117         __u16                  rcv_mss;           // 由最近接收到的段计算出的 MSS，用来确定是否延迟 ACK
118     } icsk_ack; 
119     struct { // 最大传输单元（MTU）发现相关的数据
120         int                    enabled;           // 是否启用
121 
122         /* Range of MTUs to search */
123         int                    search_high;       // 上限
124         int                    search_low;        // 下限
125 
126         /* Information on the current probe. */
127         int                    probe_size;        // 当前探测大小
128     } icsk_mtup;
129     u32                        icsk_ca_priv[16];  // 拥塞控制算法是私有参数
130     u32                        icsk_user_timeout;
131 #define ICSK_CA_PRIV_SIZE   (16 * sizeof(u32))
132 };
```

#### tcp_sock
```
/// @file include/linux/tcp.h
138 struct tcp_sock {
139     /* inet_connection_sock has to be the first member of tcp_sock */
140     struct inet_connection_sock inet_conn;
141     u16 tcp_header_len; // 首部大小
142     u16 xmit_size_goal_segs; /* Goal for segmenting output packets */
148     __be32  pred_flags;
155     u32 rcv_nxt;    // 期待收到的下一个 TCP 序号
156     u32 copied_seq; /* Head of yet unread data      */
157     u32 rcv_wup;    /* rcv_nxt on last window update sent   */
158     u32 snd_nxt;    // 等待发送的下一个 TCP 序号
159 
160     u32 snd_una;    // 最早一个未确认的序号
161     u32 snd_sml;    // 最近发送的小包的最后一个字节序号，用于判断是否启用 Nagle 算法
162     u32 rcv_tstamp; // 最近一次收到 ACK 的时间，用于 TCP 保活
163     u32 lsndtime;   // 最近一次发送数据的时间，用于拥塞窗口的设置
164 
165     u32 tsoffset;   /* timestamp offset */
166 
167     struct list_head tsq_node; /* anchor in tsq_tasklet.head list */
168     unsigned long   tsq_flags;
169 
170     /* Data for direct copy to user */
171     struct { // 控制赋值数据到用户进程的控制结构
172         struct sk_buff_head prequeue;
173         struct task_struct  *task;
174         struct iovec        *iov;
175         int         memory; // prequeue 当前消耗的内存
176         int         len; // 用户缓存大小
177 #ifdef CONFIG_NET_DMA
178         /* members for async copy */
179         struct dma_chan     *dma_chan;
180         int         wakeup;
181         struct dma_pinned_list  *pinned_list;
182         dma_cookie_t        dma_cookie;
183 #endif
184     } ucopy;
185 
186     u32 snd_wl1;    // 记录更新发送窗口的那个 ACK 的序号，用于判断是否需要更新窗口大小
187     u32 snd_wnd;    // 接收方提供的接收窗口大小
188     u32 max_window; // 接收方通告过的接收窗口值最大值
189     u32 mss_cache;  // 发送方当前有效 MSS，SOL_TCP 设置
190 
191     u32 window_clamp;   // 滑动窗口最大值
192     u32 rcv_ssthresh;   // 当前接收窗口大小的阈值
193 
194     u16 advmss;     /* Advertised MSS           */
195     u8  unused;
196     u8  nonagle     : 4,/* Disable Nagle algorithm?             */
197         thin_lto    : 1,/* Use linear timeouts for thin streams */
198         thin_dupack : 1,/* Fast retransmit on first dupack      */
199         repair      : 1,
200         frto        : 1;/* F-RTO (RFC5682) activated in CA_Loss */
201     u8  repair_queue;
202     u8  do_early_retrans:1,/* Enable RFC5827 early-retransmit  */
203         syn_data:1, /* SYN includes data */
204         syn_fastopen:1, /* SYN includes Fast Open option */
205         syn_data_acked:1,/* data in SYN is acked by SYN-ACK */
206         is_cwnd_limited:1;/* forward progress limited by snd_cwnd? */
207     u32 tlp_high_seq;   /* snd_nxt at the time of TLP retransmit. */
208 
209 /* RTT measurement */
210     u32 srtt_us;    /* smoothed round trip time << 3 in usecs */
211     u32 mdev_us;    /* medium deviation         */
212     u32 mdev_max_us;    /* maximal mdev for the last rtt period */
213     u32 rttvar_us;  /* smoothed mdev_max            */
214     u32 rtt_seq;    /* sequence number to update rttvar */
215 
216     u32 packets_out;    /* Packets which are "in flight"    */
217     u32 retrans_out;    /* Retransmitted packets out        */
218     u32 max_packets_out;  /* max packets_out in last window */
219     u32 max_packets_seq;  /* right edge of max_packets_out flight */
220 
221     u16 urg_data;   /* Saved octet of OOB data and control flags */
222     u8  ecn_flags;  /* ECN status bits.         */
223     u8  reordering; /* Packet reordering metric.        */
224     u32 snd_up;     /* Urgent pointer       */
225 
226     u8  keepalive_probes; /* num of allowed keep alive probes   */
227 /*
228  *      Options received (usually on last packet, some only on SYN packets).
229  */
230     struct tcp_options_received rx_opt;
231 
232 /*
233  *  Slow start and congestion control (see also Nagle, and Karn & Partridge)
234  */
235     u32 snd_ssthresh;   /* Slow start size threshold        */
236     u32 snd_cwnd;   /* Sending congestion window        */
237     u32 snd_cwnd_cnt;   /* Linear increase counter      */
238     u32 snd_cwnd_clamp; /* Do not allow snd_cwnd to grow above this */
239     u32 snd_cwnd_used;
240     u32 snd_cwnd_stamp;
241     u32 prior_cwnd; /* Congestion window at start of Recovery. */
242     u32 prr_delivered;  /* Number of newly delivered packets to
243                  * receiver in Recovery. */
244     u32 prr_out;    /* Total number of pkts sent during Recovery. */
245 
246     u32 rcv_wnd;    /* Current receiver window      */
247     u32 write_seq;  /* Tail(+1) of data held in tcp send buffer */
248     u32 notsent_lowat;  /* TCP_NOTSENT_LOWAT */
249     u32 pushed_seq; /* Last pushed seq, required to talk to windows */
250     u32 lost_out;   /* Lost packets         */
251     u32 sacked_out; /* SACK'd packets           */
252     u32 fackets_out;    /* FACK'd packets           */
253     u32 tso_deferred;
254 
255     /* from STCP, retrans queue hinting */
256     struct sk_buff* lost_skb_hint;
257     struct sk_buff *retransmit_skb_hint;
258 
259     /* OOO segments go in this list. Note that socket lock must be held,
260      * as we do not use sk_buff_head lock.
261      */
262     struct sk_buff_head out_of_order_queue;
263 
264     /* SACKs data, these 2 need to be together (see tcp_options_write) */
265     struct tcp_sack_block duplicate_sack[1]; /* D-SACK block */
266     struct tcp_sack_block selective_acks[4]; /* The SACKS themselves*/
267 
268     struct tcp_sack_block recv_sack_cache[4];
269 
270     struct sk_buff *highest_sack;   /* skb just after the highest
271                      * skb with SACKed bit set
272                      * (validity guaranteed only if
273                      * sacked_out > 0)
274                      */
275 
276     int     lost_cnt_hint;
277     u32     retransmit_high;    /* L-bits may be on up to this seqno */
278 
279     u32 lost_retrans_low;   /* Sent seq after any rxmit (lowest) */
280 
281     u32 prior_ssthresh; /* ssthresh saved at recovery start */
282     u32 high_seq;   /* snd_nxt at onset of congestion   */
283 
284     u32 retrans_stamp;  /* Timestamp of the last retransmit,
285                  * also used in SYN-SENT to remember stamp of
286                  * the first SYN. */
287     u32 undo_marker;    /* tracking retrans started here. */
288     int undo_retrans;   /* number of undoable retransmissions. */
289     u32 total_retrans;  /* Total retransmits for entire connection */
290 
291     u32 urg_seq;    /* Seq of received urgent pointer */
292     unsigned int        keepalive_time;   /* time before keep alive takes place */
293     unsigned int        keepalive_intvl;  /* time interval between keep alive probes */
294 
295     int         linger2;
296 
297 /* Receiver side RTT estimation */
298     struct {
299         u32 rtt;
300         u32 seq;
301         u32 time;
302     } rcv_rtt_est;
303 
304 /* Receiver queue space */
305     struct {
306         int space;
307         u32 seq;
308         u32 time;
309     } rcvq_space;
310 
311 /* TCP-specific MTU probe information. */
312     struct {
313         u32       probe_seq_start;
314         u32       probe_seq_end;
315     } mtu_probe;
316     u32 mtu_info; /* We received an ICMP_FRAG_NEEDED / ICMPV6_PKT_TOOBIG
317                * while socket was owned by user.
318                */
319 
320 #ifdef CONFIG_TCP_MD5SIG
321 /* TCP AF-Specific parts; only used by MD5 Signature support so far */
322     const struct tcp_sock_af_ops    *af_specific;
323 
324 /* TCP MD5 Signature Option information */
325     struct tcp_md5sig_info  __rcu *md5sig_info;
326 #endif
327 
328 /* TCP fastopen related information */
329     struct tcp_fastopen_request *fastopen_req;
330     /* fastopen_rsk points to request_sock that resulted in this big
331      * socket. Used to retransmit SYNACKs etc.
332      */
333     struct request_sock *fastopen_rsk;
334 };
```

#### upd_sock
```
/// @file include/linux/udp.h
42 struct udp_sock {
43     /* inet_sock has to be the first member */
44     struct inet_sock inet;
45 #define udp_port_hash       inet.sk.__sk_common.skc_u16hashes[0]
46 #define udp_portaddr_hash   inet.sk.__sk_common.skc_u16hashes[1]
47 #define udp_portaddr_node   inet.sk.__sk_common.skc_portaddr_node
48     int      pending;   /* Any pending frames ? */
49     unsigned int     corkflag;  /* Cork is required */
50     __u8         encap_type;    /* Is this an Encapsulation socket? */
51     unsigned char    no_check6_tx:1,/* Send zero UDP6 checksums on TX? */
52              no_check6_rx:1;/* Allow zero UDP6 checksums on RX? */
53     /*
54      * Following member retains the information to create a UDP header
55      * when the socket is uncorked.
56      */
57     __u16        len;       /* total length of pending frames */
58     /*
59      * Fields specific to UDP-Lite.
60      */
61     __u16        pcslen;
62     __u16        pcrlen;
63 /* indicator bits used by pcflag: */
64 #define UDPLITE_BIT      0x1        /* set by udplite proto init function */
65 #define UDPLITE_SEND_CC  0x2        /* set via udplite setsockopt         */
66 #define UDPLITE_RECV_CC  0x4        /* set via udplite setsocktopt        */
67     __u8         pcflag;        /* marks socket as UDP-Lite if > 0    */
68     __u8         unused[3];
69     /*
70      * For encapsulation sockets.
71      */
72     int (*encap_rcv)(struct sock *sk, struct sk_buff *skb);
73     void (*encap_destroy)(struct sock *sk);
74 };
```

#### inet_timewait_sock
```
/// @file include/net/inet_timewait_sock.h
 97 /*
 98  * This is a TIME_WAIT sock. It works around the memory consumption
 99  * problems of sockets in such a state on heavily loaded servers, but
100  * without violating the protocol specification.
101  */
102 struct inet_timewait_sock {
103     /*
104      * Now struct sock also uses sock_common, so please just
105      * don't add nothing before this first member (__tw_common) --acme
106      */
107     struct sock_common  __tw_common;
108 #define tw_family       __tw_common.skc_family
109 #define tw_state        __tw_common.skc_state
110 #define tw_reuse        __tw_common.skc_reuse
111 #define tw_reuseport        __tw_common.skc_reuseport
112 #define tw_bound_dev_if     __tw_common.skc_bound_dev_if
113 #define tw_node         __tw_common.skc_nulls_node
114 #define tw_bind_node        __tw_common.skc_bind_node
115 #define tw_refcnt       __tw_common.skc_refcnt
116 #define tw_hash         __tw_common.skc_hash
117 #define tw_prot         __tw_common.skc_prot
118 #define tw_net          __tw_common.skc_net
119 #define tw_daddr            __tw_common.skc_daddr
120 #define tw_v6_daddr     __tw_common.skc_v6_daddr
121 #define tw_rcv_saddr        __tw_common.skc_rcv_saddr
122 #define tw_v6_rcv_saddr     __tw_common.skc_v6_rcv_saddr
123 #define tw_dport        __tw_common.skc_dport
124 #define tw_num          __tw_common.skc_num
125 
126     int         tw_timeout;
127     volatile unsigned char  tw_substate;
128     unsigned char       tw_rcv_wscale;
129 
130     /* Socket demultiplex comparisons on incoming packets. */
131     /* these three are in inet_sock */
132     __be16          tw_sport;
133     kmemcheck_bitfield_begin(flags);
134     /* And these are ours. */
135     unsigned int        tw_ipv6only     : 1,
136                 tw_transparent  : 1,
137                 tw_flowlabel    : 20,
138                 tw_pad      : 2,    /* 2 bits hole */
139                 tw_tos      : 8;
140     kmemcheck_bitfield_end(flags);
141     u32         tw_ttd;
142     struct inet_bind_bucket *tw_tb;
143     struct hlist_node   tw_death_node;
144 };
```

#### request_sock
```
/// @file include/net/request_sock.h
48 /* struct request_sock - mini sock to represent a connection request
49  */
50 struct request_sock {
51     struct sock_common      __req_common;
52     struct request_sock     *dl_next;
53     u16             mss;
54     u8              num_retrans; /* number of retransmits */
55     u8              cookie_ts:1; /* syncookie: encode tcpopts in timestamp */
56     u8              num_timeout:7; /* number of timeouts */
57     /* The following two fields can be easily recomputed I think -AK */
58     u32             window_clamp; /* window clamp at creation time */
59     u32             rcv_wnd;      /* rcv_wnd offered first time */
60     u32             ts_recent;
61     unsigned long           expires;
62     const struct request_sock_ops   *rsk_ops;
63     struct sock         *sk;
64     u32             secid;
65     u32             peer_secid;
66 };
```

#### inet_request_sock
```
/// @file include/net/inet_sock.h
71 struct inet_request_sock {
72     struct request_sock req;
73 #define ir_loc_addr     req.__req_common.skc_rcv_saddr
74 #define ir_rmt_addr     req.__req_common.skc_daddr
75 #define ir_num          req.__req_common.skc_num
76 #define ir_rmt_port     req.__req_common.skc_dport
77 #define ir_v6_rmt_addr      req.__req_common.skc_v6_daddr
78 #define ir_v6_loc_addr      req.__req_common.skc_v6_rcv_saddr
79 #define ir_iif          req.__req_common.skc_bound_dev_if
80 
81     kmemcheck_bitfield_begin(flags);
82     u16         snd_wscale : 4,
83                 rcv_wscale : 4,
84                 tstamp_ok  : 1,
85                 sack_ok    : 1,
86                 wscale_ok  : 1,
87                 ecn_ok     : 1,
88                 acked      : 1,
89                 no_srccheck: 1;
90     kmemcheck_bitfield_end(flags);
91     struct ip_options_rcu   *opt;
92     struct sk_buff      *pktopts;
93     u32                     ir_mark;
94 };
```

#### tcp_resuqst_sock
```
/// @file include/linux/tcp.h
117 struct tcp_request_sock {
118     struct inet_request_sock    req;
119 #ifdef CONFIG_TCP_MD5SIG
120     /* Only used by TCP MD5 Signature so far. */
121     const struct tcp_request_sock_ops *af_specific;
122 #endif
123     struct sock         *listener; /* needed for TFO */
124     u32             rcv_isn;
125     u32             snt_isn;
126     u32             snt_synack; /* synack sent time */
127     u32             rcv_nxt; /* the ack # by SYNACK. For
128                           * FastOpen it's the seq#
129                           * after data-in-SYN.
130                           */
131 };
```

#### tcp_timewait_sock
```
/// @file include/linux/tcp.h
352 struct tcp_timewait_sock {
353     struct inet_timewait_sock tw_sk;
354     u32           tw_rcv_nxt;
355     u32           tw_snd_nxt;
356     u32           tw_rcv_wnd;
357     u32           tw_ts_offset;
358     u32           tw_ts_recent;
359     long              tw_ts_recent_stamp;
360 #ifdef CONFIG_TCP_MD5SIG
361     struct tcp_md5sig_key     *tw_md5_key;
362 #endif
363 };
```

#### listen_sock
```
/// @file include/net/request_sock.h
 95 struct listen_sock {
 96     u8          max_qlen_log;
 97     u8          synflood_warned;
 98     /* 2 bytes hole, try to use */
 99     int         qlen;
100     int         qlen_young;
101     int         clock_hand;
102     u32         hash_rnd;
103     u32         nr_table_entries;
104     struct request_sock *syn_table[0];
105 };
```