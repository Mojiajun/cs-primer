# 进程间通信（Interprocess Communication，IPC）

## 2、Posix IPC

### 概述
以下三种类型的IPC合称为“Posix IPC”
- Posix消息队列
- Posix信号量
- Posix共享内存

Posix IPC函数

<img src='./imgs/posix-ipc-fcns.png'>

### 创建与打开IPC
- 打开`oflag`参数
 
  <img src='./imgs/open-posix-oflag.png'>

- 创建`mode`参数
 
  <img src='./imgs/create-posix-mode.png'>

## System V IPC

### 概述
以下三种类型的IPC合称为System V IPC
- System V 消息队列
- System V信号量
- System V共享内存区

System V IPC函数

<img src='./imgs/systemv-ipc-fcns.png'>

### `key_t`键和`ftok`函数
三种类型的System V IPC使用`key_t`值作为它们的名字
```
#include <sys/ipc.h>
key_t ftok(const char *pathname, int id); // 出错返回-1
```

### 创建与打开IPC
<img src='./imgs/create-systemv-ipc.png'>

- 调用`ftok`，给它传递pathname和id
- 指定key为`IPC_PRIVATE`，这将保证会创建一个新的、唯一的IPC对象

### IPC权限
<img src='./imgs/open-systemv-oflag.png'>

## 4、管道和FIFO

### 管道
所有时样的UNIX都提供管道。由`pipe`函数创建，提供一个单路（单向）数据流
```
#include <unistd.h>
/* @param
 * fd：返回两个文件描述符，fd[0]和fd[1]。前者读，后者写
 * return：成功返回0，失败返回-1
 */
int pipe(int fd[2]);
```
<img src='./imgs/two-proc-pipe.png'>

### `popen`和`pclose`函数
标准IO函数库提供了`popen`函数，它创建一个管道并启动另外一个进程，该进程要么从该管道读取标准输入，要么往该管道写入标准输出
```
#include <stdio.h>
/* @param
 * command：shell命令行
 * type："r"（调用进程读取command的标准输出）或"w"（调用进程写到command的标准输入）
 * return：成功返回文件指针，失败返回NULL
 */
FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);
```

### FIFO
管道没有名字，只能用于有一个共同祖先进程的各个进程之间，无法在无亲缘关系的两个进程间创建一个管道并将它用作IPC通道（不考虑描述符传递）

FIFO是单向数据流，每个FIFO有一个路径名与之关联，无亲缘关系的进程可以访问同一个FIFO。FIFO也称为有名管道
```
#include <sys/types.h>
#include <sys/stat.h>
/* @param
 * pathname：路径名，是FIFO名字
 * mode：文件权限，隐含O_CREAT | O_EXCL
 * return：成功0，失败-1
 */
int mkfifo(const char *pathname, mode_t mode);
```

### 管道和FIFO的额外属性
- 调用`open`时可指定`O_NONBLOCK`表示  
`writefd = open(FIFO1, O_WRONLY | O_NONBLOCK);`
- 如果一个描述符已经打开，那么可以调用`fcntl`以启用`O_NONBLOCK`标志   
  ```
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  ```

关于管道或FIFO的读出与写入的若个额外规则
- 如果请求读出的数据量多于管道或FIFO中当前可用数据量，那么只返回这些可用的数据
- 如果请求写入的数据的字节数小于或等于`PIPE_BUF`，那么`write`操作保证是原子的
- `O_NONBLOCK`标志的设置对`write`操作的原子性没有影响
- 如果向一个没有为读打开的管道或FIFO写入，那么内核将产生一个`SIGPIPE`信号

### 单个服务器，多个客户
<img src='./imgs/fifo-single-serv-multi-clnt.png'>

例子`fifo-server.c, file-client.c`

### 管道和FIFO限制
- `OPEN_MAX`：一个进程在任意时刻打开的最大描述符数（Pisix要求至少为16）
- `PIPE_BUF`：可原子地网一个管道或FIFO的最大数据量（posix要求至少为512）

## 5、Posix消息队列

