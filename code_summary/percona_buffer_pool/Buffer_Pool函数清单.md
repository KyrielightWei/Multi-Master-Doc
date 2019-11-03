# Percona-Server函数清单
## buf0buf.cc
+ buf_block_alloc (->buf_LRU_get_free_block)：分配一个block，先从free_list中找，free_list中没有，则从lru_list末尾取一个到free_list
+ buf_block_init : 在buf_pool创建时初始化buffer控制块，主要内容是初始化各个成员变量和读写锁（由于buf_block_t中包含了buf_page_t成员变量，block init过程中也初始化了page）
+ buf_chunk_t方法实现 ：结构体声明在buf0buf.ic文件中，本文件中实现了两个函数，具体作用没有深入了解，大致上是建议内核对内存分配的一些细节进行调整（可能是建议dump到core file 和不建议）
+ buf_pool_t方法实现：分配chunk空间，释放chunk空间，对所有chunk调用madvise_dump或
madvise_dont_dump
+ buf_pool_update_madvise()：madvise系列方法....具体作用不明，检查innobase_should_madvise_buf_pool（）是否有修改，更新buf_pool_should_madvise
+ buf_chunk_init ：分配chunk中的空间，chunk->mem的空间组成是buf_block_t数组（假设包含n个）和对应的n个frame空间，初始化buf_block_t，并放入buf_pool的free_list
+ buf_chunk_not_freed：在chunk中找一个not-free的block，找到返回，未找到返回NULL
+ buf_pool_set_sizes
+ buf_pool_create : 初始化一个buf_pool实例 —— （1）初始化各种mutex；（2）初始化allocator；（3）初始化各种链表；（4）初始化buf_pool中的每个chunk；（5）创建page与buddy系统的page的控制块的哈希表；(6) 初始化flush域；（7）初始化lru和flush链表上使用的
hazard pointer
+ buf_pool_free_instance：释放buf_pool实例创初始化时分配的的资源
+ buf_pool_free：释放buf_pool全局数据结构
+ buf_pool_init：创建整个buf_pool，包含多个buf_pool_t实例，多线程初始化各个buf_pool实例（join等待所有初始化线程完成再返回函数）
+ buf_page_realloc: 重新分配控制块。从free_list中取一个空的控制块，重新定位LRU链表中的位置，重新定位unzip_LRU链表中的位置，重新定位page_hash，重新定位flush_list，设置buf_block_t的其他标志。
+ buf_resize_status：未看懂此函数作用，似乎不涉及主要功能
+ buf_block_will_withdrawn: withdrawn指buf_pool resize时，缩小buf_pool时有一些页需要回收，will_withdrawn指块位于需要回收的chunk中，新size~旧size范围中的chunk逐个搜索，block是否属于，在需要回收的chunk中则返回true，不在则返回false
+ buf_pool_withdraw_blocks：withdraw缓冲池里的blocks。（1）从free_list中搜索在withdraw范围的block；（2）从LRU链表中刷脏页，增加free_list的长度；（3）在withdrawn区域中重新定位block和buddies；（4）确认withdrawn是否足够，足够则不需要重试（尝试十次都没有完成withdrawn，则返回true，告知需要重试）
+ buf_pool_resize_hash：调整page_hash和zip_hash的大小。创建新的hash表，将原表中的数据逐个插入新的hash表，再释放原表的空间（page_hash会放入page_hash_old中保存）
+ buf_pool_resize：调整buf_pool的大小，由srv_buf_pool_size -> srv_buf_pool_old_size。（1）对所有resize的buffer pool设置新的限制；（2）失效自适应哈希；（3）设置withdraw target；（4）对于每个buf_pool调用buf_pool_withdraw_blocks()函数；（5）打印事务系统中正在运行的事务的锁信息；（6）如果should_retry_withdraw为真，则线程睡眠，等待1、2、4、8、10秒后重新withdraw；（7）获取所有哈希表的锁和buf_pool的互斥量；（8）由于此时控制块已经加入到withdraw列表，可以直接释放chunk空间；（9）控制块空间被释放，舍弃withdraw列表；（10）由于chunk数量改变，分配新的\*chunk数组空间，替换原本buf_pool的\*chunk；（11）若是resize的目标是扩大buf_pool，则分配新的chunk空间，并改变一些计数；（12）重新计算buf_pool的当前大小，并释放原有chunks数组【chunks_old】的空间；（13）设置实例中与size相关的参数；（14）若是新size与原size有两倍以上的差距，则需要重新调整page_hash与zip_hash的大小；（15）释放所有哈希表的锁与buf_pool的互斥量；（16）如果有两倍以上差距，重新调整lock_table、btr_search_sys、dict_sys的size；（17）调整插入缓冲的最大size；（18）如果resize之前为启用状态，则启动自适应哈希。
+ buf_resize_thread：调整buf_pool大小的线程执行函数。
+ buf_pool_clear_hash_index：清空buf_pool中所有页的自适应哈希。
+ buf_relocate：重新定位buffer控制块。将控制块内容复制到另一块内存区域，然后替代原本控制块在lru列表中的位置。只有状态为BUF_BLOCK_ZIP_DIRTY、BUF_BLOCK_ZIP_PAGE的page可以作为参数放入该函数。
+ HazardPointer类的实现：
    + set
    + is_hp
    + adjust —— 调整hp的值，当多个在同一个链表上的线程尝试删除hp时，将hp调整为指向前一个元素
    + LRUItr::start() 
    
  
