# mtr 与 buffer  pool的联系

## mtr的结构



![mtr](./images/mtr与buffer_pool.png)



迷你事务（mtr）是innodb存储引擎中事务的基本单位，innodb的事务结构为trx，每个trx由许多mtr组成。

mtr中的主要结构是两个内存栈，m_memo是mtr的私有内存栈，m_log是mtr的redo日志栈。mtr在执行页的读写时，将页的指针push到m_memo中；获取读写锁时，将rw_lock_t的指针push到m_memo中。产生的redo log则push到m_log栈中。

mtr提交时，将redo log刷盘，将脏页放入buffer pool的flush list中。

##  脏页与获取页

+ 获取页：从源码上看，buffer  pool获取页的函数buf_page_get_gen主要由索引系统调用。

+ 脏页：mtr提交时计算所产生的redo log的start lsn和end lsn，将mtr内存栈中的页逐一比较，若是在修改的日志范围内，则判定为脏页，添加到flush list。

mtr与buffer pool模块中的latch与rw_lock_t为页锁与线程的读写锁，事务锁的逻辑由上层的trx_sys与lock_sys处理。