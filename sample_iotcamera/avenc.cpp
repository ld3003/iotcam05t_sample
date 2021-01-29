
#include "avenc.h"
#include "ts/Ts.h"
#include "ts/Mux.h"
#include "rtp/rtp.h"
#include "srsrtmp/srs_kernel_ts.hpp"
#include "srsrtmp/srs_librtmp.hpp"
#include "srsrtmp/srs_lib_ts_demuxer.hpp"

#define VIDEOBUF_LEN 188
#define TIMEOUT 5000

#define MAX_MW_COUNT 5
static unsigned int media_mw_count = 0;
static struct MEDIA_WRITE *mwptr[MAX_MW_COUNT];
static unsigned char pausewrite_flag = 0;

static int64_t getNowMicro()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((int64_t)tv.tv_sec * 1000000 + tv.tv_usec);
}

int initavenc()
{
	for (int i = 0; i < MAX_MW_COUNT; i++)
	{
		mwptr[i] = 0;
	}
}

int write_audio(unsigned char *buf, int len)
{

	if (pausewrite_flag == 1)
		return len;
	//WriteBuf2TsFile(&chn1_ts_session, 44100, 0, buf, len, 0);
	//srs_aac_write_raw_frames(_rtmp, (char*)buf, len, getNowMicro(), getNowMicro());

	static int64_t _prev_audio_pts = 0;
	if (_prev_audio_pts == 0)
		_prev_audio_pts = getNowMicro();
	int pts = (getNowMicro() - _prev_audio_pts) / 1000;

	for (int i = 0; i < MAX_MW_COUNT; i++)
	{
		if (mwptr[i] > 0)
		{
			mwptr[i]->write_a(buf, len, pts);
		}
	}

	//srs_adts_aac_write_raw_frame(_rtmp, (char*)buf, len, pts);
	//WriteBuf2TsFile(&chn1_ts_session, 44100,pts, 0, buf, len, 0);
	return len;
}

static int64_t _prev_video_pts[3] = {0, 0, 0};
int write_video(int chn, unsigned char *buf, int len)
{

	int i;

	if (pausewrite_flag == 1)
		return len;

	if (_prev_video_pts[chn] == 0)
		_prev_video_pts[chn] = getNowMicro();
	int pts = (getNowMicro() - _prev_video_pts[chn]) / 1000;

	//if (chn == 2)
	//	printf("ptsptsptsptsptsptsptsptsptsptsptsptsptspts %d\n",pts);

	for (int i = 0; i < MAX_MW_COUNT; i++)
	{
		if (mwptr[i] > 0)
		{
			if (mwptr[i]->chn == chn)
				mwptr[i]->write_v(buf, len, pts);
		}
	}
	return len;
}

void pause_write(unsigned char flag)
{
	pausewrite_flag = flag;
}

int register_mw(struct MEDIA_WRITE *mw)
{
	for (int i = 0; i < MAX_MW_COUNT; i++)
	{
		if (mwptr[i] == 0)
		{
			mwptr[i] = mw;
			return 0;
		}
	}
	return -1;
}
int unregister_mw(struct MEDIA_WRITE *mw)
{

	for (int i = 0; i < MAX_MW_COUNT; i++)
	{
		if (mwptr[i] == mw)
		{
			mwptr[i] = 0;
			return 0;
		}
	}

	return -1;
}