### `mq_open`、`mq_close`和`mq_unlink`函数
```
#include <bits/mqueue.h>
typedef int mqd_t;

struct mq_attr {    /* linux */
  long mq_flags;	  /* Message queue flags. */
  long mq_maxmsg;	  /* Maximum number of messages. */
  long mq_msgsize;	/* Maximum message size. */
  long mq_curmsgs;	/* Number of messages currently queued. */
  long __pad[4];
};

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
/* @param
 * name：消息队列名字
 * oflag：打开模式，是O_RDONLY、O_WRONLY或O_RDWR之一，可能按位或上O_CREAT、O_EXCL或O_NONBLOCK
 * mode：如果创建，mode值
 * attr：创建消息队列的属性，如果为空指针，使用默认属性
 * return：成功返回消息队列描述符，出错返回-1
 */
mqd_t mq_open(const char *name, int oflag, ...
              /* mode_t mode, struct mq_attr *attr */);

int mq_close(mqd_t mqdes);

/** 删除消息队列
 * @param
 * name: 消息队列
 * return：成功返回0，失败返回-1
 */
int mq_unlink(const char *name);
```

### `mq_getattr`和`mq_setattr`函数
```
#include <mqueue.h>
/* @param
 * mqdes：消息队列的描述符
 * attr：接收属性的mq_attr结构体变量指针
 * return：成功返回0，失败返回-1
 */
int mq_getattr(mqd_t mqdes, struct mq_attr *attr);

/* @param
 * mqdes：消息队列描述符
 * attr：目标属性值（只使用mg_flags的值）
 * oattr：保存原来属性值
 * return：成功返回0，失败返回-1
 */
int mq_setattr(mqd_t mqdes, const struct mq_attr *attr, struct mq_attr *oattr);
```

### `mq_send`和`mq_receive`函数
```
#include <mqueue.h>
/* @param
 * mqdes：消息队列描述符
 * ptr：传送数据地址指针
 * len：传送数据长度
 *    不能小于能加到所指队列中的消息的最大大小，否则返回EMSGSIZE错误
 * prio：传送数据优先级
 *    必须小于MQ_PRIO_MAX, 0表示不使用
 * return：成功返回0，失败返回-1
 */
int mq_send(mqd_t mqdes, const char *ptr, size_t len, unsigned int prio);

/* @param
 * mqdes：消息队列描述符
 * ptr：接收数据地址指针
 * len：传送数据长度
 *    不能小于能加到所指队列中的消息的最大大小，否则返回EMSGSIZE错误
 * prio：传送数据优先级
 *    必须小于MQ_PRIO_MAX，NULL表示不使用
 * return：成功返回传送数据的字节数，出错返回-1
 */
ssize_t mq_receive(mqd_t mqdes, char *ptr, size_t len, unsigned int *priop);
```

### mqueue 客户/服务端例子
<img src='./imgs/mqueue-client-server.png'>

- client: `mqueue_client.c`
- server: `mqueue_server.c`

### 消息队列限制
- `mq_maxmsg`    队列中的最大消息数
- `mq_msgsize`   给定消息的最大字节数
- `MQ_OPEN_MAX`  一个进程能够同时打开的消息队列的最大数目
- `MQ_PRIO_MAX`  任意现已的最大优先级值加1

### `mq_notify`函数
Posix消息队列允许异步事件通知，以告知何时有一个消息放置到了某个空队列消息队列中，这种通知有两种方式供选择
- 产生一个信号
- 创建一个线程来执行一个指定的函数
```
#include <mqueue.h>
/* @param
 * mqdes：消息队列描述符
 * notification：
 * return：成功返回0，失败返回-1
 */
int mq_notify(mqd_t mqdes, const struct sigevent *notification);

#include <signal.h>
union sigval {
  int   sival_int;  /* integet value */
  void *sival_ptr;  /* pointer value */
};
struct sigevent {
  int              sigev_notify;  /* SIGEV_{NONE, SIGNAL, THREAD} */
  int              sigev_signo;   /* signal number if SIGEV_SIGNO */
  union sigval     sigev_value;   /* passed to signal handler or thread */
                                  /* following tow if SIG_THREAD */
  void           (*sigev_notify_function)(union sigval);
  pthread_attr_t  *sigev_notify_attributes
};
note: sigev_notify specifies how notification is to be performed.
    SIGEV_NONE: don't do anything when the event occurs.
    SIGEV_SIGNAL:Notify the process by sending the signal specified in sigev_signo.
    SIGEV_THREAD: Notify the process by invoking sigev_notify_function "as if" it were the start function of a new thread.
```
`mq_notify`函数说明
- 如果`notification`参数为非空，那么当前进程希望有一个消息到达指定的先前为空的队列时得到通知
- 如果`notification`参数为空指针，而且当前进程目前被注册为接收所指定队列的通知，那么已存在的注册将被撤销
- 任意时刻只有一个进程可以被注册为某个队列的通知
- 当有一个消息到达某个先前为空的队列，而且已有一个进程被注册为接收该队列的通知，只有在没有任何线程阻塞在该队列的`mq_receive`调用的前提下，通知才会发出。也就是说，在`mq_receive`调用中的阻塞比任何通知的注册都优先
- 当该通知被发送给它的注册进程时，其注册即被撤销。该进程必须再次调用`mq_notify`重新注册（如果必要）。

