# Linux 事件通知 eventfd
eventfd 是 Linux 为用户程序提供的定时器接口。该接口将定时器抽象为文件，将事件的通知和接受转换为文件的读和写。

## 接口
```
#include <sys/eventfd.h>
int eventfd(unsigned int initval, int flags);
```
eventfd() 创建一个 eventfd_ctx 对象，可以被应用程序用作事件等待和通知机制。该对象包含有内核维护的一个 8 字节无符号整数的计数器，在创建时可以通过 initval 指定初始值。

flags 用于指定标志，可以是以下标识或组合（位或）
- EFD_CLOEXEC：等同于 O_CLOEXEC
- EFD_NONBLOCK：非阻塞模式，等同于 O_NONBLOCK
- EFD_SEMAPHORE：支持信号量语义的 read()，每次读都递减 1

关于 eventfd 文件描述符的一个关键点是它可以像使用 select，poll 或 epoll 一样监视任何其他文件描述符。 这意味着应用程序可以同时监视“传统”文件的准备情况以及支持 eventfd 接口的其他内核机制的准备情况。

和管道 pipe 相比，eventfd 完全可以替换管道。此外 eventfd 占用更少的内核资源，eventfd 只消耗一个文件描述符而管道占用两个文件描述符。

## eventfd 文化描述符操作
### read()
对 eventfd 调用 read() 成功返回后，read() 读取的内容是 8 字节无符号整数（uint64_t），这个无符号整数是宿主字节序。如果缓冲区小于 8 字节，read() 出错，设置 EINVAL 错误码。read() 的语义取决于 eventfd 计数器当前是否具有非零值以及在创建 eventfd 文件描述符时是否指定了 EFD_SEMAPHORE 标志
- 如果没有设置 EFD_SEMAPHORE，并且 eventfd 的计数器不为 0，read() 读取  8 字节无符号整数，该整数的值就是计数器现在的值，计时器重新置 0。
- 如果设置了 EFD_SEMAPHORE，并且 eventfd 的计数器不为 0，read() 读取  8 字节无符号整数，该整数的值是 1，计时器递减 1。
- 如果 eventfd 计数器的值为 0，read() 会阻塞直到计数器的值变为非 0。如果在非阻塞模式下，read() 返回 0，并将 errno 设置 EAGAIN

### write()
write() 将其缓冲区中提供的 8 字节整数累加到 eventfd 的计数器上。如果累加的结果会溢出，write() 调用将阻塞直到 read() 调用执行，或者在非阻塞模式下会失败，设置 errno 为 EAGAIN。如果提供的缓冲区小于 8 字节，或者尝试写 (uint64_t)-1，write（） 失败，设置 errno 为 EINVAL。

### select()、poll() 和 epoll_wait()
eventfd 文件描述符支持 IO 复用。
- 当 eventfd 的计数器大于 0，文件描述符可读
- 如果 eventfd 的计数器至少可以再加 1 时，文件描述符可写
- 如果检测到 eventfd 的计数器发生溢出，select() 会返回文件描述符既可读，也可写；poll() 会返回文件描述符 POLLERR 事件；可以知道，write() 调用永远不会使 eventfd 的计数器发生溢出。如果溢出已经发生，read() 读取的 8 字节无符号整数的值为 (uint64_t)-1。

## eventfd() 实现
```
@file fs/eventfd.c
423 SYSCALL_DEFINE2(eventfd2, unsigned int, count, int, flags)
424 {
425     int fd, error;
426     struct file *file;
427     // 申请一个文件描述符
428     error = get_unused_fd_flags(flags & EFD_SHARED_FCNTL_FLAGS);
429     if (error < 0)
430         return error;
431     fd = error;
432 
433     file = eventfd_file_create(count, flags); // 绑定一个文件
434     if (IS_ERR(file)) {
435         error = PTR_ERR(file);
436         goto err_put_unused_fd;
437     }
438     fd_install(fd, file);
439 
440     return fd;
441 
442 err_put_unused_fd:
443     put_unused_fd(fd);
444 
445     return error;
446 }
447 
448 SYSCALL_DEFINE1(eventfd, unsigned int, count)
449 {
450     return sys_eventfd2(count, 0);
451 }
```
eventfd_file_create() 的实现如下：
```
394 struct file *eventfd_file_create(unsigned int count, int flags)
395 {
396     struct file *file;
397     struct eventfd_ctx *ctx;
398 
399     /* Check the EFD_* constants for consistency.  */
400     BUILD_BUG_ON(EFD_CLOEXEC != O_CLOEXEC);
401     BUILD_BUG_ON(EFD_NONBLOCK != O_NONBLOCK);
402 
403     if (flags & ~EFD_FLAGS_SET)
404         return ERR_PTR(-EINVAL);
405 
406     ctx = kmalloc(sizeof(*ctx), GFP_KERNEL);
407     if (!ctx)
408         return ERR_PTR(-ENOMEM);
409 
410     kref_init(&ctx->kref);
411     init_waitqueue_head(&ctx->wqh);
412     ctx->count = count; // 计数器初始值
413     ctx->flags = flags; // 标志
414     // 分配inode，dentry，绑定 ctx，file->private_data = ctx
415     file = anon_inode_getfile("[eventfd]", &eventfd_fops, ctx,
416                   O_RDWR | (flags & EFD_SHARED_FCNTL_FLAGS));
417     if (IS_ERR(file))
418         eventfd_free_ctx(ctx);
419 
420     return file;
421 }
```
eventfd_ctx 的定义如下：
```
25 struct eventfd_ctx {
26     struct kref       kref;  // 引用计数
27     wait_queue_head_t wqh;   // 等待队列
36     __u64             count; // 计数器
37     unsigned int      flags; // 标志
38 };
```