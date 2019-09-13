# Linux 定时器 timerfd
timerfd 是 Linux 为用户程序提供的定时器接口。该接口将定时器抽象为文件，当定时器到期时，对应的文件可读。

## 接口
```
#include <sys/timerfd.h>
int timerfd_create(int clockid, int flags);
int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, 
                    struct itimerspec *old_value);
int timerfd_gettime(int fd, struct itimerspec* curr_value);
```
这些系统调用创建和设置一个定时器，创建的定时器跟一个文件绑定，创建返回对应的文件描述符，对定时器的操作也通过文件描述符来进行。定时器通过文件描述符传递定时器超时通知，当定时器超时，对应的文件可读。timerfd 提供了另外一种使用定时器的方式，将监视定时器超时事件转换为文件可读事件。该“文件”在定时器超时的那一刻变得可读，这样就能很方便地融入到 select(2)，poll(2) 或 epoll(7) 框架中，用统一的方式来处理 IO 事件和超时事件。

timerfd_create(2) 创建一个定时器对象，关联一个文件，然后返回一个文件描述符。其参数含义如下
- clockid：指定用于标记计时器进度的时钟
  - CLOCK_REALTIME：使用系统时间，是从 1970-01-01 00:00:00.000 到当前的时间。更改系统时间会对这个值有影响。
  - CLOSE_MONOTONIC：表示使用从系统重启到现在的时间，更改系统时间对其没有影响
- flags：标志位，指定 timerfd 的性质
  - TFD_NONBLOCK：非阻塞模式
  - TFD_CLOEXEC：相同于 O_CLOEXEC

timerfd_settime(2) 用来设置或者取消超时时间。flags 参数指示设置的是相对时间还是决定时间，取值如下：
- 0：相对时间
- TFD_TIMER_ABSTIME：绝对时间

new_value 和 old_value 用来指定新的超时时间和返回旧的超时时间。itimespec 的定义如下：
```
struct timespec {
  time_t tv_sec;    // 秒数
  long   tv_nsec;   // 纳秒数
};
struct itimespec {
  struct timespec it_interval;  // 时间间隔，周期事件
  struct timespec it_value;     // 超时时间
};
```
其中 new_value.it_value 如果为 0（tv_sec == 0 && tv_nsec == 0）表示取消定时器，否则表示指定超时时间。old_value 不为 NULL，会返回前一次设置的超时值。

timerfd_gettime(2) 获取定时器距离下一次超时还剩下的时间，返回的时间总是表示一个相对时间。

## timerfd 文件描述符操作
### read(2)
read(2) 读取一个 8 字节的无符号整数（uint64_t），其值表示了超时次数（it_interval 设置会循环计时，会有多次超时）。如果 timerfd 是阻塞模式并且没有超时事件，read(2) 会一直阻塞直到发生超时事件。如果是非阻塞模式，会设置 EAGAIN 错误码。如果读取的数据小于 8 个字节，read(2) 失败并设置 EINVAL 错误码

### select(2)，poll(2) 和 epoll(7)
当定时器发生超时事件，文件描述符可读。

## timerfd_create(2) 系统实现
```
/// @file fs/timerfd.c
317 SYSCALL_DEFINE2(timerfd_create, int, clockid, int, flags)
318 {   
319     int ufd;
320     struct timerfd_ctx *ctx; // 定时器
321     
322     /* Check the TFD_* constants for consistency.  */
323     BUILD_BUG_ON(TFD_CLOEXEC != O_CLOEXEC);
324     BUILD_BUG_ON(TFD_NONBLOCK != O_NONBLOCK);
325     
326     if ((flags & ~TFD_CREATE_FLAGS) ||
327         (clockid != CLOCK_MONOTONIC &&
328          clockid != CLOCK_REALTIME &&
329          clockid != CLOCK_REALTIME_ALARM &&
330          clockid != CLOCK_BOOTTIME &&
331          clockid != CLOCK_BOOTTIME_ALARM))
332         return -EINVAL;
333     
334     ctx = kzalloc(sizeof(*ctx), GFP_KERNEL); // 分配已额定时器结构
335     if (!ctx)
336         return -ENOMEM;
337     
338     init_waitqueue_head(&ctx->wqh);
339     spin_lock_init(&ctx->cancel_lock);
340     ctx->clockid = clockid;
341     
342     if (isalarm(ctx))
343         alarm_init(&ctx->t.alarm,
344                ctx->clockid == CLOCK_REALTIME_ALARM ?
345                ALARM_REALTIME : ALARM_BOOTTIME,
346                timerfd_alarmproc);
347     else
348         hrtimer_init(&ctx->t.tmr, clockid, HRTIMER_MODE_ABS); // 初始化
349     
350     ctx->moffs = ktime_get_monotonic_offset();
351     // 分配一个 file 并绑定 file->private_data = ctx
352     ufd = anon_inode_getfd("[timerfd]", &timerfd_fops, ctx,
353                    O_RDWR | (flags & TFD_SHARED_FCNTL_FLAGS)); 
354     if (ufd < 0)
355         kfree(ctx);
356     
357     return ufd;
358 }
```

