# multi_net_io库

> 源码位置：multi-master-tool/multi_net_io
>
> 此文档仅说明一些会实际使用的接口函数，未列出的函数还处于实验状态，需要谨慎使用。
>
> 部分简单的检查状态的函数（如`is_init`、`is_free`）也不在此另做说明

## 使用方法

在multi_net_io目录下执行`cmake .` 与`make`后，src目录下可编译出静态库，用于C++项目的链接。

调用接口只需要包含include目录下的头文件即可。

需要机器上已经安装了libevent库，或是编译器可以搜索到libevent的头文件，include目录下不包含的libevent的头文件。

## 设计目标

远程IO的RPC需要



## 主要模块
### Libevent Handle

> 头文件位置：multi_net_io/include/libevent_handle.h

这一模块的主要功能是处理网络的发送与接收，基于libevent的简单封装。

封装时通过c++标准的原子变量、互斥量与条件标量基本实现了线程安全。

#### 初始化与销毁

```c++
  bool init_handle(int port);
  bool free_handle();  
```

这两个函数负责了libevent handle对象的初始化与销毁，不使用C++原生的构造与析构函数的原因是考虑到libevent_handle对象的复用，以及将网络连接创建与销毁由手动控制更有利于理清调用逻辑。

1. `init_handle`函数初始化对象，其主要工作有：
	+ 创建监听事件的event_base对象，并启动一个线程用于监听事件，异步的通知事件的发生
	+ 设置本地监听端口
	+ 初始化连接监听器，并由事件循环线程负责处理连接事件，即对象初始化时已经启动监听线程，监听其它机器的连接请求
2. `free_handle`函数销毁对象，其主要工作有:
   + 退出事件监听循环，结束事件循环线程
   + 清除保存的连接状态

#### 尝试连接

```c++
int get_connection_id(const char * ip,const int port,bool tryConnect);
void get_connection_ip(const int id,char * ip);
int get_connection_port(const int id);
int get_connection_count();
```

对于连接的封装是一个令人困惑的地方，IP与端口的组合似乎有些冗余，主机名又需要另外的解析处理，而我对于DNS解析的细节并不是很了解。折中之后，我选择将连接封装成使用整数的id，使用id标识不同的ip与端口，是否需要主机名到ip的转换由调用者自行决定和实现。

`get_connection_id`使这一部分的核心函数，它接收了三个参数，前两个显而易见是ip与端口号，由此获得对应的连接id。当该ip与port未与本机建立连接时，有两种情况：

+ tryConnect参数为true时，会尝试与未连接的ip、端口建立连接，返回成功建立连接的id
+ tryConnect参数为false时，返回-1表示无此连接

后续是三个函数分别用于：

+ 根据连接id获取对方的ip地址
+ 根据连接id获取对方的端口
+ 根据连接id获取连接的数量（包含本地发起的连接与接收到的连接请求）

> 注：这里有一个小问题，发起连接时无法绑定端口，由系统随机分配空余的端口，即发送方可以确定对方的ip和端口，接收方收到的来自发送方的端口却是不确定的。这一过程对于调用者是透明，调用者可以理解为连接是单向，在指定的监听端口处理收到的网络包，发送时无需考虑本地端口问题。

####  处理连接

```c++
int get_listen_connection_count();
void get_listen_connection_array(int * array);
```

由于监听连接由独立的线程负责，并由预设的异步回调函数处理收到的连接请求，因此对于调用者来说，收到的连接的信息是未知的。

+ `get_listen_connection_count`返回监听线程处理的连接请求的数量

+ `get_listen_connection_array`返回一个数组，其中包含了所有监听线程处理的连接请求的id

#### 发送与接收

````c++
bool send(const int id,const char * send_bytes,const int send_size);

bool wait_recive(const int id,char * recive_bytes,int * recive_size=0);
int get_recive_buffer_length(const int id);
````

发起连接与接收连接都可以做到之后，就是数据的发送与接收。

`send`的逻辑很简单，向指定的连接发送数据，三个参数分别是连接id、数据数组的首地址、数据数组的长度。

`wait_recive`函数用于接收数据，其前两个参数不用多说，第三个参数recive_size略显特别，这是一个int指针，该指针的值会影响`wait_recive`函数的执行逻辑：

+ 建议使用recive_size参数的默认值0。因为send函数在发送时，会在数据前方加一个头信息，头信息中记录了数据包的大小。在recive_size默认为0的情况下，`wait_recive`函数会先读取一个头信息，获知包的大小后，函数会进入条件变量的阻塞，直到接收缓冲区中有足够的数据，读取整个包返回。
+ 当recive_size参数指定为一个有具体值的int变量的地址时，函数会读取指定值大小的数据后返回，而不管数据的格式。一般情况下，建议使用默认值。

`get_recive_buffer_length`函数用于获取接收缓冲区中未处理的数据大小。

### Multi Buffer

####  DynamicBuffer

> 已弃用，可忽略

动态扩容的buffer，内部实现是内存块链表，屏蔽了离散内存块的处理细节，给调用者使用连续空间的错觉。

参考Innodb迷你事务中的动态buffer实现。

#### FixedBuffer 

固定大小的buffer，实质就是对char数组做一个封装，实现了一些buffer需要的操作，目前主要用于记录json解析的结果。

### Json RPC Packet

该类主要负责了序列化与反序列化的功能，经过序列化将传入的参数序列化成一个json字符串用于传输，反序列化则将接收到的json字符串解析成对应的对象。

基于rapidjson库，号称最快的json解析库，使用了SAX模式的接口。

#### JSON对象的构造

```c++
void set_packet_header(const char *,int size);

void set_packet_item(const char * key,const char * val,int val_size,PacketItemType type = PacketItemType::NORMAL);

void set_note_for_ptr(const char * item_key,int offset,const char * val,int size,PacketItemType type = PacketItemType::NORMAL);
```

构造一个json包有几个步骤：

1. `set_packet_header`设置一个头部信息，这是第一步，头部信息的内容自行设计，例如：服务端的函数名或是枚举量，用于服务端自主选择调用的函数。
2. `set_packet_item`设置一个KV键值对，例如：参数名和其对应的值。
3. ` set_note_for_ptr`设置一个指针注释，item_key是指针所在的KV对中的K，offset是指针在KV对中的V中的相对偏移量，val是指针指向的值，size是指针指向值的大小。

这里有一个比较重要的枚举类型PacketItemType，它用于标志设置的item或是note是否是最后一个，有四种取值，分别是：

+ FIRST ：表示这是首个项
+ NORMAL：表示这是中间的一般项，默认值
+ LAST：表示这是末尾项
+ SINGLE：表示只有一个项，该项既是首项也是末尾项

设置时注意有头有尾，缺少首尾的调用方式可能会引起未知的错误。

#### JSON 字符串的生成

```c++
const char * get_string_ptr();
int get_string_length();
```

此处的两个函数作用很直接，获取json字符串和字符串的长度。

字符串长度这里有个注意点，rapidjson生成的json字符串不仅尾部有’\0‘，首部也有。若对字符指针直接求字符串长度，会得到0。因此，打包传输时注意确认是否整个json字符串都被发送。

#### 解析出内存对象

```c++
void parse(const char * json_str,char * buffer,int size);

char * get_packet_header_ptr();
int get_packet_header_size();
char * get_packet_item_ptr(int i);
int get_packet_item_size(int i);
char * get_packet_item_ptr(const char * key);
int get_packet_item_size(const char * key);
```



