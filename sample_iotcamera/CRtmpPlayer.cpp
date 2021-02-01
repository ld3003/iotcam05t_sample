#include "CRtmpPlayer.h"
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

int CRtmpPlayer ::open_url(const char *rtmp_url)
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

void CRtmpPlayer::cleanup()
{
	if (_rtmp)
	{

		srs_rtmp_destroy(_rtmp);
		_rtmp = NULL;
	}
}

CRtmpPlayer::CRtmpPlayer()
{

	//data = (char *)malloc(VIDEOBUF_LEN);
	//pthread_mutex_init(&mutex, NULL);

	status = CRtmpPlayer::STOPPUSH;
	run_status = CRtmpPlayer::RUN_STATUS_EXIT;
	_rtmp = 0;
#ifndef NO_THREAD_LOCK
	statusmutex = PTHREAD_MUTEX_INITIALIZER;
#endif
}

CRtmpPlayer::~CRtmpPlayer()
{
	cleanup();
	//free(data);
	// TODO Auto-generated destructor stub
}

int CRtmpPlayer::getPipe()
{
	return rtmppipe;
}

int CRtmpPlayer::writeTsData(unsigned char *dat, int len)
{
	return write(rtmppipe, dat, len);
}

void CRtmpPlayer::setUrl(char *myurl)
{

	snprintf(url, sizeof(url), "%s", myurl);
	printf("%s", url);
	printf("%s", myurl);
}

int CRtmpPlayer::init()
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

void CRtmpPlayer::deinit()
{
	cleanup();
}

void CRtmpPlayer::stopPush()
{
	switch (CRtmpPlayer::status)
	{
	case CRtmpPlayer::STOPPUSH:
		break;
	default:
		run_status = CRtmpPlayer::RUN_STATUS_EXIT;
		break;
	}

	printf("stoppush status %d run_status %d \n", status, run_status);
}

void CRtmpPlayer::startPush()
{
	run_status = CRtmpPlayer::RUN_STATUS_START;
}

#define CHECK_RUNSTATUS                             \
	if (run_status == CRtmpPlayer::RUN_STATUS_EXIT) \
	{                                               \
		deinit();                                   \
		status = CRtmpPlayer::STOPPUSH;             \
	};

int CRtmpPlayer::run()
{

	printf("suck rtmp stream like rtmpdump\n");
	printf("ossrs/srs client librtmp library.\n");
	printf("version: %d.%d.%d\n", srs_version_major(), srs_version_minor(), srs_version_revision());


	srs_human_trace("rtmp url: %s", "rtmp://r.ossrs.net/live/livestream");
	srs_rtmp_t rtmp = srs_rtmp_create("rtmp://r.ossrs.net/live/livestream");

	if (srs_rtmp_handshake(rtmp) != 0)
	{
		srs_human_trace("simple handshake failed.");
		goto rtmp_destroy;
	}
	srs_human_trace("simple handshake success");

	if (srs_rtmp_connect_app(rtmp) != 0)
	{
		srs_human_trace("connect vhost/app failed.");
		goto rtmp_destroy;
	}
	srs_human_trace("connect vhost/app success");

	if (srs_rtmp_play_stream(rtmp) != 0)
	{
		srs_human_trace("play stream failed.");
		goto rtmp_destroy;
	}
	srs_human_trace("play stream success");

	for (;;)
	{
		int size;
		char type;
		char *data;
		u_int32_t timestamp, pts;

		if (srs_rtmp_read_packet(rtmp, &type, &timestamp, &data, &size) != 0)
		{
			goto rtmp_destroy;
		}
		if (srs_utils_parse_timestamp(timestamp, type, data, size, &pts) != 0)
		{
			goto rtmp_destroy;
		}
		srs_human_trace("got packet: type=%s, dts=%d, pts=%d, size=%d",
						srs_human_flv_tag_type2string(type), timestamp, pts, size);

		free(data);
	}

rtmp_destroy:
	srs_rtmp_destroy(rtmp);


}

int CRtmpPlayer::writeH264RawData(unsigned char *dat, int len, int pts)
{
	int ret;
	int wret;
	if (status == CRtmpPlayer::CONNECTED)
	{
		wret = srs_h264_write_raw_frames(_rtmp, (char *)dat, len, pts, pts);
		if (wret == 1009)
		{
			printf("CRtmpPlayer::CONNERR \n");
			status = CRtmpPlayer::CONNERR;
		}
	}
	return 0;
}

int CRtmpPlayer::writeAACRawData(unsigned char *dat, int len, int pts)
{
	int ret;
	int wret;

	if (status == CRtmpPlayer::CONNECTED)
	{
		wret = srs_adts_aac_write_raw_frame(_rtmp, (char *)dat, len, pts);
		if (wret == 1009)
		{
			printf("CRtmpPlayer::CONNERR \n");
			status = CRtmpPlayer::CONNERR;
		}
	}

	return 0;
}