## timerfd_settime(2) 实现
```
/// @file fs/timerfd.c
456 SYSCALL_DEFINE4(timerfd_settime, int, ufd, int, flags,
457         const struct itimerspec __user *, utmr,
458         struct itimerspec __user *, otmr)
459 {
460     struct itimerspec new, old;
461     int ret;
462 
463     if (copy_from_user(&new, utmr, sizeof(new))) // 从用户空间拷贝到内核空间
464         return -EFAULT;
465     ret = do_timerfd_settime(ufd, flags, &new, &old); // 设置时间
466     if (ret)
467         return ret;
468     if (otmr && copy_to_user(otmr, &old, sizeof(old))) // 拷贝旧值到用户空间
469         return -EFAULT;
470 
471     return ret;
472 }
```
do_timerfd_settime() 的实现如下：
```
/// @file fs/timerfd.c
360 static int do_timerfd_settime(int ufd, int flags,
361         const struct itimerspec *new,
362         struct itimerspec *old)
363 {
364     struct fd f;
365     struct timerfd_ctx *ctx;
366     int ret;
367 
368     if ((flags & ~TFD_SETTIME_FLAGS) ||
369         !timespec_valid(&new->it_value) ||
370         !timespec_valid(&new->it_interval))
371         return -EINVAL;
372 
373     ret = timerfd_fget(ufd, &f); // fd ==> file
374     if (ret)
375         return ret;
376     ctx = f.file->private_data; // 文件绑定的定时器
377 
378     timerfd_setup_cancel(ctx, flags); // 取消已有定时器
379 
380     /*
381      * We need to stop the existing timer before reprogramming
382      * it to the new values.
383      */
384     for (;;) {
385         spin_lock_irq(&ctx->wqh.lock);
386 
387         if (isalarm(ctx)) {
388             if (alarm_try_to_cancel(&ctx->t.alarm) >= 0)
389                 break;
390         } else {
391             if (hrtimer_try_to_cancel(&ctx->t.tmr) >= 0)
392                 break;
393         }
394         spin_unlock_irq(&ctx->wqh.lock);
395         cpu_relax();
396     }
397 
398     /*
399      * If the timer is expired and it's periodic, we need to advance it
400      * because the caller may want to know the previous expiration time.
401      * We do not update "ticks" and "expired" since the timer will be
402      * re-programmed again in the following timerfd_setup() call.
403      */
404     if (ctx->expired && ctx->tintv.tv64) {
405         if (isalarm(ctx))
406             alarm_forward_now(&ctx->t.alarm, ctx->tintv);
407         else
408             hrtimer_forward_now(&ctx->t.tmr, ctx->tintv);
409     }
410     // 拷贝旧值
411     old->it_value = ktime_to_timespec(timerfd_get_remaining(ctx));
412     old->it_interval = ktime_to_timespec(ctx->tintv);
413 
414     /*
415      * Re-program the timer to the new value ...
416      */
417     ret = timerfd_setup(ctx, flags, new); // 设置新值
418 
419     spin_unlock_irq(&ctx->wqh.lock);
420     fdput(f);
421     return ret;
422 }
```

## timerfd_gettime(2) 实现
```
/// @file fs/timerfd.c
474 SYSCALL_DEFINE2(timerfd_gettime, int, ufd, struct itimerspec __user *, otmr)
475 {
476     struct itimerspec kotmr;
477     int ret = do_timerfd_gettime(ufd, &kotmr);
478     if (ret)
479         return ret;
480     return copy_to_user(otmr, &kotmr, sizeof(kotmr)) ? -EFAULT: 0;
481 }
```
do_timefd_gettime() 定义如下
```
/// @file fs/timerfd.c
424 static int do_timerfd_gettime(int ufd, struct itimerspec *t)
425 {   
426     struct fd f;
427     struct timerfd_ctx *ctx;
428     int ret = timerfd_fget(ufd, &f);
429     if (ret)
430         return ret;
431     ctx = f.file->private_data;
432     
433     spin_lock_irq(&ctx->wqh.lock);
434     if (ctx->expired && ctx->tintv.tv64) {
435         ctx->expired = 0;
436 
437         if (isalarm(ctx)) {
438             ctx->ticks +=
439                 alarm_forward_now( 
440                     &ctx->t.alarm, ctx->tintv) - 1;
441             alarm_restart(&ctx->t.alarm);
442         } else {
443             ctx->ticks +=
444                 hrtimer_forward_now(&ctx->t.tmr, ctx->tintv)
445                 - 1;
446             hrtimer_restart(&ctx->t.tmr); // 重启
447         }
448     }
449     t->it_value = ktime_to_timespec(timerfd_get_remaining(ctx));
450     t->it_interval = ktime_to_timespec(ctx->tintv);
451     spin_unlock_irq(&ctx->wqh.lock);
452     fdput(f);
453     return 0;
454 }
```

## TODO
// 内核定时器（高精度和低进度定时器）实现