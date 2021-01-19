/******************************************************************************
  Copyright (C), 2001-2017, Qigan Tech. Co., Ltd.
 ******************************************************************************
  File Name     : sample_virvi2venc.c
  Version       : Initial Draft
  Author        : Qigan BU3-PD2 Team
  Created       : 2017/1/5
  Last Modified :
  Description   : mpp component implement
  Function List :
  History       :
******************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "sample_virvi2venc"
#include <utils/plat_log.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "media/mm_comm_vi.h"
#include "media/mpi_vi.h"
#include "media/mpi_isp.h"
#include "media/mpi_venc.h"
#include "media/mpi_sys.h"
#include "mm_common.h" x
#include "mm_comm_venc.h"
#include "mm_comm_rc.h"
#include <mpi_videoformat_conversion.h>

#include <confparser.h>
#include "sample_virvi2venc.h"
#include "sample_virvi2venc_config.h"

//#define MAX_VIPP_DEV_NUM  2
//#define MAX_VIDEO_NUM         MAX_VIPP_DEV_NUM
//#define MAX_VIR_CHN_NUM   8

//VENC_CHN mVeChn;
//VI_DEV mViDev;
//VI_CHN mViChn;
//int AutoTestCount = 0,EncoderCount = 0;

//VI2Venc_Cap_S privCap[MAX_VIR_CHN_NUM][MAX_VIR_CHN_NUM];
//FILE* OutputFile_Fd;

SampleVirvi2VencConfparser *gpSampleVirvi2VencContext = NULL;

int hal_vipp_start(VI_DEV ViDev, VI_ATTR_S *pstAttr)
{
    QG_MPI_VI_CreateVipp(ViDev);
    QG_MPI_VI_SetVippAttr(ViDev, pstAttr);
    QG_MPI_VI_EnableVipp(ViDev);
    return 0;
}

int hal_vipp_end(VI_DEV ViDev)
{
    QG_MPI_VI_DisableVipp(ViDev);
    QG_MPI_VI_DestoryVipp(ViDev);
    return 0;
}

int hal_virvi_start(VI_DEV ViDev, VI_CHN ViCh, void *pAttr)
{
    int ret = -1;

    ret = QG_MPI_VI_CreateVirChn(ViDev, ViCh, pAttr);
    if (ret < 0)
    {
        aloge("Create VI Chn failed,VIDev = %d,VIChn = %d", ViDev, ViCh);
        return ret;
    }
    ret = QG_MPI_VI_SetVirChnAttr(ViDev, ViCh, pAttr);
    if (ret < 0)
    {
        aloge("Set VI ChnAttr failed,VIDev = %d,VIChn = %d", ViDev, ViCh);
        return ret;
    }
    return 0;
}

int hal_virvi_end(VI_DEV ViDev, VI_CHN ViCh)
{
    int ret = -1;
#if 0
    /* better be invoked after QG_MPI_VENC_StopRecvPic */
    ret = QG_MPI_VI_DisableVirChn(ViDev, ViCh);
    if(ret < 0)
    {
        aloge("Disable VI Chn failed,VIDev = %d,VIChn = %d",ViDev,ViCh);
        return ret ;
    }
#endif
    ret = QG_MPI_VI_DestoryVirChn(ViDev, ViCh);
    if (ret < 0)
    {
        aloge("Destory VI Chn failed,VIDev = %d,VIChn = %d", ViDev, ViCh);
        return ret;
    }
    return 0;
}

