#ifndef __RTMPPP_H__
#define __RTMPPP_H__

#include  <pthread.h>
#include "srsrtmp/srs_librtmp.hpp"
#include "ts/Mux.h"
#include "CThread.h"

class CRtmpPlayer : public CThread
{
public:
	CRtmpPlayer();
	virtual ~CRtmpPlayer();

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