+ buf_pool_watch_is_sentinel：检查page是否在buf_pool的watch数组中。
+ buf_pool_watch_set：为一个给定的将要读入的page增加watch，为purge thread回收空间使用，函数的意义没有看懂。
+ buf_pool_watch_remove：删除watch用的sentinel块，在替换为一个真实的块之前。将watch块从page_hash中删除，清空watch的状态。
+ buf_pool_watch_unset：如果页已经被读入，则停止watching。
+ buf_pool_watch_occurred：检查页是否已经被读入。
+ buf_page_make_young：将一个页置入LRU链表的起始处，这个高级别的函数可以用于防止一些重要的页被换出buf_pool。
+ buf_page_make_young_if_needed：如果页过于old，有被换出的风险，则将其置入LRU链表的起始处。
+ buf_page_set_file_page_was_freed：将指定页的file_page_was_freed设置为true。
+ buf_page_reset_file_page_was_freed：将指定页的file_page_was_freed设置为false。
+ buf_block_try_discard_uncompressed：尝试丢弃一个压缩页的非压缩frame。
+ buf_page_get_zip：获得一个压缩页用于读。若页不在buf_pool中，则从文件中读取。
+ buf_block_init_low：初始化控制块的一些域。
+ buf_zip_decompress：解压一个快，解压到block->frame。
+ buf_block_from_ahi：从自适应哈希索引(buf_chunk_map_reg)中获取一个block。
+ buf_pointer_is_block_field_instance：检测指针指向的是否是buf_pool实例中的block。
+ buf_block_is_uncompressed：确定一个块是否由buf_chunk_init()创建，内部调用的还是buf_pointer_is_block_field_instance
+ buf_wait_for_read：等待block被读入。方法是不断尝试获得block->lock，无法获得则线程睡眠20毫秒后再次尝试。
+ Buf_fetch_normal::get：调用lookup()搜索block，找不到则从文件中读取。
+ Buf_fetch_other::get：功能同上一个函数，搜索页，找不到从文件读取，多了一些状态判断
+ Buf_fetch<T>::lookup：根据page_id找到对应的page，未找到符合条件的则返回NULL。
+ Buf_fetch<T>::is_on_watch：为page_id调用buf_pool_watch_set。
+ Buf_fetch<T>::zip_page_handler：（1）页已经被buffer-fixed或IO-fixed，返回错误，之后重试;（2）将Bpage移动到新获取的free block，并解压。未修改fix_block的值。
+ Buf_fetch<T>::check_state：检查块的状态，只有（1）当类型为BUF_BLOCK_FILE_PAGE且不处于IO fixed状态（2）Block为压缩的并可以解压 时才返回DB_SUCCESS。
+ Buf_fetch<T>::read_page：将m_page_id对应的页从文件中读取到buf_pool。
+ Buf_fetch<T>::mtr_add_page：将block添加到Mini-transaction buffer。
+ Buf_fetch<T>::is_optimistic：检测m_mode是否为Page_fetch::IF_IN_POOL或Page_fetch::PEEK_IF_IN_POOL
+ Buf_fetch<T>::temp_space_page_handler：增加bufferfix的计数
+ Buf_fetch<T>::single_page：获取一个页，经过一系列状态验证，完成预读，加入mtr之后返回给调用者。
+ buf_page_get_gen：根据mode初始化构造对应的Buf_fetch对象，调用single_page()，并将返回值返回给上层。
+ buf_page_optimistic_get：执行了一系列锁的处理、加入mtr、防止被换出、预读，未达到指定条件的情况下返回true。
+ buf_page_try_get_func：通过表空间id和页序号尝试获得页，如果页不在buffer pool中，该函数不会加载，只返回NULL值。
+ buf_page_init_low：初始化控制块的一些域。
+ buf_page_init：初始化buf_pool中的一个page，block指针必须为调用线程私有。初始化block和block->page控制块，插入到buf_pool的page_hash中。
+ buf_page_init_for_read：初始化一个page用于读，以下场景函数不做任何工作：【1】页已经在buf_pool中；【2】指定一个仅读的ibuf页，并且该页不是一个ibuf页；【3】表空间已经被删除或正在被删除。
+ buf_page_create：根据page_id创建buf_block_t。若该页已经存在于buf_pool，则返回；如果不存在于buf_pool，则在此初始化。
+ buf_page_monitor：监视buffer page的读写活动。
+ buf_read_page_handle_error：unfix页，unlach页，从page_hash和LRU中删除。
+ buf_page_check_corrupt：没有注释，推测是返回页中的校验和check是否成功。
+ buf_page_io_complete：完成一个文件页的异步读写请求。（1）检查表空间是否存在；（2）检查页是否压缩页，是则解压页并check校验和；（3）如果页是未初始化的或者不在doublewrite buffer中，则页号与表空间好应该与block->frame中的记录相同；（4）验证page校验和并且此处page应该不在加密，如果仍然是加密状态，则解密失败，且整个表空间不可读；（5） goto 
 corrupt:的处理；（6）根据io_type分别处理读和写的情况。
