# 操作系统相关问题

## 基础
### 线程与进程的区别，线程切换与进程切换？进程切换一定比线程切换开销大吗？（进程间线程切换）
### 操作系统用户态和内核态
### 操作系统内存是怎么管理的，虚拟内存是什么
### 堆内存和栈内存区别与联系
### 程序a调用程序b ，栈有什么变化
### 堆和栈的增长方向
### c程序的内存是怎么分配的
### 解释僵尸进程，孤儿进程
### 字节序的概念
### 静态链接和动态链接的优缺点
### 用户虚拟地址空间意义
### 进程拥有的资源有那些
### 父进程和子进程在内存上的关系

## 进程间通信与同步
### 进程间通信方式有哪些
1. 管道pipe：管道是一种半双工的通信方式，数据只能单向流动，而且只能在具有亲缘关系的进程间使用。进程的亲缘关系通常是指父子进程关系。
2. 命名管道FIFO：有名管道也是半双工的通信方式，但是它允许无亲缘关系进程间的通信。
4. 消息队列MessageQueue：消息队列是由消息的链表，存放在内核中并由消息队列标识符标识。消息队列克服了信号传递信息少、管道只能承载无格式字节流以及缓冲区大小受限等缺点。
5. 共享存储SharedMemory：共享内存就是映射一段能被其他进程所访问的内存，这段共享内存由一个进程创建，但多个进程都可以访问。共享内存是最快的 IPC 方式，它是针对其他进程间通信方式运行效率低而专门设计的。它往往与其他通信机制，如信号两，配合使用，来实现进程间的同步和通信。
6. 信号量Semaphore：信号量是一个计数器，可以用来控制多个进程对共享资源的访问。它常作为一种锁机制，防止某进程正在访问共享资源时，其他进程也访问该资源。因此，主要作为进程间以及同一进程内不同线程之间的同步手段。
7. 套接字Socket：套解口也是一种进程间通信机制，与其他通信机制不同的是，它可用于不同及其间的进程通信。
8. 信号 ( sinal ) ： 信号是一种比较复杂的通信方式，用于通知接收进程某个事件已经发生。

### 线程同步方法
### 解释生产者消费者模型
### 讲讲信号和信号量，它们有什么区别
### 解释信号量，条件变量，锁及相关
### 管道通信相关操作和实现
### [可重入函数](https://www.ibm.com/developerworks/cn/linux/l-reent.html)
#### 定义
- A function whose effect, when called by two or more threads, is guaranteed to be as if the threads each executed the function one after another in an undefined order, even if the actual execution is interleaved.
- 可重入（reentrant）函数可以由多于一个任务并发使用，而不必担心数据错误。可重入函数可以在任意时刻被中断， 稍后再继续运行，不会丢失数据。

#### 条件
- 可以在执行的过程中可以被打断
- 被打断之后，在该函数一次调用执行完之前，可以再次被调用（或进入，reentered)
- 再次调用执行完之后，被打断的上次调用可以继续恢复执行，并正确执行

#### 实现要求
- 不为连续的调用持有静态数据
- 不返回指向静态数据的指针，所有数据都由函数的调用者提供
- 使用本地数据，或者通过制作全局数据的本地拷贝来保护全局数据
- 绝不调用任何不可重入函数
- 不利用锁实现资源互斥访问

#### 确保可重入性的经验
- 返回指向静态数据的指针可能会导致函数的不可重入 
  ```
  调用者准备输出存储空间确保函数的可重入性
  char *StrToUpper(char *src) { /* 不可重入 */
    static char buffer[STRING_SIZE_LIMIT];
    int index;
    for(index = 0; str[index]; index++)
      buffer[index] = toupper(str[index]);
    buffer[index] = '\0';
    return buffer;
  }
  char *StrToUpper(char *src, char *dst) { /* 可重入 */
    int index;
    for(index = 0; str[index]; index++);
      dst[index] = src[index];
    dst[index] = '\0';
    return dst;
  }
  ```
- 记忆数据的状态使函数不可重入
  ```
  char getLowercaseChar(char *str) { /* 不可重入 */
    static char *buffer;
    static int index;
    char c = '\0';
    if (string != NULL) {
      buffer = str; index = 0;
    }
    while(c=buff[index]){
      if(islower(c)){
        index++; break;
      }
      index++;
    }
    return c;
  }

  char getLowercaseChar_r(char *str, int *pIndex) { /* 可重入 */ 
    char c = '\0';
    while(c=buff[*pIndex]){
      if(islower(c)) {
          (*pIndex)++; break;
      }
      (*pIndex)++;
    }
      return c;
  }
  ```
- 在大部分系统中，`malloc`和`free`都不是可重入的，因为他们使用静态数据结构来记录那些内存块是空闲的
  ```
  避免在信号处理函数中分配内存的方法是，为信号处理器预先分配要使用的内存
  ```
