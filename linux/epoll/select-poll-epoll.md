# `select`、`poll`和`epoll`的区别
`select`，`poll`，`epoll`都是IO多路复用的机制。I/O多路复用就是通过一种机制，一个进程可以监视多个描述符，一旦某个描述符就绪（一般是读就绪或写就绪），能够通知程序进行相应的读写操作。但`select`，`poll`，`epoll`本质上都是同步I/O，因为他们都需要在读写事件就绪后自己负责进行读写，也就是说这个读写过程是阻塞的，而异步I/O则无需自己负责进行读写，异步I/O的实现会负责把数据从内核拷贝到用户空间。

## `select`实现及其优缺点
`select`提供一种`fd_set`的数据结构，实际上是一个`long`类型的数组。数组每一位都能与已打开的文件描述符`fd`建立联系（`FD_SET`、`FD_CLR`），当调用`select`时，由内核遍历`fd_set`的内容，根据IO状态修改`fd_set`的内容，由此来通知描述符就绪。

### 函数接口
```
typedef struct {
  unsigned long fds_bits[__FD_SETSIZE / (8 * sizeof(long))]; /* 1024 */
}__kernel_fd_set;
typedef __kernel_fd_set fd_set;

#include <sys/select.h>
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *execeptfds,
           struct timeval *timeout);
void FD_ZERO(int fd, fd_set *set);
void FD_SET(int fd, fd_set *set);
void FD_CLR(int fd, fd_set *set);
int FD_ISSET(int fd, fd_set *set);
```
### 系统实现（Linux）
#### 调用关系
```
select(nfds, inp, outp, exp, timeout):
  SYSCALL_DEFINE5(select, nfds, inp, outp, exp, timeout):
    struct timespec *to = NULL;
    if(timeout) {/*copy and transfer from timeout to to */}
    core_sys_select(nfds, inp, outp, exp, to):
      do_select(...):
```
#### `core_sys_select`函数主要代码
```
#define FRONTEND_STACK_ALLOC	256
#define SELECT_STACK_ALLOC	FRONTEND_STACK_ALLOC
typedef struct {
  unsigned long *in, *out, *ex;
  unsigned long *res_in, *res_out, *res_ex;
} fd_set_bits;


int core_sys_select((int n, fd_set __user *inp, fd_set __user *outp,
                    fd_set __user *exp, s64 *timeout)) {
  fd_set_bits fds;
  long stack_fds[SELECT_STACK_ALLOC / sizeof(long)]; /* 32 */
  void *bits = stack_fds;
  unsigned int size = FD_BYTES(nfds);                /* 存储n个bit需要的字节数 */
  if(size > sizeof(stack_fds) / 6) {                 /* 6个bitmap */
    bits = kmalloc(6 * size, GFP_KERNEL);            /* 重新分配大小 */
  }
  fds.in      = bits;
  fds.out     = bits +   size;
  fds.ex      = bits + 2*size;
  fds.res_in  = bits + 3*size;
  fds.res_out = bits + 4*size;
  fds.res_ex  = bits + 5*size;
  
  if ((ret = get_fd_set(n, inp, fds.in)) || (ret = get_fd_set(n, outp, fds.out)) ||
      (ret = get_fd_set(n, exp, fds.ex)))  /* 将数组inp、outp、exp从用户空间拷贝到内核空间 */
    goto out;
  zero_fd_set(n, fds.res_in);              /* 返回空间清零 */
  zero_fd_set(n, fds.res_out);
  zero_fd_set(n, fds.res_ex);

  ret = do_select(n, &fds, end_time);      /* 主要是一遍一遍循环遍历每一个文件描述符，查询哪一个为就绪状态 */

  if (set_fd_set(n, inp, fds.res_in) || set_fd_set(n, outp, fds.res_out) ||
      set_fd_set(n, exp, fds.res_ex)) /* 将数组返回值从内核空间拷贝到用户空间 */
    ret = -EFAULT;

  return ret;
}
```
#### `do_select`原理
- 通过轮训文件描述符集中的文件描述符，检查描述符是否达到条件，若达到符合的相关条件则返回，否则轮训，但是当轮训的机制虽然是死循环，但是不是一直轮训，当内核轮询一遍文件描述符之后，会调用schedule_timeout函数挂起，等待fd设备或定时器来唤醒自己，然后再继续循环体看看哪些fd可用，以此提高效率。

