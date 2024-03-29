
线程同步机制包装类
===============
多线程同步，确保任一时刻只能有一个线程能进入关键代码段.
> * 信号量
> * 互斥锁
> * 条件变量

信号量
---------
信号量是一种特殊的变量，它只能取自然数值并且只支持两种操作：等待(P)和信号(V).假设有信号量SV，对其的P、V操作如下：
> * P，如果SV的值大于0，则将其减一；若SV的值为0，则挂起执行
> * V，如果有其他进行因为等待SV而挂起，则唤醒；若没有，则将SV值加一

信号量的取值可以是任何自然数，最常用的，最简单的信号量是二进制信号量，只有0和1两个值.
> * sem_init函数用于初始化一个未命名的信号量
> * sem_destory函数用于销毁信号量
> * sem_wait函数将以原子操作方式将信号量减一,信号量为0时,sem_wait阻塞
> * sem_post函数以原子操作方式将信号量加一,信号量大于0时,唤醒调用sem_post的线程

以上,成功返回0,失败返回errno

互斥锁
----------
互斥锁,也成互斥量,可以保护关键代码段,以确保独占式访问.当进入关键代码段,获得互斥锁将其加锁;离开关键代码段,唤醒等待该互斥锁的线程.
> * pthread_mutex_init函数用于初始化互斥锁
> * pthread_mutex_destory函数用于销毁互斥锁
> * pthread_mutex_lock函数以原子操作方式给互斥锁加锁
> * pthread_mutex_unlock函数以原子操作方式给互斥锁解锁

以上,成功返回0,失败返回errno

条件变量
-----------
条件变量提供了一种线程间的通知机制,当某个共享数据达到某个值时,唤醒等待这个共享数据的线程.
> * pthread_cond_init函数用于初始化条件变量
> * pthread_cond_destory函数销毁条件变量
> * pthread_cond_broadcast函数以广播的方式唤醒**所有**等待目标条件变量的线程
> * pthread_cond_wait函数用于等待目标条件变量.该函数调用时需要传入**mutex参数(加锁的互斥锁)**,函数执行时,先把调用线程放入条件变量的请求队列,然后将互斥锁mutex解锁,当函数成功返回是=时,互斥锁会再次被锁上.**也就是说函数内部会有一次解锁和加锁操作**.

三者的区别
------------
> * 信号灯与互斥锁和条件变量的主要不同在于”灯”的概念，灯亮则意味着资源可用，灯灭则意味着不可用。
> * 如果说后两中同步方式侧重于”等待”操作，即资源不可用的话，信号灯机制则侧重于点灯，即告知资源可用；没有等待线程的解锁或激发条件都是没有意义的，而没有等待灯亮的线程的点灯操作则有效，且能保持灯亮状态。当然，这样的操作原语也意味着更多的开销。
> * 通俗来说,当有任务到来,互斥锁和条件变量通知等待的线程,如果没线程等待,则本次通知没有意义;信号量通知后,会一直等到有线程sem_wait将其减1