### 例子
- 简单的信号通知`mqnotifysig1.c`
  - 当消息放置到某个空对列中，该程序产生一个`SIGUSR1`信号
  - 问题
    - 在信号处理程序中调用`mq_notify`、`mq_receive`和`printf`函数，这些是非异步信号安全函数
- 信号通知`mqnotifysig2.c`
  - 避免从信号处理程序调用任何函数的方法之一是：让处理程序仅仅设置一个全局标志，由某个线程检查该标志以确定何时接受到一个消息。
  - 通过调用`sigsuspend`阻塞，以等待某个消息的到达。当有一个消息被放置到某个空队列中时，该信号产生，主线程被阻止，信号处理程序执行并设置`mqflag`变量，主线程再次执行，发现`mq_flag`为非零，于是读出该消息
  - 问题
    - 考虑一下第一个消息被读出之前有两个消息到达的情形（课题在调用`mq_notify`前调用`sleep`模拟）。这里的基本问题是，通知只是在一个消息没放置到某个空队列上时才发出。如果在能够读出第一个消息前有两个消息到达，那么只有一个通知被发出，只能读出第一个消息，并调用`sigsuspend`等待另一个消息，而对应它的通知可能永远不会发出，在此期间，另一个消息放置于该队列中等待读取，而我们一致忽略它。
- 使用非阻塞`mq_receive`的信号通知`mqnotifysig3.c`
  - 当使用`mq_notify`产生信号时，总是以非阻塞模式读取消息队列
  - 问题
    - 不够高效，处理器处于忙等（轮询查看队列里有没有数据）。
- 使用`sigwait`代替信号处理程序的信号通知`mqnotifysig4.c`
  - 更为简易（并且更为高效）的办法之一是阻塞在某个函数中，仅仅等待该信号的递交，而不是让内核执行一个只为设置一个标志的信号处理程序。`sigwait`提供了这种能力
  ```
  #include <signal.h>
  /* @param
   * set：等待发生的信号集
   * sig：被递交信号的个数
   * return：成功返回0，失败返回正值，设置errno
   */
  int sigwait(const sigset_t *set, int *sig);
  ```
- 使用`select`的Posix消息队列`mqnotifusig5.c`
  - 消息队列描述符（`mqd_t`变量）不是“普通”描述符，它不能用在`select`或`poll`中
  - 可以伴随一个管道和`mq_notify`函数使用他们（在信号处理函数中调用`write`向管道写数据，`select`用来检测管道是否有数据可读）
- 启动线程`mqnotifythread.c`
  - 异步事件通知的另一种方式是把`sig_notify`设置成`SIGEV_THREAD`，这回创建一个新的线程。该线程调用由`sigev_notify_function`指定的函数，所用的参数由`sigev_value`指定。新线程的属性由`sigev_notify_attributes`指定，要是默认属性合适的化，它可以是一个空指针。

### Posix实时信号
信号可以划分为两个大组
- 其值在`SIGRTMIN`和`SIGRTMAX`之间（包括两者）的实时信号。Posix要求至少提供`RTSIG_MAX`这种实时信号，而该常值的最小值为8
- 所有其他信号：`SIGALRM`、`SIGINT`、`SIGKILL`等

接收某个信号的进程的`sigaction`调用中是否指定了新的`SA_SIGINFO`标志会有实时行为差异

<img src='./imgs/posix-realtime.png'>

