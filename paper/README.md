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
|   Distributed Lock Management with RDMA  | zhang | 正在看... |
|   多核cache/Cache-Conscious Concurrency Control of Main-Memory Indexes on Shared-Memory Multiprocessor Systems   |  zhang |  文章提出了多核访问主存索引的乐观并发控制方式，采用多版本，有点类似于RAC里面的读，具体总结在 summary 中。  |
|   分布式cache/Cache Coherency in Oracle Parallel Server   | zhang  | 这篇文章就一页，简单提了Oracle共享存储的主要优势,以及Oracle Parallel Server（OPS）怎样解决读写冲突的（和RAC差不多，甚至没有共享缓存，脏页先刷盘才能读），最后提了一下hash locking 和 细粒度锁的区别及使用场景 |