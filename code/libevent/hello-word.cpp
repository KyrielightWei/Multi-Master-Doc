/*
	来源 ：libevent/sample/
*/

/*
  This example program provides a trivial server program that listens for TCP
  connections on port 9995.  When they arrive, it writes a short message to
  each client connection, and closes each connection once it is flushed.
  在9995端口上监听，当有连接建立时，给客户端传一个hello word，刷新后关闭每个连接

  Where possible, it exits cleanly in response to a SIGINT (ctrl-c).
*/


#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

//服务器发送给客户端的信息
static const char MESSAGE[] = "Hello, World!\n";

//端口号
static const int PORT = 9995;

static void listener_cb(struct evconnlistener*, evutil_socket_t,struct sockaddr*, int socklen, void*);//监听函数
static void conn_writecb(struct bufferevent*, void*);//写回调函数
static void conn_eventcb(struct bufferevent*, short, void*);//其他事件回调函数
static void signal_cb(evutil_socket_t, short, void*);//信号事件回调函数

int main(int argc, char** argv)
{
	struct event_base* base;//event_base对象，跟踪挂起或活动的事件，将其返回给应用程序
	struct evconnlistener* listener;//监听
	struct event* signal_event;//信号事件
	struct sockaddr_in sin;//IPV4 socket address

//如果是windows环境，需要用WSAstarup初始化 socket
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	base = event_base_new();//创建event_base实例
	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);

	listener = evconnlistener_new_bind(base, listener_cb, (void*)base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,(struct sockaddr*) & sin,	sizeof(sin));
	/*	base 用于监听连接的event_base
		evconnlistener_cb 回调函数 当其为NULL时这个监听被禁用直到函数建立
		ptr 指针传递给回调函数
		flags 标志，控制回调函数的行为
		backlog -1代表合理的设定值
		sa 监听地址 
		socklen 地址长度	*/

	/*	backlog--
		TCP建立连接阶段，在内核中维护两个队列：未完成队列和完成队列
		backlog历史上被定义为两个队列之和	 */

	/*	flags--
		LEV_OPT_CLOSE_ON_FREE：释放监听器时会把底层的套接字也关闭
		LEV_OPT_REUSEABLE：标记套接字是可重用的，这样一旦关闭，可以立即打开其他套接字，在相同端口进行监听
		LEV_OPT_LEAVE_SOCKETS_BLOCKING	默认情况下，监听器收到连接后将设置为非阻塞的，该标记禁用此行为
		LEV_OPT_CLOSE_ON_EXEC 设 the close-on-exec
		LEV_OPT_THREADSAFE 加锁
		LEV_OPT_DISABLED 监听器以禁用状态建立，后期要通过evconnlistener_enable()启用
		LEV_OPT_DEFERRED_ACCEPT 延迟accept()连接，直到数据可用
		LEV_OPT_REUSEABLE_PORT 多个服务器（进程/线程）绑定到相同端口
		LEV_OPT_BIND_IPV6ONLY 监听器只工作在IPv6 socket*/

	if (!listener) {
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}
	//当SIGINT(interrupt) signal事件到来时需要做什么
	signal_event = evsignal_new(base, SIGINT, signal_cb, (void*)base);
	//event_new(base,SIGINT,EV_SIGNAL|EV_PERSIST,signal_cb,(void*)base) signal_cb回调函数

	if (!signal_event || event_add(signal_event, NULL) < 0) {
		//event_add NULL:一直等待 return -1出错，0成功
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}

	event_base_dispatch(base);//事件调度回路，循环

	evconnlistener_free(listener);//释放listener
	event_free(signal_event);//释放event
	event_base_free(base);//释放base

	printf("done\n");
	return 0;
}

//有新链接到来时的回调函数
static void listener_cb(struct evconnlistener* listener, evutil_socket_t fd,
	struct sockaddr* sa, int socklen, void* user_data)
/*	typedef void (*evconnlistener_cb)(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int socklen, void *);
	*/
{
	struct event_base* base = (struct event_base*)user_data;//传过来的base
	struct bufferevent* bev;//缓存IO bufferevent由一个底层的传输端口（如套接字），一个读取缓冲区和一个写入缓冲区组成。
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);//创建一个缓冲区 BEV_OPT_CLOSE_ON_FREE释放bufferevent时关闭底层..
	if (!bev) {
		fprintf(stderr, "Error constructing bufferevent!");
		event_base_loopbreak(base);//立即退出
		return;
	}
	bufferevent_setcb(bev, NULL, conn_writecb, conn_eventcb, NULL);
	//缓冲区的回调函数：bufev,readcb读回调函数,writecb写回调函数,eventcb事件处理函数,回调函数的参数
	bufferevent_enable(bev, EV_WRITE);//启用bufferevent，写功能
	bufferevent_disable(bev, EV_READ);//禁用bufferevent，读功能
	bufferevent_write(bev, MESSAGE, strlen(MESSAGE));//向缓冲区写“hello word”
}

//写回调函数，写了heloword后在服务器端显示flush answer
static void conn_writecb(struct bufferevent* bev, void* user_data)
{
	struct evbuffer* output = bufferevent_get_output(bev);//返回输出缓冲区
	if (evbuffer_get_length(output) == 0) {
		printf("flushed answer\n");
		bufferevent_free(bev);//释放缓冲区
	}
}

//事件处理，发生错误时的回调函数
static void conn_eventcb(struct bufferevent* bev, short events, void* user_data)
{
	if (events & BEV_EVENT_EOF) {
		printf("Connection closed.\n");
	}
	else if (events & BEV_EVENT_ERROR) {
		printf("Got an error on the connection: %s\n",
			strerror(errno));/*XXX win32*/
	}
	/* None of the other events can happen here, since we haven't enabled
	 * timeouts */
	bufferevent_free(bev);
}

//有SIGINT到来时的回调函数
static void signal_cb(evutil_socket_t sig, short events, void* user_data)
{
	struct event_base* base = (struct event_base*)user_data;
	struct timeval delay = { 2, 0 };//sec,microsec

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);//在给定时间后退出循环（event_base_dispatch(base)），2s
}