### 优缺点
- 优点
  - 跨平台
- 缺点
  - 单个进程能够监视的文件描述符的数量存在最大限制，通常是1024。当然可以更改数量，但由于`select`采用轮询的方式扫描文件描述符，文件描述符数量越多，性能越差
  - 每次调用`select`，都需要把fd集合从用户空间拷贝到内核空间，在返回时，将返会数组从内核空间拷贝到用户空间
  - `select`返回的是含有整个监视的文件描述符，应用程序需要遍历整个数组才能发现哪些句柄发生了事件
  - 会（清空）修改传入的`fd_set`数组，每次都需要重新传入

## `poll`实现及其优缺点
`poll`和`select`类似，没有本质差别，管理多个描述符也是进行轮询，根据描述符的状态进行处理。但是`poll`没有最大描述符数量的限制，并且传入的`fds`在`poll`函数返回后不会清空，活动事件记录在`revents`成员中。

### 函数接口
```
struct pollfd {
  int fd;         /* file descriptor */
  short events;   /* requested events */
  short revents;  /* returned events */
};
typedef unsigned long int nfds_t;

#include <poll.h>
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```
### 系统实现（Linux）
#### 调用关系
```
poll(ufds, nfdsm, timout_msecs):
  SYSCALL_DEFINE3(poll, ufds, nfdsm, timout_msecs):
    do_sys_poll(ufds, nfds, to):
      do_poll(nfds, head, &table, end_time)
```
#### `do_sys_poll`函数实现
```
#define FRONTEND_STACK_ALLOC	256
#define POLL_STACK_ALLOC	FRONTEND_STACK_ALLOC
struct poll_list {
  struct poll_list *next;
  int len;
  struct pollfd entries[0];
};


int do_sys_poll(struct pollfd __user *ufds, unsigned int nfds,
		struct timespec *end_time) {
  struct poll_wqueues table;
  int err = -EFAULT, fdcount, len, size;
  long stack_pps[POLL_STACK_ALLOC/sizeof(long)];   /* 32 */
  struct poll_list *const head = (struct poll_list *)stack_pps;
  struct poll_list *walk = head;
  unsigned long todo = nfds;

  len = min_t(unsigned int, nfds, N_STACK_PPS);
  for (;;) {
    walk->next = NULL; walk->len = len;
    if (!len)
      break;
    if (copy_from_user(walk->entries, ufds + nfds-todo, 
            sizeof(struct pollfd) * walk->len))  /* 将数据从用户空间拷贝至内核空间 */
      goto out_fds;

    todo -= walk->len;
    if (!todo)
      break;

    len = min(todo, POLLFD_PER_PAGE);
    size = sizeof(struct poll_list) + sizeof(struct pollfd) * len;
    walk = walk->next = kmalloc(size, GFP_KERNEL);
    if (!walk) {
      err = -ENOMEM;
      goto out_fds;
    }
  }

  poll_initwait(&table);
  fdcount = do_poll(nfds, head, &table, end_time); /* 然后轮流调用所有fd对应的poll */
  poll_freewait(&table);

  for (walk = head; walk; walk = walk->next) {
    struct pollfd *fds = walk->entries;
    int j;

    for (j = 0; j < walk->len; j++, ufds++)
      if (__put_user(fds[j].revents, &ufds->revents)) /* 将返回数据从内核空间拷贝至用户空间 */
        goto out_fds;
  }
  err = fdcount;
out_fds:
  walk = head->next;
  while (walk) {
    struct poll_list *pos = walk;
    walk = walk->next;
    kfree(pos);
  }

  return err;
}
```
### 优缺点
- 优点
  - `select`会修改描述符，而`poll`不会；
  - `poll`的描述符类型使用链表实现，没有描述符数量的限制；
- 缺点
  - 每次调用`poll`，都需要把`pollfd`链表从用户空间拷贝到内核空间，在返回时，将返会数据从内核空间拷贝到用户空间
  - `poll`返回的是含有整个`pollfd`链表，应用程序需要遍历整个链表才能发现哪些句柄发生了事件

