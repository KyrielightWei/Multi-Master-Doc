#### 主要解决的问题：  
通常的索引并发控制是，对需要读写的节点上锁，但是这样会导致共享内存多处理器上的所谓一致性缓存未命中（后面举例说明）。（这里“共享”指处理器的多个核对唯一的内存作读写）。文章提出了一种乐观的，不用上锁的并发控制方式，来减小这种 cache missing。  
说白了就是通过这种并发控制，能够支持更快速的读/写。

##### 文章 2.3 节提到了几个并发访问的索引设计方案：  
1.  lock coupling 上锁粒度大
2.  Blink-Tree  
3.  tree-level locking 对整个树上锁，无疑性能会很差
4.  physical versioning 这种方法简而言之就是，更新索引节点时，创建对应节点的副本，更新在副本上进行；有一个垃圾回收机制，当旧版本的节点没有读写时，就会删除旧版本的节点。


#### 下图是冷启动时，四个CPU核并行遍历内存索引的例子。
![figure1](https://github.com/PokemonWei/Multi-Master-Doc/blob/zhang-dev/summary/images/Cache-Conscious%20Concurrency%20Control%20of%20Main-Memory%20Indexes%20on%20Shared-Memory%20Multiprocessor%20Systems/figure1.png)

n1-n7是7个数据块，图中索引共四个路径，每个CPU依次遍历一条路径（因为都需要根节点的锁，所以根节点不能并发访问）。p1读入n1、n2、n4之后，p2也读了同样的n1、n2,那么需要将p1核内缓存 n1、n2失效，同理，后面的核读相应数据时，也需要将前面部分核内部分数据缓存失效。这就是开始将的一致性缓存未命中。

>这里存在的疑问是，既然是遍历索引，应该只读，为什么会存在使其他核内缓存数据失效的情况？  
3.1 小节开头解释是，无论是独占锁还是共享锁，都需要涉及到内存写操作，所以会存在这样的情况。

#### 本文提出的方案：  
在B+树和CSB+树（一种改进的B+树，为缓存设计的，可以减少B+树的指针数量，从而缓存更多数据）的索引头部加入版本信息，读操作先读取版本信息，读完之后再读一遍版本，比较是否一致，不一致就重新执行读操作。  
文章把这种并发控制叫做乐观控制，有点类似事务里面的乐观控制。  
写操作还是加上独占锁。

#### 为什么能这么做？  
3.2 节分析了更新操作访问的节点位置，大多靠近叶子结点，
遍历时不需要从上层上锁一直到下层（读也是要上锁的），直接到叶子结点附近上锁更新就行。  
此时“多版本”读不需要上锁，那么读进缓存内的数据也就没必要失效了。

而且，文章解释“核数”相对于“存放数据的叶子结点数量”来说很少，写操作只会针对某一个（或者几个），读读、读写、写写都不会产生太大冲突，所以这种方案是可行的。

#### 读写流程
```
Algorithm UpdateNode 
U1. Acquire latch. 
U2. Update the content. 
U3. Increment version. 
U4. Release latch.

Algorithm ReadNode 
R1. Copy the value of version into a register R. 
R2. Read the content of the node.
R3. Iflatch is locked, go to R1. 
R4. If the current value ofversion is different from the copied value in R, go toR1.
```

写操作其实还包含结点的分裂情况，作者在第 5 节也做了详细的说明。
