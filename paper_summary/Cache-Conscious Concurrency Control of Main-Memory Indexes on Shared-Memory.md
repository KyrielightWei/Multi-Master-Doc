### Cache-Conscious Concurrency Control of Main-Memory Indexes on Shared-Memory Multiprocessor Systems

首尔大学发表在 2001 年 VLDB

### 背景 
文章主要关注多核处理器上内存数据库的索引优化。  
之前的研究提出了基于B+树改进的CSB+Tree来提高空间利用率（可以参考这篇文章：[内存数据库中B+树和CSB+树的性能比较](http://search.cnki.net/down/default.aspx?filename=TXSJ201512193&dbcode=CJFD&year=2015&dflag=pdfdown)），但是不管是B+tree或是CSB+tree，之前的并发控制方法都没有考虑多核 L2 缓存上块（或缓存行）失效的问题（cache miss）；多核并发访问索引时，由于 cache miss增多，性能随之严重下降。  

文章提出了新的并发控制方法 latch-free index traversal (OLFIT) ，这种方法能够有效减少 L2 缓存上（可以直接理解为核内缓存）的 cache miss。

### 造成cache miss的原因：
通常搜索一条记录时，单核单线程情况下根本不用考虑并发；为提高搜索效率，多核时使用lock coupling 控制方法（[索引并发(蟹行协议和B-Link树)](https://blog.csdn.net/popvip44/article/details/57468202) 中的蟹行协议）保障一致性。

既然是遍历、搜索记录，没有修改页面，为什么会产生缓存失效？  
考虑两种情况：1. 锁信息在树节点内部 2. 锁信息和节点分开。  
多核处理器在申请锁时，不管是共享锁还是排它锁，势必会对锁信息做修改，参照 MESI 协议，只要是核 p1 对某个缓存行（这里是内存里的锁信息）做修改，其他核 pi 都需要将自己核内的对应缓存行失效。这样，不管是 1 还是 2 的情况，其他核内缓存的相同数据块都不能使用，因为缓存的锁信息失效。

文中 3.1 节举了个简单例子来表明上述过程：
![figure1](https://github.com/PokemonWei/Multi-Master-Doc/blob/zhang-dev/summary/images/Cache-Conscious%20Concurrency%20Control%20of%20Main-Memory%20Indexes%20on%20Shared-Memory%20Multiprocessor%20Systems/figure1.png)
流程不再用文字描述。  
这个例子充分说明了凡是使用上锁的方法，都会产生cache miss 问题。

### 参照对象
2.3 节简要描述了几种并发控制方法，也是后面实验对比的对象：
1. lock coupling: 这种方法在遍历树时，先用共享锁锁住根节点。然后尝试获取子节点的共享锁，获得子节点共享锁后，释放父节点上的锁，重复这个过程知道搜索到叶节点。
> 参考 [索引并发(蟹行协议和B-Link树)](https://blog.csdn.net/popvip44/article/details/57468202)
2. tree-level locking：直接锁住整个树根，可以想象并发性能有多差。
3. physical versioning：这种方法读写都不用上锁，修改记录时，会申请一块内存，把旧版本页面拷贝到新页，在这块新页上做修改。后台有一个管理器负责合并新版本的合并；另外还有一个垃圾回收器，当旧版本页面没有人读时，会回收这块空间。

这几个方法存在的问题：1、2都使用了锁来控制并发，这样就会产生cache miss问题，3 虽然使用了多版本，但是修改时，会额外产生内存分配和拷贝的开销。

### 主要内容
文章提出的方法是，读操作采用多版本读，写操作使用锁的方法来控制并发。这里多版本并不是说创建多个副本，而是一个页面上的版本标志位。

如下图所示：
![figure1](https://github.com/PokemonWei/Multi-Master-Doc/blob/zhang-dev/summary/images/Cache-Conscious%20Concurrency%20Control%20of%20Main-Memory%20Indexes%20on%20Shared-Memory%20Multiprocessor%20Systems/figure3.png)

OLFIT 在B树节点的头部加入了 latch 和 version 字段，更新节点时，事务获取获取这个锁，然后更新节点，之后增加 version 的版本，最后释放锁；读操作读取这个节点时，防止在读的过程中被别人更新，会在事务开始读一次这个 version，然后在结束时再读一次这个 version，两次都相同说明没人修改过，若不同则重新读取。

采用这种基于 version 的读，就可以不用让多核内的同一缓存页面失效。  
另外一个值得注意的是，写操作需要对页面修改，这样不管是什么并发控制方法，修改必定会产生脏页，除修改页面的本核以外，其他核都应该将该页设置为无效状态。毫无疑问，这样肯定会造成一定的 cache miss。参照论文 3.2 节建模分析的结果，作者认为 OLFIT 方案只对叶节点上锁，而叶节点的数量相对于核数来说是一个大的多的量级，不同核对同一个页面做并发 读、写 的可能性是极低的，这样造成的cache miss也小的多。

### 实验
这部分没有细看，对比其他几种控制方法来讲，作者没有展示 OLFIT 在哪种实验场景下性能会弱，只是在更新场景下，性能会稍微低于 tree-level locking（貌似想不出很好的解释，为什么直接对树根上锁性能会好一点，作者也没有解释）。

### 总结
个人认为这种 多核缓存+内存 的架构，内存任然扮演着集中式管理器的作用，核对缓存行的修改都需要刷到内存，然后是其他核内相应的数据失效。

若是我们采用RAC方案，每个节点持有锁表的一部分，假设节点 N1 持有页面 PA 的锁，节点 N2 需要申请 PA 的共享锁，此时节点 N3 同样也能够获取 PA 的共享锁，并不像多核架构里面那样，需要将其他节点的共享锁失效。  
这里 N1 本就是 PA 锁的管理者，它很明确知道锁在哪些其他节点持有，它也是参与者之一；而 MESI 不一样，MESI 里多核是参与者，内存只是提供一致性的媒介，所以才需要“失效广播”这种操作。  