## [`epoll`实现及其优缺点](https://tqr.ink/2017/10/05/implementation-of-epoll/)
相对于`select`来说，`epoll`没有描述符个数限制，使用一个文件描述符管理多个描述符。调用`epoll_ctl`注册事件的时候将相关数据拷入内核，以后调用`epoll_wait`不会像`select`或`poll`那样，每次都从用户空间拷贝数据到内核空间。并且与`select`或`poll`返回所有事件不同的是，`epoll`返回的是处于就绪的事件的列表。此外`epoll`是基于事件驱动的，在所有添加事件会对应文件建立回调关系，也就是说，当相应的事件发生时会调用这个回调方法，它会将发生的事件添加到就绪链表中。

### 接口函数
```
typedef union epoll_data{
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event {
  uint32_t events;	/* Epoll events */
  epoll_data_t data;	/* User data variable */
} __EPOLL_PACKED;

#include <sys/epoll.h>
int epoll_create(int size);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```
### 系统实现
#### `epoll_create`函数实现
```
epoll_create(int size):
  SYSCALL_DEFINE1(epoll_create， size):
    sys_epoll_create1(0):

SYSCALL_DEFINE1(epoll_create1, int, flags) { /* open an eventpoll file discriptor */
  int error, fd;
  struct eventpoll *ep = NULL;
  struct file *file;

  error = ep_alloc(&ep); /* ep = kzalloc(sizeof(*ep), GFP_KERNEL); */
  fd = get_unused_fd_flags(O_RDWR | (flags & O_CLOEXEC));
  file = anon_inode_getfile("[eventpoll]", &eventpoll_fops, ep,
          O_RDWR | (flags & O_CLOEXEC));
  ep->file = file;
  fd_install(fd, file);
  return fd;
}
```
`epoll_create`系统调用完成了`fd`、`file`、`eventpoll`三个对象之间的关联，并将`fd`返回给用户态应用程序。每一个`fd`都会对应一个`struct eventpoll`类型的对象，用户通过`fd`可以将目标需要检测的事件添加到`eventpoll`中，对于`struct eventpoll`类型，其定义为:
```
struct eventpoll {
  spinlock_t lock;
  struct mutex mtx;
  wait_queue_head_t wq;
  wait_queue_head_t poll_wait;
  struct list_head rdllist;   /* 就绪的epitem */
  struct rb_root rbr;         /* 红黑树的根结点，所有的epitem */
  struct epitem *ovflist;     /* 在将就绪事件拷贝到用户空间是发生事件的存放地方 */
  struct wakeup_source *ws;
  struct user_struct *user;
  struct file *file;
  int visited;
  struct list_head visited_list_link;
};
```
`epitem`主要用于存储一个待监控目标文件的相关信息，每当要添加一个待监控目标设备到`eventpoll`里时，实际上就是为该设备创建了一个这样的结构体，并将它挂到了`eventpoll::rbr`红黑树中，例如我们要添加一个socket函数创建的socket文件描述符fd时。
```
struct epitem {
	union {
    struct rb_node rbn;
    struct rcu_head rcu;
	};
  struct list_head rdllink;   /* 已经就绪的epitem连接到struct eventpoll::rdllist */
  struct epitem *next;        /* 在拷贝时就绪的epitem连接struct eventpoll::vflist单向链表 */
  struct epoll_filefd ffd;    /* 监听对象的fd和struct file*/
  int nwait;
  struct list_head pwqlist;   
  struct eventpoll *ep;       /* 当前epitem所属eventpoll */
  struct list_head fllink;
  struct wakeup_source __rcu *ws;
  struct epoll_event event;   /*监听事件*/
};

struct epoll_filefd {
  struct file *file; // fd对应的file结构
  int fd;            // 目标文件描述符
} __packed;
```
`eventpoll`中的监听目标文件`epitem`通过`eppoll_entry`的封装挂载到目标文件的等待队列。之后目标文件事件就绪，就会调用节点的回调函数从而通知监听者
```
/* Wait structure used by the poll hooks */
struct eppoll_entry {
  struct list_head llink;    /* 关联到epitem */
  struct epitem *base;       /* 与之关联的epitem的起始地址 */
  wait_queue_t wait;         /* 连接到目标文件的等待队列 */
  wait_queue_head_t *whead;  /* 具体连接的的等待对立 */
};
```

