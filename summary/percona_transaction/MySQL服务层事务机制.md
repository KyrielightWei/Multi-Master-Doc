

## MySQL服务层的事务

+ 语句事务（statement transaction）
+ 标准事务（normal transaction）

语句事务是标准事务的一部分，其提交结果不会持久化到磁盘，用于保证语句原子性，并自动维护一个savepoint。

当有一个新连接时，thd->transaction成员初始化为空。执行需要操作表的语句时，存储引擎执行语句前需要向服务层注册。thd维护参与当前语句的所有存储引擎列表和参与当前标准事务的所有存储引擎列表，用于之后发出commit或rollback请求。

比较特殊的地方是，即使在单机的场景下，服务层的事务也可能会采用两阶段提交协议。

官方文档中关于采用两阶段提交协议的场景的描述为：

+ 所有参与的引擎都支持2PC
+ 事务至少在两个存储引擎中修改数据

执行2PC过程的单机事务称为内部XA事务，而外部XA事务才是真正的分布式事务。

许多源码解析的博客中都有提到，启用binlog时，也会使事务执行两阶段提交协议。

## MySQL 服务层事务函数调用栈

事务提交调用栈，无论是语句事务还是常规事务都会调用到ha_commit_trans函数。

| 文件名                        | 函数名                   |
| ----------------------------- | ------------------------ |
| percona-server/sql/handler.cc | ha_commit_trans          |
| percona-server/sql/handler.cc | tc_log->commit(thd, all) |
| percona-server/sql/tc_log.cc  | ha_commit_low            |
```
handlerton *ht = ha_info->ht();
ht->commit(ht, thd, all);
```
提交最终落实到所有涉及的存储引擎的commit调用。
handlerton是一个存储引擎的实例，初始化在innodb_init函数中，commit是一个函数指针，初始化为函数innobase_commit的地址。

> 除了commit之外，prepare和rollback也是类似的调用栈。

### innodb中的事务commit
在innobase_commit中根据服务层发出的提交类型不同，调用存储引擎中的不同函数：
1. 若提交类型是常规事务，则调用innobase_commit_low(trx) ==> trx_commit_for_mysql(trx)
2. 若提交类型是语句事务，则调用trx_mark_sql_stat_end，告知存储引擎语句执行结束，更新undo log的savepoint

### innodb中的事务start
由官方文档中的描述，可知MySQL事务的启动并非显式的，而是隐式的启动，调用的是start_stmt和external_lock函数。
若是自动提交，则语句的开始就是事务的开始，语句的提交就是事务的提交。非自动提交时，事务的开始与提交的时机也有明确的语句告知。
最终到trx层调用的是TrxInInnoDB::begin_stmt(trx)。