static void *GetEncoderFrameThread(void *pArg)
{
    int ret = 0;
    int count = 0;

    VI2Venc_Cap_S *pCap = (VI2Venc_Cap_S *)pArg;
    VI_DEV nViDev = pCap->Dev;
    VI_CHN nViChn = pCap->Chn;
    VENC_CHN nVencChn = pCap->mVencChn;
    VENC_STREAM_S VencFrame;
    VENC_PACK_S venc_pack;
    VencFrame.mPackCount = 1;
    VencFrame.mpPack = &venc_pack;
    alogd("Cap threadid=0x%lx, ViDev = %d, ViCh = %d\n", pCap->thid, nViDev, nViChn);

    if (nVencChn >= 0 && nViChn >= 0)
    {
        MPP_CHN_S ViChn = {MOD_ID_VIU, nViDev, nViChn};
        MPP_CHN_S VeChn = {MOD_ID_VENC, 0, nVencChn};
        ret = QG_MPI_SYS_Bind(&ViChn, &VeChn);
        if (ret != SUCCESS)
        {
            alogd("error!!! vi can not bind venc!!!\n");
            return (void *)FAILURE;
        }
    }
    //alogd("start start recv success!\n");
    ret = QG_MPI_VI_EnableVirChn(nViDev, nViChn);
    if (ret != SUCCESS)
    {
        alogd("VI enable error!");
        return (void *)FAILURE;
    }
    ret = QG_MPI_VENC_StartRecvPic(nVencChn);
    if (ret != SUCCESS)
    {
        alogd("VENC Start RecvPic error!");
        return (void *)FAILURE;
    }

    for (;;)
    {
        if ((ret = QG_MPI_VENC_GetStream(nVencChn, &VencFrame, 4000)) < 0) //6000(25fps) 4000(30fps)
        {
            alogd("get first frmae failed!\n");
            continue;
        }
        else
        {
            if (VencFrame.mpPack != NULL && VencFrame.mpPack->mLen0)
            {
                printf("write encode CHN: %d %fKB\n", nViChn,(float)VencFrame.mpPack->mLen0/1024);
                //fwrite(VencFrame.mpPack->mpAddr0, 1, VencFrame.mpPack->mLen0, gpSampleVirvi2VencContext->mOutputFileFp);
            }
            if (VencFrame.mpPack != NULL && VencFrame.mpPack->mLen1)
            {
                printf("write encode CHN: %d %fKB\n", nViChn,(float)VencFrame.mpPack->mLen1/1024);
                //fwrite(VencFrame.mpPack->mpAddr1, 1, VencFrame.mpPack->mLen1, gpSampleVirvi2VencContext->mOutputFileFp);
            }
            ret = QG_MPI_VENC_ReleaseStream(nVencChn, &VencFrame);
            if (ret < 0)
            {
                alogd("falied error,release failed!!!\n");
            }
        }
    }
    return NULL;
}

void Virvi2Venc_HELP()
{
    alogd("Run CSI0/CSI1+Venc command: ./sample_virvi2venc -path ./sample_virvi2venc.conf\r\n");
}

#if 0

########### paramter (ref to tulip_tarzanx.conf)############
[parameter]
auto_test_count = 1
encoder_count = 1800
#src parameter
#dev number : 0 ~3, vipp_number
#src_width *src_height : 720p / 1080p;
#src_frame_rate : 25
dev_num = 0
src_width = 1920
src_height = 1080
src_frame_rate = 20
#dest parameter
#dest_encoder_type is H .264 / H .265 / MJPEG
#dest_width *dest_height is VGA;
#dest_frame_rate is 25
#pic_format is nv21(to enc pixelfmt must be yuv420sp)
dest_encoder_type = H.265
dest_width = 1920
dest_height = 1080
dest_frame_rate = 20
dest_bit_rate = 8388608  //4M(4194304) 8M(8388608)
dest_pic_format = nv21
output_file_path = "QG_VirviEncoder.H265"

#endif