实时行为隐含如下特征
- 信号是排队的。也就是说产生几次，就提交几次。对于不排队的信号来说，产生了三次的某种信号可能只提交一次
- 当有多个`SIGRTMIN`到`SIGRTMAX`范围内的解阻塞信号排队时，值较小的信号先于值较大的信号递交。
- 当某个非实时信号递交时，传递给它的信号处理程序的唯一参数是该信号的值。实时信号比其他信号携带更多的信息。通过设置`SA_SIGINFO`标志（`act.sa_falgs = SA_SIGINFO`）安装的任意实时信号的信号处理函数声明如下：
  ```
  void func(int signo, siginfo_t *info, void *context);

  typedef struct {
    int          si_signo;  /* same value as signo argument */
    int          si_code;   /* SI_{USER, QUEUE, TIMER, ASYNCIO, MESGQ} */
    union sigval si_value;  /* interget or pointer from sender*/
  } siginfo_t;
  /**
   SI_USER：信号由kill函数发出
   SI_QUEUE：信号由sigqueue函数发出
   SI_TIMER：信号由timer_settime函数设置的某个定时器的倒是发生
   SI_ASYNCIO：信号由某个异步IO请求的完成产生
   SI_MESGQ：信号在有一个消息被放置到某个空消息队列中时产生
   */
  ```

例子`rtsignal.c`

### 使用内存映射IO实现Posix消息队列

## 6、System V消息队列

### 概述
System V消息队列使用**消息队列标识符**表示。具有足够特权的进程可以向一个给定队列里读取数据。
```

消息队列结构
#include <sys/msg.h>
struct msqid_ds {
  struct ipc_perm  msg_perm;    /* read_write permissions */
  struct msg      *msg_first;   /* ptr to first message on queue */
  struct msg      *msg_last;    /* ptr to last message on queue */
  msglen_t         msg_cbytes;  /* current # bytes on queue */
  msgqnum_t        msg_qnum;    /* current # of mseeages on queue */
  msglen_t         msg_qbytes;  /* max # of bytes allowed on queue */
  pid_t            msg_lspid;   /* pid of last msgsnd() */
  pid_t            msg_lrpid;   /* pid of last msgrcv() */
  time_t           msg_stime;   /* time of last msgsnd() */
  time_t           msg_rtime;   /* time of last msgrcv() */
  time_t           msg_ctime;   /* time of last msgctl() */
};
```
<img src='./imgs/system-v-msg-queue.png'>

### `msgget`函数
用于创建一个新的消息队列或访问一个已存在的消息队列
```
#include <sys/msg.h>
/* @param 
 * key：既可以是ftok的返回值，也可以是IPC_PRIVATE常值
 * oflag：读写权限值的组合
 * return：成功返回非负标识符，失败返回-1
 */
int msgget(key_t key, int oflag);
```

当创建一个新消息队列时，`msdid_ds`结构的如下成员被初始化：
- `msg_perm`结构的`uid`和`cuid`成员被设置成当前进程的有效用户ID，`gid`和`cgid`成员被设置为当前进程的有效组ID
- `oflag`的读写权限位存放在`msg_perm.mode`中
- `msg_qnum`、`msg_lspid`、`msg_lrpid`、`msg_stime`和`msg_rtime`被设置为0
- `msg_ctime`被设置成当前时间
- `msg_qbytes`被设置成系统限制值

### `msgsnd`函数
```
#include <sys/msg.h>
/* @param
 * msqid：标识符
 * ptr：struct msgbuf结构指针
 * length：消息长度，字节数
 * flag：既可以是0，也可以是IPC_NOWIAT（非阻塞）
 * return：成功返回0，失败返回-1
 */
int msgsnd(int msqid, const void *ptr, size_t length, int flag);

struct msgbuf {
  long mtype;    /* message type, must be > 0 */
  char mtext[1]; /* message data */
};
```

### `msgrcv`函数
```
#include <sys/msg.h>
/* @param
 * msqid：标识符
 * ptr：接收消息的存放位置
 * length：ptr指向的缓冲区中数据部分的大小
 * type：指定希望从给定的队列中读出什么样的消息
 *    0：返回队列中的第一个消息
 *    >0：返回其类型值为type的第一个消息
 *    <0：返回其类型值小于或等于type参数的结对值的消息中类型值最小的第一个消息
 * flag：指定所请求的消息不再所指定的队列中时该做如何处理
 *    0：阻塞，知道下列某个事件发生为止：
 *        （1）有一个所请求类型的消息可读
 *        （2）由msqid标识的消息队列从系统删除（此时返回EIDRM错误）
 *        （3）调用线程被某个捕获的信号所中断（此时返回EINTR错误）
 *    IPC_NOWAIT：立即返回一个ENOMSG错误
 *    MSG_NOERROR：消息大于length时直接截断（没有设置就返回E2BIG错误）
 * return：成功返回读取的字节数，是否返回-1
 */
ssize_t msgrcv(int msqid, void *ptr, size_t length, long type, int flag);
```

