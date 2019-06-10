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

## Posix消息队列

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
  - 问题
    - 考虑一下第一个消息被读出之前有两个消息到达的情形（课题在调用`mq_notify`前调用`sleep`模拟）。这里的基本问题是，通知只是在一个消息没放置到某个空队列上时才发出。如果在能够读出第一个消息前有两个消息到达，那么只有一个通知被发出，只能读出第一个消息，并调用`sigsuspend`等待另一个消息，而对应它的通知可能永远不会发出，在此期间，另一个消息放置于该队列中等待读取，而我们一致忽略它。
- 使用非阻塞`mq_receive`的信号通知