- 为了编写没有bug的代码，要特别小心处理进程范围内的全局变量，如`errno`和`h_errno`
  ```
  if(closed(fd) < 0) { /* errno的危险用法 */
    printf(stderr, "error in close, errno: %d", errno);
    exit(1);
  }

  在信号处理器内保存和恢复errno的值
  void signalHandler(int signo) {
    int errno_saved = errno;
    /* signal handler working */
    errno = errno_saved;
  }
  ```
- 如果底层的函数处于关键部分，并且生成并处理信号，那么这可能会导致函数不可重入
  ```
  通过阻塞信号，代码的关键区域可以被保护起来不受一组特定信号的影响
  1. 保存当前信号设置
  2. 阻塞不相关信号
  3. 使代码的关键部分完成其工作
  4. 重置信号

  sigset_t newmask, oldmask, zeromask;
  signal(SIGALRM, sighandler);
  
  /* Initialize the signal sets */
  sigemptyset(&newmask); sigemptyset(&zeromask);
  /* add the signal to the set */
  sigaddset(&newmask, SIGALRM);
  /* block SIGALRM and save current signal mask */
  sigprocmask(SIG_BLOCK, &newmask, &oldmask);
  /**
   * ......
   */
  /* allow all signals and pause */
  sigsuspend(&zeromask);
  /* resume to the original signal mask */
  sigprocmask(SIG_SETMASK, &oldmask, NULL)
  ```

### 信号安全函数（异步信号安全函数）
#### 定义
- An async-signal-safe function is one that can be safely called from within a signal handler.
- 信号中断会打断当前正在执行程序，正常执行程序被挂起，转而执行信号处理函数。信号安全函数能够保证当被打断程序再次运行时，一定能够得到正确的结果。
- 在进程中断期间，系统会保存和恢复进程的上下文，然而恢复的上下文仅限于返回地址，cpu寄存器等之类的少量上下文，而函数内部使用的诸如全局或静态变量，buffer等并不在保护之列，所以如果这些值在函数被中断期间发生了改变，那么当函数回到断点继续执行时，其结果就不可预料了。

#### [实现](https://www.ibm.com/developerworks/cn/linux/l-cn-signalsec/)
- 确保（a）信号处理程序仅调用异步信号安全函数，并且（b）信号处理程序本身对于主程序中的全局变量是可重入的。
- 当调用不安全的函数或对信号处理程序也访问的全局数据进行操作时，在主程序中阻止信号传递。
- 信号处理函数尽量只执行简单的操作，譬如只是设置一个外部变量，其它复杂的操作留在信号处理函数之外执行；
- errno 是线程安全，即每个线程有自己的 errno，但不是异步信号安全。如果信号处理函数比较复杂，且调用了可能会改变 errno 值的库函数，必须考虑在信号处理函数开始时保存、结束的时候恢复被中断线程的 errno 值；
- 信号处理函数只能调用可以重入的 C 库函数；譬如不能调用 malloc（），free（）以及标准 I/O 库函数等；
- 信号处理函数如果需要访问全局变量，在定义此全局变量时须将其声明为 volatile，以避免编译器不恰当的优化。

### 线程安全函数
#### 定义
- A function that may be safely invoked concurrently by multiple threads. 
- 一个函数被称为线程安全的（thread-safe），当且仅当被多个并发进程反复调用时，它会一直产生正确的结果

#### 线程不安全函数
- 不保护共享变量的函数；
- 函数状态随着调用改变的函数；
- 返回指向静态变量指针的函数；
- 调用线程不安全函数的函数；

### 可重入函数与线程安全的区别与联系
- 线程安全是在多线程情况下引发的，而可重入函数可以在只有一个线程的情况下发生。
- 线程安全不一定是可重入的，而可重入函数则一定是线程安全的。
- 如果一个函数有全局变量，则这个函数既不是线程安全也不是可重入的。
- 如果一个函数当中的数据全身自身栈空间的，则这个函数即使线程安全也是可重入的。
- 如果将对临界资源的访问加锁，则这个函数是线程安全的；但如果重入函数的话加锁还未释放，则会产生死锁，因此不能重入。
- 线程安全函数能够使不同的线程访问同一块地址空间，而可重入函数要求不同的执行流对数据的操作不影响结果，使结果是相同的。

### 锁、进程同步和相关操作
## cache
### cache原理和设计？
### cache一致性？
### 为什么要设置cache？硬件设备上是如何实现
### buffer和cache的区别？
### cache淘汰策略？可以用什么数据结构存储？为什么？优缺点是？（LRU 链表）
### CACHE一致性如何解决；（MEIS状态机转移机制）

## linux
### 内存中的栈的实现原理
### 怎么样能让栈增长
### linux中怎么查看堆栈
### 解释一下.so文件
### Linux写时复制技术
### Linux中的map操作

## 应用
### 线程级并行方式有哪些？（openmp pthread等）
### 如何调整多线程的负载均衡
### 假设有两个线程，一个线程怎么了解到另外一个线程是否崩溃；
### 实现一个线程安全的queue的方法；（加锁，CAS队列，lock-free queue，mutex等）
### 可能导致segmentation fault可能的操作？
### Linux共享库的概念和意义
### ork会有什么操作？
### 用户调用write和read后操作系统会发生什么