+ buf_must_be_all_freed_instance：无返回值函数，buf_pool中的所有页都处于可替换的状态，若存在不可替换的页，则打印信息并停止mysql_server。
+ buf_refresh_io_stats：刷新统计信息，用于打印每秒平均值。
+ buf_pool_invalidate_instance：使得buf_pool中的文件页失效。（1）等待所有类型的flush完成；（2）调用buf_must_be_all_freed_instance检查是否所有页都处于可替换状态；（3）扫描LRU，free所有blocks；（4）调整buf_pool参数值，重置统计信息。
+ buf_pool_invalidate：当一次存档恢复完成时，失效buffer pool中的所有文件页。对所有的buf_pool实例调用buf_pool_invalidate_instance。
+ buf_pool_validate_instance：确认一个buffer pool实例中的数据。由DEBUG宏控制函数是否启用。
+ buf_validate：对每个buf_pool实例调用buf_pool_validate_instance。由DEBUG宏控制函数是否启用。
+ buf_print_instance：打印buf_pool实例的信息。由DEBUG宏控制函数是否启用。
+ buf_print：对每个buf_pool实例调用buf_print_instance。由DEBUG宏控制函数是否启用。
+ buf_get_latched_pages_number_instance：返回latched页的数量。由DEBUG宏控制函数是否启用。
+ buf_get_latched_pages_number：对每个buf_pool实例调用buf_get_latched_pages_number_instance。由DEBUG宏控制函数是否启用。
+ buf_get_n_pending_read_ios：返回buf_pool的pending读操作。
+ buf_get_modified_ratio_pct：返回buffer pool中修改的页/所有页的百分比。
+ buf_stats_aggregate_pool_info：整合一个buf_pool的统计信息到total统计信息。
+ buf_stats_get_pool_info：收集一个buffer pool实例的统计信息到数组中。
+ buf_print_io_instance：打印buffer IO的信息到指定文件。
+ buf_print_io：调用buf_print_io_instance打印总和、单独的buffer IO信息。
+ buf_pool_check_no_pending_io：检查buffer pool当前无pending的IO操作。
+ 重载buf_pool_t的<<符号：打印给定的buf_pool_t对象。
+ get_page_type_str：获得page类型，并返回成const char * 。
+ buf_pool_free_all：释放所有的buf_pool实例和全局数据结构。

## buf0flu.cc

+ incr_flush_list_size_in_bytes：依据block的大小增加buf_pool中记录的flush list大小。
+ buf_flush_insert_in_flush_rbt：插入一个page到flush红黑树中，并返回前一个节点的值的指针。根据buf_pool_t的注释，红黑树是在恢复时用于加速flush_list的插入的。
+ buf_flush_delete_from_flush_rbt：从flush红黑树中删除一个页。
+ buf_flush_block_cmp：比较buffer pool中两个修改过的block。比较的keys是<oldest_modification, space, offset>，该函数用于红黑树对块排序。注意使用红黑树的目的，实际上我们仅需要按照oldest_modification排序即可，后面两个域用于区分块，只有>、<，没有==。（输入参数是偶两个buf_page_t的指针的指针）
+ buf_flush_init_flush_rbt：初始化红黑树，用于在恢复过程中加速flush_list的插入，每个buf_pool实例创建一个红黑树。
+ buf_flush_free_flush_rbt：销毁红黑树。
+ buf_are_flush_lists_empty_validate：检查所有buf_pool实例，flush_list是否为空。
+ buf_flush_list_order_validate：检查flush_list中的两个相邻页顺序是否有效。仅用于assertion。
> flush list中存在一个松散的顺序，最早加入的页的oldest_modification不大于所有脏页的最小
oldest_modification，这一松散顺序是为了保证下一个检查点的计算

+ buf_flush_borrow_lsn：从最近加入flush list的脏页“借”一个lsn。
+ buf_flush_insert_into_flush_list：插入一个脏页到flush_list。（1）检查是否处于恢复过程中，是则需要更新flush红黑树；（2）如果lsn为0，则将页的newest_modification更新为“借”的lsn和日志系统的flushed_to_disk_lsn中较大的那个；（3）page插入到flush_list的首部。
+ buf_flush_insert_sorted_into_flush_list：将脏页插入flush_list的正确的位置。此函数用于恢复。当红黑树存在时，插入红黑树获取前一个pag指针，红黑树不存在，则搜索flush_list找到合适的位置。
+ buf_flush_ready_for_replace：如果此file page可以立即替换，则返回true。传入的page参数必须是buf_page_in_file()且位于LRU链表中。可以立即替换的条件—— `bpage->oldest_modification == 0 && bpage->buf_fix_count == 0 &&buf_page_get_io_fix(bpage) == BUF_IO_NONE`

