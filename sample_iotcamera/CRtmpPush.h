#ifndef __RTMP_H__
#define __RTMP_H__

#include  <pthread.h>
#include "srsrtmp/srs_librtmp.hpp"
#include "ts/Mux.h"

#define THREAD_CLASS_NAME RTMPCThread
class THREAD_CLASS_NAME
{
public:
private:
	pthread_t tid;

private:
	static void *start_thread(void *arg)
	{
		THREAD_CLASS_NAME *ptr = (THREAD_CLASS_NAME *)arg;
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
	virtual int run() = 0; //基类中的虚函数要么实现，要么是纯虚函数（绝对不允许声明不实现，也不纯虚）
};

class CRtmpPush : public THREAD_CLASS_NAME
{
public:
	CRtmpPush();
	virtual ~CRtmpPush();

	int run();
	void stopPush();
	void startPush();
	void setUrl(char *myurl);
	int getPipe();
	int writeTsData(unsigned char *dat, int len);
	int writeH264RawData(unsigned char *dat, int len, int pts);
	int writeAACRawData(unsigned char *dat, int len, int pts);
	int init();

	unsigned char getrun_status()
	{
		return status;
	};

private:
	int open_url(const char *rtmp_url);
	void cleanup();

	enum
	{
		CONNINIT,
		CONNECTED,
		CONNERR,
		STOPPUSH,

		RUN_STATUS_START,
		RUN_STATUS_EXIT,
		RUN_STATUS_END,

	};

	void deinit();

	srs_rtmp_t _rtmp;
	unsigned char status;
	unsigned char run_status;
	int rtmppipe;
	char *data;
	char url[512];

	pthread_mutex_t statusmutex;
};

#endif /* RTMPPUSH_H_ */
