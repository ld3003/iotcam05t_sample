#include <stdio.h>
#include "sample_ai2aenc.h"
#include "sample_virvi2venc.h"
#include "EVENTLOOP/eventloop.h"
#include "wifi/wifi_ap.h"
#include "wifi/wifi_sta.h"

class TestEvent : public Event
{
public:
	TestEvent(){};
	~TestEvent(){};
	virtual int process()
	{
		printf("%s %s\n", "TEST", "EVENT");
	}
};

int main(int argc, char **argv)
{
	EventLoop *mainEvl = create_eventloop();
	venc_main(argc, argv);
	//aenc_main(0, 0);

	wifi_sta_init();

	for (;;)
	{

		TestEvent *te = new TestEvent();
		event_push(mainEvl, te);
		sleep(1);
	}
	return 0;
}