#### `epoll_ctl`函数实现
```
epoll_ctl(epfd, op, fd, event):
  SYSCALL_DEFINE4(epoll_ctl, epfd, op, fd, event):
    /* 如果不是删除操作，就将epoll_event拷贝到内核空间 */
    struct epoll_event epds;
    if(ep_op_has_event(op) &&
       copy_from_user(&epds, event, sizeof(struct epoll_event)))
      goto error_return;
    struct fd f = fdget(epfd);  /* 返回文件描述符对应的struct file */
    struct fd tf = fdget(fd);
    struct eventpoll *ep = f.file->private_data; /* 获取epfd关联的struct eventpoll对象 */
    struct epitem *epi = ep_find(ep, tf.file, fd); /* 在红黑树查找是否有对应的事件 */

    switch(op) {
      case EPOLL_CTL_ADD: /* 不存在添加 */
        ep_insert(ep, &epds, tf.file, fd, full_check); break;
      case EPOLL_CTL_DEL: /* 存在删除 */
        ep_remove(ep, epi); break;
      case EPOLL_CTL_MOD: /* 存在修改 */
        ep_modify(ep, epi, &epds);
    }
```
为目标文件添加监控事件时，首先要保证当前`struct eventpoll`里面还没有对该目标文件进行监听，如果存在，就返回`-EEXIST`错误。否则说明参数正常，然后先默认设置对目标文件的`POLLERR`和`POLLHUP`监听事件，然后调用`ep_insert`函数。`ep_insert`函数核心的两个工作是：（1）将回调函数加入到要监听的文件文件描述符；（2）将要监听事件插入到的红黑树里面。

在分析`ep_insert`函数前，先介绍虚拟文件系统`poll`机制重要的`poll_table`结构。定义在`include/linux/poll.h`
```
typedef void (*poll_queue_proc)(struct file *, wait_queue_head_t *, struct poll_table_struct *);
typedef struct poll_table_struct {
	poll_queue_proc _qproc;
	unsigned long _key;
} poll_table;
```
成员`_key`记录感兴趣的事件，在`epoll`实现中，`_key`被初始化为`~0`，表示监听任何事件。成员`_qproc`是一个回调函数，为了方便处理，`epoll`定义类个包装结构体。
```
struct ep_pqueue {
	poll_table pt;
  struct epitem *epi;
};
```
在`ep_insert`函数中，首先申请一个`struct epitem`对象管理需要监听的文件和事件，然后将它和`poll_table`用`struct ep_pqueue`关联起来，此时`_key`的值为`~0`，`pt->_qproc`指向函数`ep_ptable_queue_proc`。该函数核心是将监听的`epitem`通过`eppoll_entry`挂到目标文件的等待队列上，并指定回调函数是`ep_poll_callback`，该函数的核心是：（1）如果发生监听事件并且`epitem`还没有处于就绪队列上，就将处于就绪的`epitem`挂到`eventpoll::rdlist`就绪队列上；（2）唤醒阻塞在`epoll_wait`上的进程
```
static int ep_insert(struct eventpoll *ep, struct epoll_event *event,
          struct file *tfile, int fd, int full_check) {
  if (!(epi = kmem_cache_alloc(epi_cache, GFP_KERNEL)))  /* 用slab分配一个struct epitem对象 */
	  return -ENOMEM;
  INIT_LIST_HEAD(&epi->rdllink);
  INIT_LIST_HEAD(&epi->fllink);
  INIT_LIST_HEAD(&epi->pwqlist);
  epi->ep = ep;
  ep_set_ffd(&epi->ffd, tfile, fd);
  epi->event = *event;
  epi->nwait = 0;
  epi->next = EP_UNACTIVE_PTR;
  if (epi->event.events & EPOLLWAKEUP) {
    error = ep_create_wakeup_source(epi);
    if (error)
      goto error_create_wakeup_source;
  } else {
    RCU_INIT_POINTER(epi->ws, NULL);
  }

  epq.epi = epi;
  init_poll_funcptr(&epq.pt, ep_ptable_queue_proc);

  revents = ep_item_poll(epi, &epq.pt);

  ep_rbtree_insert(ep, epi); /* 将epitem加入到红黑树中 */

  if ((revents & event->events) && !ep_is_linked(&epi->rdllink)) {
    list_add_tail(&epi->rdllink, &ep->rdllist);   /* 此时就就绪，加到就绪链表 */
    ep_pm_stay_awake(epi);

    /* Notify waiting tasks that events are available */
    if (waitqueue_active(&ep->wq))
      wake_up_locked(&ep->wq);
    if (waitqueue_active(&ep->poll_wait))
      pwake++;
  }
  /* ... */
}
```
接下来调用`op_item_poll`，这个函数调用对应文件的`poll`函数。用TCP为例，调用关系为：
```
op_item_poll(epi, pt):
  pt->_key = epi->event.events;
  tcp_poll(...):
    sock_poll_wait(...):
      poll_wait(...):
        p->_qproc(filep, wait_address, p);
```
最后调用`poll_table::_qproc`函数。`epoll`中指向的是`ep_ptable_queue_proc`，执行整个注册过程--（1）设置监听时间`_key`；（2）将`epitem`挂到目标文件的等待队列上，最后返回目标文件当前的状态。此时就就绪，加到就绪链表。最后调用`ep_rbtree_insert`将`epitem`加入到红黑树中。