+ buf_flush_ready_for_flush：检查block是否是脏页，且准备flush。
+ buf_flush_remove：从flush_list中删除block。
+ buf_flush_relocate_on_flush_list：重定位flush_list中的buffer控制块。该函数假设bpage的内容已经拷贝到dpage中。用dpage替换bpage在flush_list与红黑树中的位置。
+ buf_flush_write_complete：写操作完成时，更新flush系统中的数据结构。（1）从flush_list中删除页；（2）修改io_fix状态；（3）减少flush_type计数；（4）flush batch若是终结，则发出广播，通知线程；【os_event是mysql中的条件变量】（5）更新double write buffer。
+ buf_flush_update_zip_checksum：计算压缩页的校验并更新页。更新校验和和lsn。
+ buf_flush_init_for_writing：为了写入到表空间，初始化页。（1）若页是压缩的，根据文件页类型处理压缩数据（拷贝与否），更新lsn和checksum后函数返回；（2）若页并非压缩，则将newest_lsn写入到页的头部和尾部；（3）是否跳过校验和检验，是则跳过；
+ buf_flush_write_block_low：对buffer page做一个异步写。当使用模拟的aio和使用double write buffer时，必须在发出一组写请求后调用buf_dblwr_flush_buffered_writes。（1）先刷日志直到newest_modification；（2）写前初始化，压缩页和非压缩页处理方式不同；（3）关闭临时表空间的double-write buffer，因为会在flush期间增加开销，非临时表空间仍然通过double write buffer写入；（4）对于单个页，BUF_FLUSH_SINGLE_PAGE类型，sync参数为true，使用同步IO将正在处理的表空间的修改刷到磁盘。
+ buf_flush_page：异步的将一个页刷到文件中。（1）处理各种互斥量和前提条件的准备；（2）压缩页直接可以flush，非压缩页需要获取rw_lock；（3）如果可以执行flush，则设置io_fix标志、设置flush类型、修改buf_pool中记录的信息等系列处理后，调用buf_flush_write_block_low函数。
+ buf_flush_page_try：debug函数，调用buf_flush_page刷一个BUF_FLUSH_SINGLE_PAGE类型的页。
+ buf_flush_check_neighbor：根据page_id检查一个页是否在buffer pool中，且可以被flush。
+ buf_flush_try_neighbors：将flush区域中的所有可flush的页刷到磁盘。（1）根据情况计算flush区域的上下界，上下界之间仅包含可刷的页【srv_flush_neighbors决定是否刷附近的页，这一机制与预读类似，刷页时将邻近区域可刷的页一起刷盘】；（2）经过一系列判断后，对上下界间每一个满足条件的也调用buf_flush_page，并对成功的页计数作为返回值。
+ buf_flush_page_and_try_neighbors：检查块是否是脏页且准备flush，如果是则刷页并尝试刷邻近的页。buf_flush_try_neighbors的上层函数。
+ buf_free_from_unzip_LRU_list_batch：移动未压缩的frame到free list，该函数实际不刷任何数据到磁盘，只是将unzip_LRU尾部压缩页的未压缩frame拆卸到free list。
+ buf_flush_LRU_list_batch：从LRU list的末尾刷脏页。可以替换的页直接从LRU list移动到free list，可以刷盘的页执行邻近刷盘，刷盘结束后IO helper线程会将其放入free list。
+ buf_do_LRU_batch：刷盘并将页从LRU或unzip_LRU移动到free list。系统状态决定从LRU还是从unzip_LRU。
+ buf_do_flush_list_batch：从flush list的尾部刷脏页。batch刷。
+ buf_flush_batch：从LRU list或flush list末尾刷脏页。BUF_FLUSH_LRU则从LRU末尾刷脏页，BUF_FLUSH_LIST则从flush list末尾刷脏页。
+ buf_flush_start：开始执行flush batch，在LRU或flush list上。（1）标志init_flush[flush_type]为true表示已经开始该类型的flush batch；（2）os_event_reset(buf_pool->no_flush[flush_type])。
+ buf_flush_end：终止一个flush batch。（1）设置init_flush[flush_type]为false；os_event_set(buf_pool->no_flush[flush_type])；（2）调用buf_dblwr_flush_buffered_writes刷缓冲的写操作或唤醒模拟的aio线程完成工作。
+ buf_flush_wait_batch_end：等待直到给定类型的flush batch结束。buf_pool参数为NULL时，则等待所有buf_pool实例。os_event_wait
+ buf_flush_do_batch：执行给定类型的flushing batch。buf_flush_start -> buf_flush_batch -> buf_flush_end。
+ buf_flush_lists：所有的buffer pool实例从flush list的尾部刷脏页。
+ buf_flush_single_page_from_LRU：从LRU list的末尾刷一个页，从page_hash与free list中删除，并添加到free list。可替换直接替换，可刷先刷，再置入free list。
+ buf_flush_LRU_list：清理给定buf_pool实例的LRU list尾部。（从LRU尾部将可替换的页放入free list；将脏页从LRU的尾部刷到磁盘），扫描每个buffer pool的深度由动态配置参数innodb_LRU_scan_depth控制。
+ buf_flush_wait_LRU_batch_end：等待所有buf_pool实例的LRU flush结束。
+ af_get_pct_for_dirty：基于脏页数量计算是否需要执行flushing，返回管理脏页比需要的io_capacity的百分比。
+ af_get_pct_for_lsn：基于生成redo的速率计算是否需要flushing，返回管理redo空间需要的io_capacity的百分比。似乎与 adaptive flushing有关系。
+ page_cleaner_flush_pages_recommendation：由page_cleaner线程几乎每秒一次的调用，基于多种因素判断是否有执行flush的需要。返回值时推荐flush的页数量。
+ pc_sleep_if_neede：如果page_cleaner线程在一秒内完成工作，则睡眠。
+ buf_flush_page_cleaner_init：初始化page cleaner，参数是将要创建的page cleaner线程的数量。（1）分配并初始化全局的page_cleaner_t，每个buf_pool一个page_cleaner_slot;（2）创建page_coordinator线程，执行buf_flush_page_coordinator_thread函数，传入参数为将要创建的page cleaner线程的数量；（3）每个buf_pool创建一个LRU_manager线程；（4）为了确保page cleaner线程和LRU manager线程激活，当前线程睡眠1000微秒。
+ buf_flush_page_cleaner_close：关闭page_cleaner。（1）睡眠1000微秒，等待所有工作线程退出；（2）销毁slots；（3）销毁“完成”和“请求”的os_event；（4）销毁page_cleaner对象。
+ pc_request：请求所有的slots，flush所有的buffer pool实例。设置好slot的状态，请求slot的数量等变量后，向page_cleaner->is_requested发出信号，唤醒线程。
+ pc_flush_slot：flush一个slot。（1）在slots数组中找到一个正在被请求的；（2）改变状态和计数，做一些正确性检查，调用buf_flush_do_batch刷slot收到的请求的页；（3）改变计数，若flushing计数和requested计数为0，则发出信号，通知线程is_finished。
+ pc_wait_finished：等待直到所有的flush请求完成。（1）等待is_finished信号；（2）重置page_cleaner和所有slot的参数。
+ buf_flush_page_cleaner_disabled_loop：debug函数，用于使得page cleaner和LRU manager线程失效。[没细看]
+ buf_flush_page_cleaner_disabled_debug_update：debug函数，使page cleaner线程（coordinator和worker）和LRU manager线程失效。[没细看]
+ buf_flush_page_coordinator_thread：负责刷脏页的线程，仅有一个coordinator线程，输入参数是将要创建的page cleaner线程的数量。（1）初始化线程；（2）创建n_page_cleaners-1个工作线程，因为coordinator也算作一个page cleaner线程；（3）处理恢复过程中的flushing请求；（4）等待buf_flush_event；（5）开始循环，直到服务进程关闭；（6）根据需求使线程睡眠，获得ret_sleep，可能为0，为0即跳过睡眠；（7） 如果睡眠时间未超时且有flush同步需求，则发出刷全部脏页的请求，coordinator也会以slot为单位处理请求，等待所有slot完成请求；（8）else if (srv_check_activity(last_activity))，发出刷推荐数量或0个脏页的请求，coordinator也会以slot为单位处理请求，等待所有slot完成请求；（9）else if(睡眠时间超时超时)，从所有buf_pool实例的flush list末尾刷脏页；（10）else 什么也不做，无activity，被event唤醒；（11）结束循环；（12）若需要fast shutdown，则直接跳到thread_exit代码处；（13）注释：一般和缓慢shutdown场景下，page_cleaner线程需要等待服务器中所有其它线程结束；shutdown的第一阶段SRV_SHUTDOWN_CLEANUP其它线程可能还在工作，第二阶段SRV_SHUTDOWN_FLUSH_PHASE可以确定不会再有新的脏页；LRU manager线程在第一阶段仍然在flush，而第二阶段中止。（14）进入正常shutdown流程，发出刷所有页的请求，以slot为单位处理请求，等待所有flush请求完成，睡眠10000微秒，循环直到状态不再是SRV_SHUTDOWN_CLEANUP且lru manager 线程全部退出；（15）再循环一次....直到flush成功且无页可刷结束循环，注释中说明是为了最后清理buffer pool的flush_list；（16）thread_exit部分：唤醒所有工作线程，使其退出，销毁当前线程。
+ buf_flush_page_cleaner_thread：page_cleaner的工作线程。（1）初始化线程，增加page_cleaner的n_workers计数；（2）开始线程循环（3）等待被page_cleaner->is_requested事件唤醒；（4）如果page_cleaner->is_running为false，跳出循环；（5）以slot为单位执行flush；（6）循环结束，减少n_worker计数，销毁当前线程。
+ buf_flush_sync_all_buf_pools：刷flush list中所有可刷的页，并等待完成。（注释中提到唤醒了page cleaner线程，源码中并未看到，个人认为是一个错误的注释）
+ buf_lru_manager_sleep_if_needed：使得LRU manager线程睡眠到指定时间。
+ buf_lru_manager_adapt_sleep_time：基于上次flush结果和free list长度调整LRU manager线程的睡眠时间。
+ buf_lru_manager_thread：LRU manager线程的逻辑，每个buf_pool实例创建一个此线程，任务是实施LRU list中脏页的flush和驱逐到free list。（1）初始化线程，增加LRU manager线程计数；（2）启动线程循环，直到服务进程开始shutdown且离开SRV_SHUTDOWN_CLEANUP阶段；（3）调用buf_lru_manager_sleep_if_needed和buf_lru_manager_adapt_sleep_time；（4）调用buf_flush_LRU_list刷页，调用buf_flush_wait_batch_end等待flush完成，从源码逻辑上调用buf_flush_wait_batch_end毫无意义，同一个线程下先对条件变量发出信号，再wait一下，这在mysql的os_event中是允许的【由于signal_count的存在】，但这样没有任何逻辑上意义，只能理解为提高代码的可读性；（5）线程循环结束，减少线程计数，销毁当前线程。
+ buf_flush_request_force：调整buf_flush_sync_lsn，唤醒page cleaner线程。
+ buf_flush_validate_low：debug函数，验证flush list。
+ buf_flush_validate：buf_flush_validate_low的封装。
+ buf_flush_get_dirty_pages_count：获取所有buf_pool实例的指定表空间有多少脏页。
+ FlushObserver的实现：没有看懂该类的作用，在flush模块中基本没有调用，注释中的说明是应用在BtrBulk.cc文件。