### `msgctl`函数
```
#include <sys/msg.h>
/* @param
 * msqid：标识符
 * cmd：控制操作命令
 * buff：
 * return：成功返回0，出错返回-1
 */
int msgctl(int msqid, int cmd, struct msqid_ds *buff);
```
`msgctl`函数提供3个命令
- IPC_RMID：从系统中删除由mqsid指定的消息队列。当前在该队列上的任何消息都被丢弃，第三个参数被忽略
- IPC_SET：给指定的消息队列设置其msgid_ds结构的以下4个成员`msg_perm.uid`、`msg_perm.gid`、`msg_perm.mode`和`msg_qbytes`。它们的值来自有`buff`参数指向的结构中的相应成员
- IPC_STAT：（通过`buff`参数）给调用者返回当前`msqid_ds`结构
  
### 复用消息
与一个队列中的每个消息相关联的类型字段提供了两个特性
- 类型字段可用于标识消息，从而允许多个进程在单个队列上复用消息
- 类型字段可用作优先级字段
  
例子：每个应用一个队列

<img src='./imgs/system-v-multiplexing.png'>

例子：每个客户一个队列

<img src='./imgs/system-v-multiplexing2.png'>

### 消息队列的限制
<img src='./imgs/system-v-restrict.png'>

## 7、互斥锁和条件变量

### 互斥锁：上锁与解锁
互斥锁指代相互排斥，用于保护临界区，以保证任何时刻只有一个线程（或进程）在执行其中的代码。
```
#include <pthread.h>
// 均返回：成功0，失败返回正数Exxx值
int pthread_mutex_lock(pthread_mutex_t *mptr);
int pthread_mutex_trylock(pthread_mutex_t *mptr);
int pthread_mutex_unlock(pthread_mutex_t *mptr);
```
如果尝试给一个已由某个线程锁住的互斥锁上锁，那么`pthread_mutex_lock`将阻塞到该互斥锁解锁为止。`pthread_mutex_trylock`是对应的非阻塞函数，如果该互斥锁已锁住，它就返回`EBUSY`错误。

如果互斥锁变量是静态分配的，那么可以把它初始化成常值`PTHREAD_MUTEX_INITIALIZER`。如果是动态分配的（例如调用`malloc`），必须在运行时之前调用`pthread_mutex_init`函数初始化

### 生产者和消费者问题`mutex_prodcons1.c`
生产者产生所有数据之后，消费者开始启动

<img src='./imgs/producer-and-comsumer.png'>

### 对比上锁与等待`mutex_prodcons2.c`
生产者和消费者并发执行，采用轮询的方式查看某个数据是否有生产者生成

### 条件变量：等待与信号发送`mutex_prodcons3.c`
条件变量用于等待。每个条件变量总是有一个互斥锁与之关联。

如果条件变量是静态分配的，那么可以把它初始化成常值`PTHREAD_MUTEX_INITIALIZER`，如果是动态分配的（例如调用`malloc`），必须在运行时之前调用`pthread_cond_init`函数初始化
```
#include <pthread.h>
/* @param
 * cptr：条件变量指针
 * mptr：互斥锁指针
 * return：成功返回0，失败返回为正的Exxx值
 * 会先解除*mptr，然后阻塞在等待对列里休眠，直到再次被唤醒。唤醒后，该进程会先锁定*mptr，再读取资源
 */
int pthread_cond_wait(pthread_cond_t *cptr, pthread_mutex_t *mptr);

/* @param
 * cptr：条件变量指针
 * return：成功返回0，失败返回为正的Exxx值
 */
int pthread_cond_signal(pthread_cond_t *cptr);
```
总的来说，给条件变量发送信号的代码大体如下：
```
struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  whatever variables maintain the condition
} var = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, ... };

pthread_mutex_lock(&var.mutex);
set condition true;
pthread_cond_signal(&val.cond); // 上锁冲突，即使唤醒，不能获取var.mutex，不能运行
pthread_mutex_unlock(&var.mutex);
```
测试条件并进入睡眠以等待该条件变为真的代码如下：
```
pthread_mutex_lock(&var.mutex);
while(condtion is false)
  pthread_cond_wait(&var.cond, &var.mutex); // 先解除mutex，让权等待
modify condition;
pthread_mutex_unlock(&var.mutex);
```
避免上锁冲突
```
int dosignal;
pthread_mutex_lock(&nready.mutex);
dosignal = (nready.nready == 0);
nready.nready++;
pthread_mutex_unlock(&nready.mutex);

if(dosignal)
  pthread_cond_signal(&nready.cond);
```

