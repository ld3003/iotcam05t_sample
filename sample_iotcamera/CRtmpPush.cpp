#include "CRtmpPush.h"
#include <iostream>
#include "srsrtmp/srs_librtmp.hpp"
#include "srsrtmp/srs_lib_ts_demuxer.hpp"
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include <fcntl.h>
/////////////////////////////////////////

#define PUSH_FILE 0
#define VIDEOBUF_LEN 188
#define TIMEOUT 5000

#define NO_THREAD_LOCK

//#define TS_PACKET_SIZE 1024
//const char url[]={"rtmp://47.93.103.232:1935/live/demo"};

static int64_t getNowMicro()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

int CRtmpPush ::open_url(const char *rtmp_url)
{
	int ret = 0;

	_rtmp = srs_rtmp_create(rtmp_url);
	if (!_rtmp)
	{
		return -1;
	}

	srs_rtmp_set_timeout(_rtmp, TIMEOUT, TIMEOUT);

	ret = srs_rtmp_handshake(_rtmp);
	if (ret)
	{
		return ret;
	}

	ret = srs_rtmp_connect_app(_rtmp);
	if (ret)
	{
		return ret;
	}

	return srs_rtmp_publish_stream(_rtmp);
}

void CRtmpPush::cleanup()
{
	if (_rtmp)
	{

		srs_rtmp_destroy(_rtmp);
		_rtmp = NULL;
	}
}

CRtmpPush::CRtmpPush()
{

	//data = (char *)malloc(VIDEOBUF_LEN);
	//pthread_mutex_init(&mutex, NULL);

	status = CRtmpPush::STOPPUSH;
	run_status = CRtmpPush::RUN_STATUS_EXIT;
	_rtmp = 0;
#ifndef NO_THREAD_LOCK
	statusmutex = PTHREAD_MUTEX_INITIALIZER;
#endif
}

CRtmpPush::~CRtmpPush()
{
	cleanup();
	//free(data);
	// TODO Auto-generated destructor stub
}

int CRtmpPush::getPipe()
{
	return rtmppipe;
}

int CRtmpPush::writeTsData(unsigned char *dat, int len)
{
	return write(rtmppipe, dat, len);
}

void CRtmpPush::setUrl(char *myurl)
{

	snprintf(url, sizeof(url), "%s", myurl);
	printf("%s", url);
	printf("%s", myurl);
}

int CRtmpPush::init()
{
	int ret = open_url(url);

	if (ret)
	{
		printf("connect rtmp url(%s) failed,ret = %d\n", url, ret);
		return ret;
	}
	else
	{
		printf("connect rtmp url(%s) succes,ret = %d\n", url, ret);
	}
	return 0;
}

void CRtmpPush::deinit()
{
	cleanup();
}

void CRtmpPush::stopPush()
{
	switch (CRtmpPush::status)
	{
	case CRtmpPush::STOPPUSH:
		break;
	default:
		run_status = CRtmpPush::RUN_STATUS_EXIT;
		break;
	}

	printf("stoppush status %d run_status %d \n", status, run_status);
}

void CRtmpPush::startPush()
{
	run_status = CRtmpPush::RUN_STATUS_START;
}

#define CHECK_RUNSTATUS                           \
	if (run_status == CRtmpPush::RUN_STATUS_EXIT) \
	{                                             \
		deinit();                                 \
		status = CRtmpPush::STOPPUSH;             \
	};

int CRtmpPush::run()
{

	for (;;)
	{

#ifndef NO_THREAD_LOCK
		pthread_mutex_lock(&statusmutex);
#endif

		//printf("status %d run_status %d \n",status,run_status);

		switch (status)
		{

		case CRtmpPush::STOPPUSH:
			if (run_status == CRtmpPush::RUN_STATUS_START)
				status = CRtmpPush::CONNINIT;
			break;

		case CRtmpPush::CONNINIT:

			//关闭码流输出
			printf("connect to server \n");
			if (init() == 0)
			{
				printf("connect success \n");
				status = CRtmpPush::CONNECTED;
			}
			else
			{
				printf("connect err \n");
				deinit();
				sleep(2);
			}

			if (run_status == CRtmpPush::RUN_STATUS_EXIT)
			{
				deinit();
				status = CRtmpPush::STOPPUSH;
			};

			break;

		case CRtmpPush::CONNECTED:
			if (run_status == CRtmpPush::RUN_STATUS_EXIT)
			{
				deinit();
				status = CRtmpPush::STOPPUSH;
			};
			break;

		case CRtmpPush::CONNERR:
			usleep(1000 * 100);
			deinit();
			status = CRtmpPush::CONNINIT;
			if (run_status == CRtmpPush::RUN_STATUS_EXIT)
			{
				deinit();
				status = CRtmpPush::STOPPUSH;
			};
			break;

		default:
			break;
		}

#ifndef NO_THREAD_LOCK
		pthread_mutex_unlock(&statusmutex);
#endif
		usleep(1000 * 100);
	}

	printf("EXIT PUSH RTMP THREAD !\n");
	return 0;
}

int CRtmpPush::writeH264RawData(unsigned char *dat, int len, int pts)
{
	int ret;
	int wret;
	if (status == CRtmpPush::CONNECTED)
	{
		wret = srs_h264_write_raw_frames(_rtmp, (char *)dat, len, pts, pts);
		if (wret == 1009)
		{
			printf("CRtmpPush::CONNERR \n");
			status = CRtmpPush::CONNERR;
		}
	}
	return 0;
}

int CRtmpPush::writeAACRawData(unsigned char *dat, int len, int pts)
{
	int ret;
	int wret;

	if (status == CRtmpPush::CONNECTED)
	{
		wret = srs_adts_aac_write_raw_frame(_rtmp, (char *)dat, len, pts);
		if (wret == 1009)
		{
			printf("CRtmpPush::CONNERR \n");
			status = CRtmpPush::CONNERR;
		}
	}

	return 0;
}
