/*
	��Դ ��libevent/sample/
*/

/*
  This example program provides a trivial server program that listens for TCP
  connections on port 9995.  When they arrive, it writes a short message to
  each client connection, and closes each connection once it is flushed.
  ��9995�˿��ϼ������������ӽ���ʱ�����ͻ��˴�һ��hello word��ˢ�º�ر�ÿ������

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

//���������͸��ͻ��˵���Ϣ
static const char MESSAGE[] = "Hello, World!\n";

//�˿ں�
static const int PORT = 9995;

static void listener_cb(struct evconnlistener*, evutil_socket_t,struct sockaddr*, int socklen, void*);//��������
static void conn_writecb(struct bufferevent*, void*);//д�ص�����
static void conn_eventcb(struct bufferevent*, short, void*);//�����¼��ص�����
static void signal_cb(evutil_socket_t, short, void*);//�ź��¼��ص�����

int main(int argc, char** argv)
{
	struct event_base* base;//event_base���󣬸��ٹ�������¼������䷵�ظ�Ӧ�ó���
	struct evconnlistener* listener;//����
	struct event* signal_event;//�ź��¼�
	struct sockaddr_in sin;//IPV4 socket address

//�����windows��������Ҫ��WSAstarup��ʼ�� socket
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	base = event_base_new();//����event_baseʵ��
	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);

	listener = evconnlistener_new_bind(base, listener_cb, (void*)base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,(struct sockaddr*) & sin,	sizeof(sin));
	/*	base ���ڼ������ӵ�event_base
		evconnlistener_cb �ص����� ����ΪNULLʱ�������������ֱ����������
		ptr ָ�봫�ݸ��ص�����
		flags ��־�����ƻص���������Ϊ
		backlog -1���������趨ֵ
		sa ������ַ 
		socklen ��ַ����	*/

	/*	backlog--
		TCP�������ӽ׶Σ����ں���ά���������У�δ��ɶ��к���ɶ���
		backlog��ʷ�ϱ�����Ϊ��������֮��	 */

	/*	flags--
		LEV_OPT_CLOSE_ON_FREE���ͷż�����ʱ��ѵײ���׽���Ҳ�ر�
		LEV_OPT_REUSEABLE������׽����ǿ����õģ�����һ���رգ����������������׽��֣�����ͬ�˿ڽ��м���
		LEV_OPT_LEAVE_SOCKETS_BLOCKING	Ĭ������£��������յ����Ӻ�����Ϊ�������ģ��ñ�ǽ��ô���Ϊ
		LEV_OPT_CLOSE_ON_EXEC �� the close-on-exec
		LEV_OPT_THREADSAFE ����
		LEV_OPT_DISABLED �������Խ���״̬����������Ҫͨ��evconnlistener_enable()����
		LEV_OPT_DEFERRED_ACCEPT �ӳ�accept()���ӣ�ֱ�����ݿ���
		LEV_OPT_REUSEABLE_PORT ���������������/�̣߳��󶨵���ͬ�˿�
		LEV_OPT_BIND_IPV6ONLY ������ֻ������IPv6 socket*/

	if (!listener) {
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}
	//��SIGINT(interrupt) signal�¼�����ʱ��Ҫ��ʲô
	signal_event = evsignal_new(base, SIGINT, signal_cb, (void*)base);
	//event_new(base,SIGINT,EV_SIGNAL|EV_PERSIST,signal_cb,(void*)base) signal_cb�ص�����

	if (!signal_event || event_add(signal_event, NULL) < 0) {
		//event_add NULL:һֱ�ȴ� return -1����0�ɹ�
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}

	event_base_dispatch(base);//�¼����Ȼ�·��ѭ��

	evconnlistener_free(listener);//�ͷ�listener
	event_free(signal_event);//�ͷ�event
	event_base_free(base);//�ͷ�base

	printf("done\n");
	return 0;
}

//�������ӵ���ʱ�Ļص�����
static void listener_cb(struct evconnlistener* listener, evutil_socket_t fd,
	struct sockaddr* sa, int socklen, void* user_data)
/*	typedef void (*evconnlistener_cb)(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int socklen, void *);
	*/
{
	struct event_base* base = (struct event_base*)user_data;//��������base
	struct bufferevent* bev;//����IO bufferevent��һ���ײ�Ĵ���˿ڣ����׽��֣���һ����ȡ��������һ��д�뻺������ɡ�
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);//����һ�������� BEV_OPT_CLOSE_ON_FREE�ͷ�buffereventʱ�رյײ�..
	if (!bev) {
		fprintf(stderr, "Error constructing bufferevent!");
		event_base_loopbreak(base);//�����˳�
		return;
	}
	bufferevent_setcb(bev, NULL, conn_writecb, conn_eventcb, NULL);
	//�������Ļص�������bufev,readcb���ص�����,writecbд�ص�����,eventcb�¼�������,�ص������Ĳ���
	bufferevent_enable(bev, EV_WRITE);//����bufferevent��д����
	bufferevent_disable(bev, EV_READ);//����bufferevent��������
	bufferevent_write(bev, MESSAGE, strlen(MESSAGE));//�򻺳���д��hello word��
}

//д�ص�������д��heloword���ڷ���������ʾflush answer
static void conn_writecb(struct bufferevent* bev, void* user_data)
{
	struct evbuffer* output = bufferevent_get_output(bev);//�������������
	if (evbuffer_get_length(output) == 0) {
		printf("flushed answer\n");
		bufferevent_free(bev);//�ͷŻ�����
	}
}

//�¼�������������ʱ�Ļص�����
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

//��SIGINT����ʱ�Ļص�����
static void signal_cb(evutil_socket_t sig, short events, void* user_data)
{
	struct event_base* base = (struct event_base*)user_data;
	struct timeval delay = { 2, 0 };//sec,microsec

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);//�ڸ���ʱ����˳�ѭ����event_base_dispatch(base)����2s
}
