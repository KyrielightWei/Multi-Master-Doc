# Multi-Master Cache 参考论文
## 分工
> + 在该表格中标注自己负责的论文和总结，长总结建立新的markdown文件于summary文件夹下，并记录总结文件的文件名
> + 总结分为两种：
    1.简要总结：说明论文价值不高，或是仅有较少的参考价值，总结中说明可参考的点或是无参考价值即可；
    2.详细总结：总结论文中具有参考价值的部分，尽可能附带图片等说明
> + 加入新的论文或完成总结，请发pull request再合并到master分支

|  论文标题   | 阅读人 | 总结 |
|  ----  | ----  | ----  |
|   Maintaining Cache Coherency for B +  Tree Indexes in a Shared Disks Cluster  | wei | （总结文件名 or 简要总结） |
|   RDMA-database/Distributed Lock Management with RDMA  | zhang | 正在看... |
|   多核cache/Cache-Conscious Concurrency Control of Main-Memory Indexes on Shared-Memory Multiprocessor Systems   |  zhang |  在修改  |
|   分布式cache/Cache Coherency in Oracle Parallel Server   | zhang  | 这篇文章就一页，简单提了Oracle共享存储的主要优势,以及Oracle Parallel Server（OPS）怎样解决读写冲突的（和RAC差不多，甚至没有共享缓存，脏页先刷盘才能读），最后提了一下hash locking 和 细粒度锁的区别及使用场景 |
|   分布式cache/Cache Consistencyand Concurrency Control in a ClientServer DBMS Architecture   | zhang  |  这篇文章主要比较了五种并发控制方案的性能，架构确实讲的是多个客户端连接一个服务端的场景，cache存在于client端，事务执行也在客户端。但是，并没有太多的cache一致性细节，Introductino第二段就讲明了论文实验在实现时，并发控制就已经包含了一致性检查，且直言下文会交叉使用"并发控制"和"cache一致性"这两个术语，后面整篇论文没再提一致性是怎么做的。第2节 第三段的假设交代，client缓存在使用前被拉到client,然后在缓存驱逐/事务提交时被刷到server，个人理解相当于在存储层提供了这个一致性保障。  |
|   分布式cache/Cache Fusion Extending Shared-Disk Clusters with Shared Caches   | zhang | 2001年oracle发表在VLDB上的文章，较详细地描述了RAC 的读写过程以及故障恢复，篇幅不长，相当于回顾下前期调研的结果，没有过多的总结。 |  
|   分布式cache/Database Locking Protocols for Large-Scale Cache-Coherent Shared Memory Multiprocessors Design, Implementation and Performance   | zhang | 在看 |
|   分布式cache/Performance of Cache Coherency Schemes in a Shared Disks Transaction Environment   | zhang | 这篇文章在多计算实例，共享存储架构下，减少了细粒度锁（record lock）带来的通信开销，提出了CCS和改进的ECCS方案，总结在summary/Performance  of Cache Coherency Schemes in a Shared Disks Transaction Environment|
