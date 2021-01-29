#ifndef _SAMPLE_VIRVI2VENC_H_
#define _SAMPLE_VIRVI2VENC_H_

#include <plat_type.h>
#include <tsemaphore.h>


#include "media/mm_comm_vi.h"
#include "media/mpi_vi.h"
#include "media/mpi_isp.h"
#include "media/mpi_venc.h"
#include "media/mpi_sys.h"
#include "mm_common.h"
#include "mm_comm_venc.h"
#include "mm_comm_rc.h"
#include <mpi_videoformat_conversion.h>

#include <confparser.h>
#include "sample_virvi2venc.h"
#include "sample_virvi2venc_config.h"


#define MAX_FILE_PATH_SIZE (256)
typedef struct awVI2Venc_PrivCap_S
{
    pthread_t thid;
    VI_DEV Dev;
    VI_CHN Chn;
    QG_S32 s32MilliSec;
    PAYLOAD_TYPE_E EncoderType;
    VENC_CHN mVencChn;
} VI2Venc_Cap_S;

typedef struct SampleVirvi2VencCmdLineParam
{
    char mConfigFilePath[MAX_FILE_PATH_SIZE];
} SampleVirvi2VencCmdLineParam;

typedef struct SampleVirvi2VencConfig
{
    int AutoTestCount;
    int EncoderCount;
    int DevNum;
    int SrcWidth;
    int SrcHeight;
    int SrcFrameRate;
    PAYLOAD_TYPE_E EncoderType;
    int mTimeLapseEnable;
    int mTimeBetweenFrameCapture;
    int DestWidth;
    int DestHeight;
    int DestFrameRate;
    int DestBitRate;
    PIXEL_FORMAT_E DestPicFormat; //MM_PIXEL_FORMAT_YUV_PLANAR_420
    char OutputFilePath[MAX_FILE_PATH_SIZE];
} SampleVirvi2VencConfig;

typedef struct SampleVirvi2VencConfparser
{
    SampleVirvi2VencCmdLineParam mCmdLinePara;
    SampleVirvi2VencConfig mConfigPara;
    FILE *mOutputFileFp;

    VENC_CHN mVeChn[2];

    ISP_DEV mIspDev;
    VI_DEV mViDev;
    //VI_CHN mViChn;

    VI_ATTR_S mViAttr;
    VENC_CHN_ATTR_S mVEncChnAttr[2];
    VENC_FRAME_RATE_S mVencFrameRateConfig;

    VI2Venc_Cap_S privCap[MAX_VIPP_DEV_NUM][MAX_VIR_CHN_NUM];
} SampleVirvi2VencConfparser;

#ifdef __cplusplus //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的

extern "C"
{



#endif

    extern int venc_main(int argc, char *argv[]);

#ifdef __cplusplus
}

#endif

#endif /* _SAMPLE_VIRVI2VENC_H_ */