#### `epoll_wait`函数实现
```
epoll_wait(epfd, events, maxevents, timeout):
  SYSCALL_DEFINE4(epoll_wait, epfd, events, maxevents, timeout):
    struct fd f = fdget(epfd);
    struct eventpoll *ep = f.file->private_data;
    ep_poll(epfd, events, maxevents, timeout):
```
首先获取`epfd`关联的`struct fd`对象，然后找到管理的`struct eventpoll`对象，再调用`ep_poll`获取就绪事件。
```
int ep_poll(struct eventpoll *ep, struct epoll_event __user *events,
		        int maxevents, long timeout) {
  if(timeout > 0) {
    /* 用户态时间转换成内核时间 */
  } else if(timeout == 0) {
    goto check_events;
  }
fetch_events:
  if(!ep_events_available(ep)) {
    /* 将当前进程加入到ep->wq等待队列里面，然后在一个无限for循环里面，
     * 首先调用set_current_state(TASK_INTERRUPTIBLE)，将当前进程
     * 设置为可中断的睡眠状态，然后当前进程就让出cpu，进入睡眠，直到有其
     * 他进程调用wake_up或者有中断信号进来唤醒本进程，它才会去执行接下来的代码
     */
  }
check_events:
  eavail = ep_events_available(ep);
  if (!res && eavail && /* 将发生的事件拷贝到用户空间 */
      !(res = ep_send_events(ep, events, maxevents)) && !timed_out)
    goto fetch_events;
  return res;
}
```
如果有就绪事件发生，则调用`ep_send_events`函数做进一步处理，在`ep_send_events`函数中又会调用`ep_scan_ready_list`函数获取`eventpoll`中的`rdllist`链表。由于在我们扫描处理`rdllist`链表的时候可能同时会有就绪事件发生，在这个时间段内发生的就绪事件会临时存放在`ovflist`链表中，待`rdllist`处理完毕之后，再将`ovflist`中的内容移动到`rdllist`链表中，等待下次`epoll_wait`的调用。
```
ep_send_events(op, events, maxevents):
  struct ep_send_events_data esed;
  esed.maxevents = maxevents;
  esed.events = events;
  ep_scan_ready_list(ep, ep_send_events_proc, &esed, 0, false)

int ep_scan_ready_list(struct eventpoll *ep,
          int (*sproc)(struct eventpoll *, struct list_head *, void *),
          void *priv, int depth, bool ep_locked) {

  list_splice_init(&ep->rdllist, &txlist); /* 用txlist获取rdllist链表的内容 */
  ep->ovflist = NULL;                      /* 初始化，遍历rdllist时发生的时间放到此处 */

  error = (*sproc)(ep, &txlist, priv);     /* 将数据从内核空间拷贝到用户空间 */

  spin_lock_irqsave(&ep->lock, flags);
  for (nepi = ep->ovflist; (epi = nepi) != NULL;
       nepi = epi->next, epi->next = EP_UNACTIVE_PTR) {
    if (!ep_is_linked(&epi->rdllink)) {
      list_add_tail(&epi->rdllink, &ep->rdllist); /* 将临时存储的事件转移到rdllist */
      ep_pm_stay_awake(epi);
    }
  }
  ep->ovflist = EP_UNACTIVE_PTR;
  list_splice(&txlist, &ep->rdllist);  /* 将没有完全拷贝的事件转移到rdllist */
  /* 唤醒工作 */
}
```
`ep_scan_ready_list`函数中调用`ep_send_events_proc`将数据从内核空间拷贝到用户空间，并不是将`rdllist`链表上所有的结点都拷贝到用户空间，而是先做检查`ep_item_poll(epi, &pt)`，被监听事件确实发生的才会拷贝。如果是Level Trigger，事件再次加入rdllist，表示下次调用epoll_wait会再次检查该事件
```
int ep_send_events_proc(struct eventpoll *ep, struct list_head *head, void *priv) {
  struct ep_send_events_data *esed = priv;
  int eventcnt;
  unsigned int revents;
  struct epoll_event __user *uevent;

  for (eventcnt = 0, uevent = esed->events;
       !list_empty(head) && eventcnt < esed->maxevents;) {
    epi = list_first_entry(head, struct epitem, rdllink);
    list_del_init(&epi->rdllink);      /* 将epitem从就绪队列删除 */
    revents = ep_item_poll(epi, &pt);  /* 确认监听事件发生，可能有进程进行的操作 */
    if (revents) {
      /* 将当前的事件和用户传入的数据都copy给用户空间 */
      if (__put_user(revents, &uevent->events) ||
          __put_user(epi->event.data, &uevent->data)) {
        list_add(&epi->rdllink, head);
        ep_pm_stay_awake(epi);
        return eventcnt ? eventcnt : -EFAULT;
      }
      eventcnt++;
      uevent++;
      if (epi->event.events & EPOLLONESHOT)
        epi->event.events &= EP_PRIVATE_BITS;
      else if (!(epi->event.events & EPOLLET)) { /* 如果是Level Trigger */
        /* 再次加入rdllist，表示调用epoll_wait会再次检查该事件 */
        list_add_tail(&epi->rdllink, &ep->rdllist); 
        ep_pm_stay_awake(epi);
      }
    }
  }
  return eventcnt;
}
```