## buf0lru.cc
+ BUF_LRU_OLD_TOLERANCE（20）：LRU_old指针前面的块的数量，包含指向的块本身，必须是buf_pool->LRU_old_ratio/BUF_LRU_OLD_RATIO_DIV * 整个LRU list的长度。该宏定义的是允许的差值，这个值必须足够小，LRU_old不允许指向LRU list的任何一个边缘。
+ buf_LRU_old_threshold_ms：移动到new lru list的前提是第一次访问至少在这么多毫秒以前。
+ incr_LRU_size_in_bytes：根据page的大小增加LRU大小的统计信息。
+ buf_LRU_evict_from_unzip_LRU：判断是否应该从unzip_LRU而不是一般的LRU中换出页。其中有一套启发式的计算规则，主要与负载是cpu bound还是IO bound有关。
+ buf_LRU_drop_page_hash_batch：尝试删除属于某个表空间的一组page hash索引。
+ buf_LRU_drop_page_hash_for_tablespace：删除一个表空间的所有page hash索引。只起到“尽可能”效果，未必全部删除。
+ buf_flush_yield：刷一个表空间的脏页时，我们不希望占用CPU和资源。内部调用了c++11的函数std::this_thread::yield()当前线程“放弃”执行，让操作系统调度另一线程继续执行。具体的实现逻辑：（1）释放各种mutex，设置page为IO_PIN；（2）调用yield（）；（3）重新获得各种mutex，设置page为IO_NONE。
+ buf_flush_try_yield：如果已经占用资源很久，则释放LRU list和flush list的mutex，并执行yield（）。
+ buf_flush_or_remove_page：在给定的buf_pool实例中的给定表空间中删除一个页。flush参数决定是直接删除还是刷了再删除。
+ buf_flush_or_remove_pages：删除或刷盘再删除给定buf_pool实例中给定表空间中的所有脏页。
+ buf_flush_dirty_pages：循环调用buf_flush_or_remove_pages，失败则线程睡眠2000微秒后继续循环，直到成功。
+ buf_LRU_remove_all_pages：删除特定buf_pool实例特定表空间的所有页。
+ buf_LRU_remove_pages：根据buf_remove_t参数选择不同的函数调用，实现删除指定buf_pool指定表空间的页。
+ buf_LRU_flush_or_remove_pages：刷或者删除指定表空间的所有页。循环每个实例调用buf_LRU_remove_pages，BUF_REMOVE_ALL_NO_WRITE还需要删除page hash。
+ buf_LRU_insert_zip_clean：debug函数，以LRU顺序插入一个压缩的块到zip_clean。
+ buf_LRU_free_from_unzip_LRU_list：尝试从unzip LRU list释放一个压缩块的非压缩页，压缩页保留并且不需要clean。内部调用了buf_LRU_free_page。
+ buf_LRU_free_from_common_LRU_list：尝试从一般的LRU中free一个clean的页。
+ buf_LRU_scan_and_free_block：尝试free一个可替换的block。先调用buf_LRU_free_from_unzip_LRU_list，返回false再调buf_LRU_free_from_common_LRU_list。
+ buf_LRU_buf_pool_running_out：如果任意实例中只有小于25%的buffer pool可用时返回true。可用的定义：free_list长度+lru_list长度。
+ buf_LRU_get_free_only: 从free_list中取出一个空的控制块。（1）从free_list头部取一个block；（2）若是成功取到，则从free_list中删除block；（3）若block不会withdraw，则返回该block；（4）若block应该withdraw，则加入withdraw链表，然后重新从free_list中获取block；（5）若free_list中无法获取到新block，则返回NULL。
+ buf_LRU_check_size_of_non_data_objects：检查多少个buf_pool被非数据对象占用，非数据对象指的是AHI、lock heap等等。只有二十分之一和三分之一的可用空间时，会报警。
+ buf_LRU_handle_lack_of_free_blocks：诊断获取free页时的错误，如果已经过去两秒，则请求InnoDB monitor在错误日志输出。
+ buf_LRU_get_free_block
    + 注释
    从buf_pool中返回一个free block。该block从free list中获取，若是free list为空，则从LRU list的末尾移动到free list。该函数当用户线程需要一个clean块存放读入的页时调用。
        + iteration 0：
            + 从free list中获取一个块，成功则结束
            + 如果设置了buf_pool->try_LRU_scan，依据扫描LRU找到一个clean block，放入free list，在free list中重新获取块
            + 将LRU尾部的脏页刷到磁盘，将页放入 free list，在free list中重新获取块
        + iteration 1：
            + 与iteration 0基本相同，区别是free list不够时一定会扫描整个LRU list
        + iteration 2：
            + 与iteration 1相同，但会睡眠10ms
    + 源码逻辑
    此处有两种处理free list为空的算法：
        + 一种是mysql 5.6中的算法（1）当使用dblwr且后台正在执行LRU flushing的场景下，等待flush完成；（2）除此之外，传入参数调用buf_LRU_scan_and_free_block；（3）如果扫描整个LRU无法获得free block，则睡眠等待page_cleaner线程刷一个batch；（4）如果仍然无法找到free block，则尝试从LRU刷一个脏页，放入free list。
        + 另一种是percona 5.6中的，睡眠等待cleaner线程产生新的free页。