int venc_main(int argc, char *argv[])
{
    int ret, count = 0, result = 0;
    //int vipp_dev;
    int virvi_chn;
    //int isp_dev;

    SampleVirvi2VencConfparser *pContext = (SampleVirvi2VencConfparser *)malloc(sizeof(SampleVirvi2VencConfparser));
    gpSampleVirvi2VencContext = pContext;
    memset(pContext, 0, sizeof(SampleVirvi2VencConfparser));

#if 0
    printf("sample_virvi2venc buile time = %s, %s.\r\n", __DATE__, __TIME__);
    if (argc != 3)
    {
        Virvi2Venc_HELP();
        exit(0);
    }
    SampleVirvi2VencConfparser *pContext = (SampleVirvi2VencConfparser *)malloc(sizeof(SampleVirvi2VencConfparser));
    gpSampleVirvi2VencContext = pContext;
    memset(pContext, 0, sizeof(SampleVirvi2VencConfparser));
    /* parse command line param,read sample_virvi2venc.conf */
    if (ParseCmdLine(argc, argv, &pContext->mCmdLinePara) != 0)
    {
        aloge("fatal error! command line param is wrong, exit!");
        result = -1;
        goto _exit;
    }
    char *pConfigFilePath;
    if (strlen(pContext->mCmdLinePara.mConfigFilePath) > 0)
    {
        pConfigFilePath = pContext->mCmdLinePara.mConfigFilePath;
    }
    else
    {
        pConfigFilePath = DEFAULT_SAMPLE_VIPP2VENC_CONF_PATH;
    }
    /* parse config file. */
    if (loadSampleVirvi2VencConfig(&pContext->mConfigPara, pConfigFilePath) != SUCCESS)
    {
        aloge("fatal error! no config file or parse conf file fail");
        result = -1;
        goto _exit;
    }

#endif

    //while (count != pContext->mConfigPara.AutoTestCount)
    {
        alogd("======================================.\r\n");
        alogd("Auto Test count start: %d. (MaxCount==1000).\r\n", count);
        system("cat /proc/meminfo | grep Committed_AS");
        alogd("======================================.\r\n");
        MPP_SYS_CONF_S mSysConf;
        memset(&mSysConf, 0, sizeof(MPP_SYS_CONF_S));
        mSysConf.nAlignWidth = 32;

        //初始化 mpp 系统
        QG_MPI_SYS_SetConf(&mSysConf);
        ret = QG_MPI_SYS_Init();
        if (ret < 0)
        {
            aloge("sys Init failed!");
            return -1;
        }

        pContext->mViDev = 0; //pContext->mConfigPara.DevNum;
        /* dev:0, chn:0,1,2,3,4...16 */
        /* dev:1, chn:0,1,2,3,4...16 */
        /* dev:2, chn:0,1,2,3,4...16 */
        /* dev:3, chn:0,1,2,3,4...16 */
        /*Set VI Channel Attribute*/
        memset(&pContext->mViAttr, 0, sizeof(VI_ATTR_S));
        pContext->mViAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        pContext->mViAttr.memtype = V4L2_MEMORY_MMAP;
        pContext->mViAttr.format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420);
        pContext->mViAttr.format.field = V4L2_FIELD_NONE;
        pContext->mViAttr.format.width = 1920;  //pContext->mConfigPara.SrcWidth;
        pContext->mViAttr.format.height = 1080; //pContext->mConfigPara.SrcHeight;
        pContext->mViAttr.fps = 20;             //pContext->mConfigPara.SrcFrameRate;
        /* update configuration anyway, do not use current configuration */
        pContext->mViAttr.use_current_win = 0;
        pContext->mViAttr.nbufs = 5;
        pContext->mViAttr.nplanes = 2;

        /* MPP components */
        pContext->mVeChn[0] = 0;
        pContext->mVeChn[1] = 1;

        /* venc chn attr */
        memset(&pContext->mVEncChnAttr[0], 0, sizeof(VENC_CHN_ATTR_S));
        memset(&pContext->mVEncChnAttr[1], 0, sizeof(VENC_CHN_ATTR_S));

        //SIZE_S wantedVideoSize = {pContext->mConfigPara.DestWidth, pContext->mConfigPara.DestHeight};
        //int wantedFrameRate = pContext->mConfigPara.DestFrameRate;
        pContext->mVEncChnAttr[0].VeAttr.Type = PT_H264;      //pContext->mConfigPara.EncoderType;
        pContext->mVEncChnAttr[0].VeAttr.MaxKeyInterval = 20; //pContext->mConfigPara.DestFrameRate;
        pContext->mVEncChnAttr[0].VeAttr.SrcPicWidth = 1920;  //pContext->mConfigPara.SrcWidth;
        pContext->mVEncChnAttr[0].VeAttr.SrcPicHeight = 1080; //pContext->mConfigPara.SrcHeight;
        pContext->mVEncChnAttr[0].VeAttr.Field = VIDEO_FIELD_FRAME;
        pContext->mVEncChnAttr[0].VeAttr.PixelFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420; //pContext->mConfigPara.DestPicFormat;
                                                                                           //int wantedVideoBitRate = pContext->mConfigPara.DestBitRate;

        pContext->mVEncChnAttr[1].VeAttr.Type = PT_H265;      //pContext->mConfigPara.EncoderType;
        pContext->mVEncChnAttr[1].VeAttr.MaxKeyInterval = 20; //pContext->mConfigPara.DestFrameRate;
        pContext->mVEncChnAttr[1].VeAttr.SrcPicWidth = 1920;  //pContext->mConfigPara.SrcWidth;
        pContext->mVEncChnAttr[1].VeAttr.SrcPicHeight = 1080; //pContext->mConfigPara.SrcHeight;
        pContext->mVEncChnAttr[1].VeAttr.Field = VIDEO_FIELD_FRAME;
        pContext->mVEncChnAttr[1].VeAttr.PixelFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420; //pContext->mConfigPara.DestPicFormat;
                                                                                           //int wantedVideoBitRate = pContext->mConfigPara.DestBitRate;

#define TARGET_IMG_W 1920
#define TARGET_IMG_H 1080
#define TARGET_RATE 8388608

        if (PT_H264 == pContext->mVEncChnAttr[0].VeAttr.Type)
        {
            pContext->mVEncChnAttr[0].VeAttr.AttrH264e.Profile = 1;
            pContext->mVEncChnAttr[0].VeAttr.AttrH264e.bByFrame = TRUE;
            pContext->mVEncChnAttr[0].VeAttr.AttrH264e.PicWidth = TARGET_IMG_W;  //pContext->mConfigPara.DestWidth;
            pContext->mVEncChnAttr[0].VeAttr.AttrH264e.PicHeight = TARGET_IMG_H; //pContext->mConfigPara.DestHeight;
            pContext->mVEncChnAttr[0].VeAttr.AttrH264e.mLevel = H264_LEVEL_51;
            pContext->mVEncChnAttr[0].VeAttr.AttrH264e.mbPIntraEnable = TRUE;
            pContext->mVEncChnAttr[0].RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
            pContext->mVEncChnAttr[0].RcAttr.mAttrH264Cbr.mBitRate = TARGET_RATE; //pContext->mConfigPara.DestBitRate;
            pContext->mVEncChnAttr[0].RcAttr.mAttrH264Cbr.mMaxQp = 51;
            pContext->mVEncChnAttr[0].RcAttr.mAttrH264Cbr.mMinQp = 1;
        }
        else if (PT_H265 == pContext->mVEncChnAttr[0].VeAttr.Type)
        {
            pContext->mVEncChnAttr[0].VeAttr.AttrH265e.mProfile = 0;
            pContext->mVEncChnAttr[0].VeAttr.AttrH265e.mbByFrame = TRUE;
            pContext->mVEncChnAttr[0].VeAttr.AttrH265e.mPicWidth = TARGET_IMG_W;  //pContext->mConfigPara.DestWidth;
            pContext->mVEncChnAttr[0].VeAttr.AttrH265e.mPicHeight = TARGET_IMG_H; // pContext->mConfigPara.DestHeight;
            pContext->mVEncChnAttr[0].VeAttr.AttrH265e.mLevel = H265_LEVEL_62;
            pContext->mVEncChnAttr[0].VeAttr.AttrH265e.mbPIntraEnable = TRUE;
            pContext->mVEncChnAttr[0].RcAttr.mRcMode = VENC_RC_MODE_H265CBR;
            pContext->mVEncChnAttr[0].RcAttr.mAttrH265Cbr.mBitRate = TARGET_RATE; //pContext->mConfigPara.DestBitRate;
            pContext->mVEncChnAttr[0].RcAttr.mAttrH265Cbr.mMaxQp = 51;
            pContext->mVEncChnAttr[0].RcAttr.mAttrH265Cbr.mMinQp = 1;
        }
        else if (PT_MJPEG == pContext->mVEncChnAttr[0].VeAttr.Type)
        {
            pContext->mVEncChnAttr[0].VeAttr.AttrMjpeg.mbByFrame = TRUE;
            pContext->mVEncChnAttr[0].VeAttr.AttrMjpeg.mPicWidth = TARGET_IMG_W;  //pContext->mConfigPara.DestWidth;
            pContext->mVEncChnAttr[0].VeAttr.AttrMjpeg.mPicHeight = TARGET_IMG_H; //pContext->mConfigPara.DestHeight;
            pContext->mVEncChnAttr[0].RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            pContext->mVEncChnAttr[0].RcAttr.mAttrMjpegeCbr.mBitRate = TARGET_RATE; //pContext->mConfigPara.DestBitRate;
        }

#define TARGET_IMG_W2 640
#define TARGET_IMG_H2 480
#define TARGET_RATE2 83886
        if (PT_H264 == pContext->mVEncChnAttr[1].VeAttr.Type)
        {
            pContext->mVEncChnAttr[1].VeAttr.AttrH264e.Profile = 1;
            pContext->mVEncChnAttr[1].VeAttr.AttrH264e.bByFrame = TRUE;
            pContext->mVEncChnAttr[1].VeAttr.AttrH264e.PicWidth = TARGET_IMG_W2;  //pContext->mConfigPara.DestWidth;
            pContext->mVEncChnAttr[1].VeAttr.AttrH264e.PicHeight = TARGET_IMG_H2; //pContext->mConfigPara.DestHeight;
            pContext->mVEncChnAttr[1].VeAttr.AttrH264e.mLevel = H264_LEVEL_51;
            pContext->mVEncChnAttr[1].VeAttr.AttrH264e.mbPIntraEnable = TRUE;
            pContext->mVEncChnAttr[1].RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
            pContext->mVEncChnAttr[1].RcAttr.mAttrH264Cbr.mBitRate = TARGET_RATE2; //pContext->mConfigPara.DestBitRate;
            pContext->mVEncChnAttr[1].RcAttr.mAttrH264Cbr.mMaxQp = 51;
            pContext->mVEncChnAttr[1].RcAttr.mAttrH264Cbr.mMinQp = 1;
        }
        else if (PT_H265 == pContext->mVEncChnAttr[1].VeAttr.Type)
        {
            pContext->mVEncChnAttr[1].VeAttr.AttrH265e.mProfile = 0;
            pContext->mVEncChnAttr[1].VeAttr.AttrH265e.mbByFrame = TRUE;
            pContext->mVEncChnAttr[1].VeAttr.AttrH265e.mPicWidth = TARGET_IMG_W2;  //pContext->mConfigPara.DestWidth;
            pContext->mVEncChnAttr[1].VeAttr.AttrH265e.mPicHeight = TARGET_IMG_H2; // pContext->mConfigPara.DestHeight;
            pContext->mVEncChnAttr[1].VeAttr.AttrH265e.mLevel = H265_LEVEL_62;
            pContext->mVEncChnAttr[1].VeAttr.AttrH265e.mbPIntraEnable = TRUE;
            pContext->mVEncChnAttr[1].RcAttr.mRcMode = VENC_RC_MODE_H265CBR;
            pContext->mVEncChnAttr[1].RcAttr.mAttrH265Cbr.mBitRate = TARGET_RATE2; //pContext->mConfigPara.DestBitRate;
            pContext->mVEncChnAttr[1].RcAttr.mAttrH265Cbr.mMaxQp = 51;
            pContext->mVEncChnAttr[1].RcAttr.mAttrH265Cbr.mMinQp = 1;
        }
        else if (PT_MJPEG == pContext->mVEncChnAttr[1].VeAttr.Type)
        {
            pContext->mVEncChnAttr[1].VeAttr.AttrMjpeg.mbByFrame = TRUE;
            pContext->mVEncChnAttr[1].VeAttr.AttrMjpeg.mPicWidth = TARGET_IMG_W2;  //pContext->mConfigPara.DestWidth;
            pContext->mVEncChnAttr[1].VeAttr.AttrMjpeg.mPicHeight = TARGET_IMG_H2; //pContext->mConfigPara.DestHeight;
            pContext->mVEncChnAttr[1].RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            pContext->mVEncChnAttr[1].RcAttr.mAttrMjpegeCbr.mBitRate = TARGET_RATE2; //pContext->mConfigPara.DestBitRate;
        }

        pContext->mVencFrameRateConfig.SrcFrmRate = 20; //pContext->mConfigPara.SrcFrameRate;
        pContext->mVencFrameRateConfig.DstFrmRate = 20; //pContext->mConfigPara.DestFrameRate;
#if 0
        /* has invoked in QG_MPI_SYS_Init() */
        result = VENC_Construct();
        if (result != SUCCESS)
        {
            alogd("VENC Construct error!");
            result = -1;
            goto _exit;
        }
#endif
        hal_vipp_start(pContext->mViDev, &pContext->mViAttr);
        //QG_MPI_ISP_Init();
        QG_MPI_ISP_Run(pContext->mIspDev);
        // for (virvi_chn = 0; virvi_chn < MAX_VIR_CHN_NUM; virvi_chn++)
        for (virvi_chn = 0; virvi_chn < 2; virvi_chn++)
        {
            memset(&pContext->privCap[pContext->mViDev][virvi_chn], 0, sizeof(VI2Venc_Cap_S));
            pContext->privCap[pContext->mViDev][virvi_chn].Dev = pContext->mViDev;
            pContext->privCap[pContext->mViDev][virvi_chn].Chn = virvi_chn;
            pContext->privCap[pContext->mViDev][virvi_chn].s32MilliSec = 5000; // 2000;
            pContext->privCap[pContext->mViDev][virvi_chn].EncoderType = pContext->mVEncChnAttr[virvi_chn].VeAttr.Type;
            //if (0 == virvi_chn) /* H264, H265, MJPG, Preview(LCD or HDMI), VDA, ISE, AIE, CVBS */
            {
                /* open isp */
                if (pContext->mViDev == 0 || pContext->mViDev == 1)
                {
                    pContext->mIspDev = 0;
                }
                else if (pContext->mViDev == 2 || pContext->mViDev == 3)
                {
                    pContext->mIspDev = 1;
                }

                result = hal_virvi_start(pContext->mViDev, virvi_chn, NULL);
                if (result < 0)
                {
                    alogd("VI start failed!\n");
                    result = -1;
                    goto _exit;
                }
                pContext->privCap[pContext->mViDev][virvi_chn].thid = 0;
                result = QG_MPI_VENC_CreateChn(pContext->mVeChn[virvi_chn], &pContext->mVEncChnAttr[virvi_chn]);
                if (result < 0)
                {
                    alogd("create venc channel[%d] falied!\n", pContext->mVeChn[virvi_chn]);
                    result = -1;
                    goto _exit;
                }
                pContext->privCap[pContext->mViDev][virvi_chn].mVencChn = pContext->mVeChn[virvi_chn];
                QG_MPI_VENC_SetFrameRate(pContext->mVeChn[virvi_chn], &pContext->mVencFrameRateConfig);
                VencHeaderData vencheader;
                //open output file

                /*
                pContext->mOutputFileFp = fopen(pContext->mConfigPara.OutputFilePath, "wb+");
                if (!pContext->mOutputFileFp)
                {
                    aloge("fatal error! can't open file[%s]", pContext->mConfigPara.OutputFilePath);
                    result = -1;
                    //goto _exit;
                }
                */

                if (PT_H264 == pContext->mVEncChnAttr[virvi_chn].VeAttr.Type)
                {
                    QG_MPI_VENC_GetH264SpsPpsInfo(pContext->mVeChn[virvi_chn], &vencheader);
                    if (vencheader.nLength)
                    {
                        //fwrite(vencheader.pBuffer, vencheader.nLength, 1, pContext->mOutputFileFp);
                    }
                }
                else if (PT_H265 == pContext->mVEncChnAttr[virvi_chn].VeAttr.Type)
                {
                    QG_MPI_VENC_GetH265SpsPpsInfo(pContext->mVeChn[virvi_chn], &vencheader);
                    if (vencheader.nLength)
                    {
                        //fwrite(vencheader.pBuffer, vencheader.nLength, 1, pContext->mOutputFileFp);
                    }
                }
                result = pthread_create(&pContext->privCap[pContext->mViDev][virvi_chn].thid, NULL, GetEncoderFrameThread, (void *)&pContext->privCap[pContext->mViDev][virvi_chn]);
                if (result < 0)
                {
                    alogd("pthread_create failed, Dev[%d], Chn[%d].\n", pContext->privCap[pContext->mViDev][virvi_chn].Dev, pContext->privCap[pContext->mViDev][virvi_chn].Chn);
                    continue;
                }
            }
        }

        for (virvi_chn = 0; virvi_chn < 1; virvi_chn++)
        {
            int eError = 0;
            alogd("wait get encoder frame thread exit!");
            pthread_join(pContext->privCap[pContext->mViDev][virvi_chn].thid, (void *)&eError);
            alogd("get encoder frame thread exit done!");
        }

        result = QG_MPI_VENC_StopRecvPic(pContext->mVeChn);
        if (result != SUCCESS)
        {
            alogd("VENC Stop Receive Picture error!");
            result = -1;
            goto _exit;
        }
#if 1
        /* better call QG_MPI_VI_DisableVirChn immediately after QG_MPI_VENC_StopRecvPic was invoked */
        for (virvi_chn = 0; virvi_chn < 1; virvi_chn++)
        {
            ret = QG_MPI_VI_DisableVirChn(pContext->mViDev, virvi_chn);
            if (ret < 0)
            {
                aloge("Disable VI Chn failed,VIDev = %d,VIChn = %d", pContext->mViDev, virvi_chn);
                return ret;
            }
        }
#endif
        result = QG_MPI_VENC_ResetChn(pContext->mVeChn);
        if (result != SUCCESS)
        {
            alogd("VENC Reset Chn error!");
            result = -1;
            goto _exit;
        }
        QG_MPI_VENC_DestroyChn(pContext->mVeChn);
        if (result != SUCCESS)
        {
            alogd("VENC Destroy Chn error!");
            result = -1;
            goto _exit;
        }
        for (virvi_chn = 0; virvi_chn < 1; virvi_chn++)
        {
            result = hal_virvi_end(pContext->mViDev, virvi_chn);
            if (result < 0)
            {
                alogd("VI end failed!\n");
                result = -1;
                goto _exit;
            }
        }

        QG_MPI_ISP_Stop(pContext->mIspDev);
        QG_MPI_ISP_Exit();

        hal_vipp_end(pContext->mViDev);
        /* exit mpp systerm */
        ret = QG_MPI_SYS_Exit();
        if (ret < 0)
        {
            aloge("sys exit failed!");
            return -1;
        }
        fclose(pContext->mOutputFileFp);
        pContext->mOutputFileFp = NULL;
        alogd("======================================.\r\n");
        alogd("Auto Test count end: %d. (MaxCount==1000).\r\n", count);
        alogd("======================================.\r\n");
        count++;
    }
    if (pContext != NULL)
    {
        free(pContext);
        pContext = NULL;
    }
    gpSampleVirvi2VencContext = NULL;
    printf("sample_virvi2venc exit!\n");
    return 0;
_exit:
    if (pContext != NULL)
    {
        free(pContext);
        pContext = NULL;
    }
    gpSampleVirvi2VencContext = NULL;
    return result;
}
