#ifndef __MQTTPROCESS__
#define __MQTTPROCESS__

#include <pthread.h>

class MQTTPROCESS
{
public:
	MQTTPROCESS(){};
	~MQTTPROCESS(){};

private:
	pthread_t tid;

private:
	static void *start_thread(void *arg)
	{
		MQTTPROCESS *ptr = (MQTTPROCESS *)arg;
		ptr->run(); //线程的实体是run
	}

public:
	int start()
	{
		if (pthread_create(&tid, NULL, start_thread, (void *)this) != 0) //´创建一个线程(必须是全局函数)
		{

			return -1;
		}
		return 0;
	}
	int run();
};

int mqttArrived_Fun(char *from, char *msg);


typedef void(*lpFunc)(char*,char*); 
void device_state_mng(lpFunc fun);
#endif