+ buf_LRU_old_adjust_len：移动LRU_old指针，使得old list的长度在允许的范围内。若old_len较小，则前移old指针，增加长度；若old_len较大，则后移old指针，减少长度。移动过程中遇到的block需要改变old状态。
+ buf_LRU_old_init：初始化LRU list中的old指针，该函数需要当LRU list达到BUF_LRU_OLD_MIN_LEN长度时调用。（1）先将整个LRU list设置为old；（2）调用buf_LRU_old_adjust_len将old指针调整到正确的位置。
+ buf_unzip_LRU_remove_block_if_needed：如果参数表示的页在unzip_LRU list中，则从list中移除。
+ buf_LRU_adjust_hp：如果有需要，调整LRU hazard的值，调用LRU hazard的adjust函数。
+ buf_LRU_remove_block：从LRU list中删除一个block。（1）重要：删除前尝试调整LRU hazard的值；（2）如果定义了old指针，且恰好指向该block，则移动到前一个block处；（3）从LRU list中删除block，并尝试从unzip LRU中尝试删除；（4）如果LRU list的长度短到不足以定义LRU_old，则清除old标志；（5）更新old list长度；（6）如果有必要，调整old指针的位置。
+ buf_unzip_LRU_add_block：添加一个块到LRU list，此块已经解压。如果是old，则添加到unzip_LRU的末尾，如果非old，则添加到unzip_LRU的头部。
+ buf_LRU_add_block_low：添加块到LRU list。（1）如果非old块或LRU_old未定义，则添加到LRU_list的首部，否则添加到LRU_old的后一个位置，即old list的首部；（2）如果插入后长度大于BUF_LRU_OLD_MIN_LEN，则尝试调整old指针位置；（3）如果插入后长度等于BUF_LRU_OLD_MIN_LEN，则初始化old list；（4）如果插入后长度小于BUF_LRU_OLD_MIN_LEN，则根据LRU_old是否存在，设置old标志；（5）如果块是一个带有解压frame的压缩块，则放入unzip_LRU list。
+ buf_LRU_add_block：buf_LRU_add_block_low的封装。
+ buf_LRU_make_block_young：将一个块移动到LRU list的开头。
+ buf_LRU_free_page：尝试free一个block，如果bpage是一个仅压缩的页，描述符对象也会被free。如果页是一个带有非压缩frame的压缩页，则仅释放非压缩frame。具体源码逻辑是将bpage的内容拷贝到一个新分配的buf_page_t结构中（并非从buf_pool中分配，而是直接从堆分配），用新的page替换bpage在LRU list和flush list中的位置，将bpage所在的block从list和page hash中删除，放入到free list中。（非压缩页的frame存放于block结构中，上层没有任何block结构包含的page自然只剩下了压缩页）。
+ buf_LRU_block_free_non_file_page：将一个block放入free list。只能处理BUF_BLOCK_MEMORY和BUF_BLOCK_READY_FOR_USE类型的页，将其内容清空之后，放入withdrawn list或是free list。
+ buf_LRU_block_remove_hashed：将一个block从LRU list和page hash中取出，如果block是仅压缩的【BUF_BLOCK_ZIP_PAGE】，则free 对象。
+ buf_LRU_block_free_hashed_page：将某个file page（没有hash index）放入free list。（1）改变block的状态为BUF_BLOCK_MEMORY；（2）调用buf_LRU_block_free_non_file_page。
+ buf_LRU_free_one_page：从LRU list删除页，放入free list。先调用buf_LRU_block_remove_hashed，再调用buf_LRU_block_free_hashed_page。
+ buf_LRU_old_ratio_update_instance：更新一个buf_pool实例的LRU_old_ratio。
+ buf_LRU_old_ratio_update：更新每个buf_pool实例的LRU_old_ratio。
+ buf_LRU_stat_update：更新历史统计信息。
+ buf_LRU_validate_instance：验证一个buffer pool实例的LRU list。
+ buf_LRU_validate：验证每个buffer pool实例的LRU list。
+ buf_LRU_print_instance：打印一个buffer pool实例的信息。
+ buf_LRU_print：打印所有buffer pool实例的LRU list。


