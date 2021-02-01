#include <stdio.h>
#include "MPPPLAT/sample_ai2aenc.h"
#include "MPPPLAT/sample_virvi2venc.h"
#include "EVENTLOOP/eventloop.h"
#include "wifi/wifi_ap.h"
#include "wifi/wifi_sta.h"
#include "event.h"
#include "MPPPLAT/mppplat.h"
#include "CRtmpPlayer.h"

static struct timeval CLOCK_TV;

static struct timeval TIMER_TV = {1, 0};

static void gettimeofday_cb(int nothing, short int which, void *ev)
{
	if (gettimeofday(&CLOCK_TV, NULL))
	{
		perror("gettimeofday()");
		event_loopbreak();
	}

	evtimer_add((struct event *)ev, &TIMER_TV);
}

static void udp_cb(const int sock, short int which, void *arg)
{
	struct sockaddr_in server_sin;
	socklen_t server_sz = sizeof(server_sin);
	char buf[sizeof(CLOCK_TV)];

	/* Recv the data, store the address of the sender in server_sin */
	if (recvfrom(sock, &buf, sizeof(buf) - 1, 0, (struct sockaddr *)&server_sin, &server_sz) == -1)
	{
		perror("recvfrom()");
		event_loopbreak();
	}

	/* Copy the time into buf; note, endianess unspecified! */
	memcpy(buf, &CLOCK_TV, sizeof(CLOCK_TV));

	/* Send the data back to the client */
	if (sendto(sock, buf, sizeof(CLOCK_TV), 0, (struct sockaddr *)&server_sin, server_sz) == -1)
	{
		perror("sendto()");
		event_loopbreak();
	}
}

int xmain(int argc, char **argv)
{
	int ret, port, sock, fd[2];

	struct event timer_event, udp_event;
	struct sockaddr_in sin;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(1497);

	if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)))
	{
		perror("bind()");
		exit(EXIT_FAILURE);
	}

	/* Initialize libevent */
	event_init();

	/* Add the clock event */
	evtimer_set(&timer_event, &gettimeofday_cb, &timer_event);
	evtimer_add(&timer_event, &TIMER_TV);

	/* Add the UDP event */
	event_set(&udp_event, sock, EV_READ | EV_PERSIST, udp_cb, NULL);
	event_add(&udp_event, 0);

	/* Enter the event loop; does not return. */
	event_dispatch();
	close(sock);
	return 0;
}

class TestEvent : public Event
{
public:
	TestEvent(){};
	~TestEvent(){};
	virtual int process(EventLoop *evl, Event *ev)
	{
		printf("%s %s\n", "TEST", "EVENT");
	}
};

struct MYEvent
{
	int type;
	void *data;
	struct event ev;
};

//回调函数处理

static void event_cb(evutil_socket_t fd, short event, void *arg)
{
	MYEvent *myEvent = (MYEvent *)arg;
	if (!myEvent)
	{
		return;
	}

	switch (myEvent->type)
	{
	case 0:
		std::cout << "event 1 occur" << std::endl;
		if (myEvent->data)
		{
			delete[] myEvent->data;
		}
		break;
	default:
		break;
	}
	delete myEvent;
	return;
}

int xxmain(int argc, char **argv)
{

	EventLoop *mainEvl = create_eventloop_thread();
	//venc_main(argc, argv);
	//aenc_main(0, 0);
	//wifi_sta_init();

	for (;;)
	{

		TestEvent *te = new TestEvent();
		event_push(mainEvl, te);
		sleep(1);
	}
	return 0;
}

#include <iostream>
#include <event2/event.h>
#include <event2/util.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
using namespace std;

#define MY_EVENT_TRAN 10000 //定义自己的事件

struct event_base *base;

void test_cb(int sock, short event, void *args)
{
	cout << "sock" << sock << "  "
		 << "event:" << event << endl;
	cout << *((string *)args) << endl;
}

void app_exit(int sock, short event, void *args)
{
	cout << "signal exit" << endl;
	event_base_loopexit(base, 0);
}

void self_event_cb(int sock, short event, void *args)
{
	cout << "sock" << sock << "  "
		 << "event:" << event << endl;
	cout << ((int *)args)[0] << endl;
}

int main()
{

	base = event_base_new();

	string msg = "hello world";
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 10;


	CRtmpPlayer * player = new CRtmpPlayer();
	player->start();

	for(;;){sleep(1000);};


	venc_main(0,0);

	int *m = new int[1];
	m[0] = 10;

	//int m=10; ！important 将这样定义的作为参数传给回调函数会报内存错误，传给回调的参数的内存必须是从heap上申请的。

	struct event *my_event = event_new(base, -1, EV_PERSIST, test_cb, &msg);
	struct event *my_event2 = event_new(base, -1, EV_PERSIST, self_event_cb, m);
	struct event *sig_event = event_new(base, SIGINT, EV_SIGNAL | EV_PERSIST, app_exit, 0);

	evsignal_initialized(sig_event);

	event_add(my_event, NULL);
	event_add(my_event2, &timeout); //每隔10010毫秒触发一次这个事件，event_del()可以删除该事件
	event_add(sig_event, NULL);
	event_active(my_event, MY_EVENT_TRAN, 0); //activate self define event
	event_active(my_event2, MY_EVENT_TRAN, 0); //activate self define event

	event_base_dispatch(base);
	delete[] m;
	event_free(my_event);
	event_free(my_event2);
	event_free(sig_event);
	event_base_free(base);
	return 0;
}