## 总结
1. select，poll实现需要自己不断轮询所有fd集合，直到设备就绪，期间可能要睡眠和唤醒多次交替。而epoll其实也需要调用epoll_wait不断轮询就绪链表，期间也可能多次睡眠和唤醒交替，但是它是设备就绪时，调用回调函数，把就绪fd放入就绪链表中，并唤醒在epoll_wait中进入睡眠的进程。虽然都要睡眠和交替，但是select和poll在“醒着”的时候要遍历整个fd集合，而epoll在“醒着”的时候只要判断一下就绪链表是否为空就行了，这节省了大量的CPU时间。这就是回调机制带来的性能提升。
2. select，poll每次调用都要把fd集合从用户态往内核态拷贝一次，并且要把current往设备等待队列中挂一次，而epoll只要一次拷贝，而且把current往等待队列上挂也只挂一次（在epoll_wait的开始，注意这里的等待队列并不是设备等待队列，只是一个epoll内部定义的等待队列）。这也能节省不少的开销。
3. 应用场景  
  - `select`的`timeout`参数精度为1ns，而`poll`和`epoll`为 1ms，因此`select`更加适用于实时性要求比较高的场景。select 可移植性更好，几乎被所有主流平台所支持。 
  - `poll`没有最大描述符数量的限制，如果平台支持并且对实时性要求不高，应该使用`poll`而不是`select`。
  - `epoll`只需要运行在Linux平台上，有大量的描述符需要同时轮询，并且这些连接最好是长连接。需要同时监控小于1000个描述符，就没有必要使用`epoll`，因为这个应用场景下并不能体现`epoll`的优势。需要监控的描述符状态变化多，而且都是非常短暂的，也没有必要使用`epoll`。因为`epoll`中的所有描述符都存储在内核中，造成每次需要对描述符的状态改变都需要通过`epoll_ctl`进行系统调用，频繁系统调用降低效率。并且`epoll`的描述符存储在内核，不容易调试。
4. select/poll/epoll区别

|--|select|poll|epoll|
|:-:|:-:|:-:|:-:|
|操作方式|遍历|遍历|回调|
|底层实现|数组|链表|红黑树|
|IO效率|每次线性遍历，O(n)|每次线性遍历，O(n)|调用回调函数，O(1)|
|最大连接数|1024(x86)或2048(x64)|无上限|无上限|
|fd拷贝|每次调用，从用户态烤到内核态|每次调用，从用户态烤到内核态|调用epoll_ctl是拷贝进内存并保存，<br>之后调用epoll_wait只拷贝就绪事件|