+ buf_LRU_block_remove_hashed：从LRU list和page hash table中取出block，如果该block是BUF_BLOCK_ZIP_PAGE，则free该对象。

## buf0buddy.cc
> 此部分只列出一些重要的函数
##### buf_buddy_is_free
假设buf_buddy_alloc()分配的内存全部用于压缩页的frames。
观察分配的对象内部，space_id域的值存在以下三种情况：
+ BUF_BUDDY_STAMP_FREE：block在zip_free链表中
+ BUF_BUDDY_STAMP_NONFREE：block已经被分配但还未初始化
+ 压缩表空间的一个可用的表空间的id
 
 stamp首部的size的值是zip_free index
+ 注意：仅有当buf已经被free时，我们才可以安全的读取stamp.size
##### buf_buddy_alloc_zip
尝试从zip_free中分配一个block。

在zip_free链表中找到i大小的块，若是失败，则递归调用自己，去i+1链表中搜索可用的块。

##### buf_buddy_block_free
释放UNIV_PAGE_SIZE大小的buffer frame。
将buf指向的空间全部清0，将buf对象的bpage对象放入free list。

##### buf_buddy_block_register
分配一个buffer block。
将block插入到buf_pool->zip_hash中。

##### buf_buddy_alloc_from
从一个更大的对象中分配block。