### 条件变量：定时等待和广播
通常`pthread_cond_signal`只唤醒等待在相应条件变量上的一个线程。在某些情况下一个线程认定有多个其他线程应被唤醒，这时可调用`pthread_cond_broadcast`唤醒阻塞在相应条件变量上的所有线程。
```
#include <pthread.h>
/* @param
 * cptr：条件变量
 * return：成功返回0，失败返回为正的Exxx值
 */
int pthread_cond_broadcast(pthread_cond_t *cptr);

/* @param
 * cptr：条件变量
 * mptr：互斥变量
 * abstime：绝对时间，子UTC时间1970年1月1日以来流逝的秒数和纳秒数
 * return：成功返回0，失败返回为正的Exxx值
 */
int pthread_cond_timedwait(pthread_cond_t *cptr, pthread_mutex_t *mptr, const struct timespec *abstime);

struct timespect {
  time_t tv_sec;  /* seconds */
  long   tv_nsec; /* nanoseconds */
}
```

### 互斥锁和条件变量的属性
互斥锁和条件变量是用以下函数初始化或摧毁的
```
#include <pthread.h>
/** 
 * mptr和cptr必须指向以分配的pthread_mutex_t或pthread_cond_t变量
 * attr指向属性变量，如果为NULL，使用默认属性
 * 均返回：成功返回0，失败返回为正的Exxx值
 */
int pthread_mutex_init(pthread_mutex_t *mptr, const pthread_mutextattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mptr);
int pthread_cond_init(pthread_cond_t *cptr, const pthread_condarrt_t *attr);
int pthread_cond_destroy(pthread_cond_t *cptr);
```
互斥锁属性的数据类型为`pthread_mutexattr_t`，条件变量属性的数据类型为`pthread_condattr_t`，它们由以下函数初始化或摧毁
```
#include <pthread.h>
/**
* 均返回：成功返回0，失败返回为正的Exxx值
*/
int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);
```
一旦某个互斥锁属性对象或某个条件变量属性对象被初始化，就通过调用不同函数启用或禁止特定的属性。举例来说，我们将在以后各章中使用的一个属性是：指定互斥锁或条件变量在不同进程间共享，而不是只在单个进程内的不同线程间共享。这个属性是用以下函数取得或存入的
```
#include <pthread.h>
/**
* value的值可以是
*     PTHREAD_PROCESS_PRIVATE：进程的线程间共享
*     PTHREAD_PROCESS_SHARED：进程间共享
* 均返回：成功返回0，失败返回为正的Exxx值
*/
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *valptr);
int pthread_mutexattr_setpshared(const pthread_mutexattr_t *attr, int value);
int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *valptr);
int pthread_condattr_setpshared(const pthread_condattr_t *attr, int value);
```

### 持有锁期间进程终止
- 进程终止时系统不会自动释放持有的互斥锁、读写锁和Posix信号量，进程终止时内核总是自动清理的唯一同步锁类型是`fcntl`记录锁。使用System V信号量时，应用程序可以选择进程终止时内核是否自动清理某个信号量锁
- 一个线程也可以在持有某个互斥锁期间终止，自己调用`pthread_exit`或被另一个线程取消。如果线程调用`pthread_exit`资源终止时，这时它应该知道自己还持有一个互斥锁(对于程序员来说)，如果是被另一个线程取消的情况，线程可以安装清楚处理程序(`pthread_cleanup_push`)，在被取消时调用来释放相应的锁。
- 对于线程意外操作导致进程终止的情况，就和进程终止时相同。