##### buf_buddy_alloc_low
目标：分配一个block。
1. 调用buf_buddy_alloc_zip，返回的buf_buddy_free_t \*直接转换为buf_block_t \*，进入func_exit
2. zip_free 链表分配失败时，从free list中获取一个block，分配到block则进入alloc_big
3. alloc_big: 调用buf_buddy_block_register注册到zip_hash，调用buf_buddy_alloc_from从block的frame空间中分配buf_block_t 
4. func_exit: 返回block，退出函数

##### buf_buddy_relocate
与lru和flush中的relocate不同，不需要修改链表，将page->zip.data改为指向dst空间，将src空间销毁

##### buf_buddy_free_low
释放一个block
有合并邻近的budyy为一个大的块，用一个合适大小的块替代部分利用的块的功能

##### buf_buddy_realloc
重新分配一个block

##### buf_buddy_condense_free
合并所有free的buddies##### buf_buddy_is_free
假设buf_buddy_alloc()分配的内存全部用于压缩页的frames。
观察分配的对象内部，space_id域的值存在以下三种情况：
+ BUF_BUDDY_STAMP_FREE：block在zip_free链表中
+ BUF_BUDDY_STAMP_NONFREE：block已经被分配但还未初始化
+ 压缩表空间的一个可用的表空间的id
 
 stamp首部的size的值是zip_free index
+ 注意：仅有当buf已经被free时，我们才可以安全的读取stamp.size
##### buf_buddy_alloc_zip
尝试从zip_free中分配一个block。

在zip_free链表中找到i大小的块，若是失败，则递归调用自己，去i+1链表中搜索可用的块。

##### buf_buddy_block_free
释放UNIV_PAGE_SIZE大小的buffer frame。
将buf指向的空间全部清0，将buf对象的bpage对象放入free list。

##### buf_buddy_block_register
分配一个buffer block。
将block插入到buf_pool->zip_hash中。

##### buf_buddy_alloc_from
从一个更大的对象中分配block。

##### buf_buddy_alloc_low
目标：分配一个block。
1. 调用buf_buddy_alloc_zip，返回的buf_buddy_free_t \*直接转换为buf_block_t \*，进入func_exit
2. zip_free 链表分配失败时，从free list中获取一个block，分配到block则进入alloc_big
3. alloc_big: 调用buf_buddy_block_register注册到zip_hash，调用buf_buddy_alloc_from从block的frame空间中分配buf_block_t 
4. func_exit: 返回block，退出函数

##### buf_buddy_relocate
与lru和flush中的relocate不同，不需要修改链表，将page->zip.data改为指向dst空间，将src空间销毁

##### buf_buddy_free_low
释放一个block
有合并邻近的budyy为一个大的块，用一个合适大小的块替代部分利用的块的功能

##### buf_buddy_realloc
重新分配一个block

##### buf_buddy_condense_free
合并所有free的buddies