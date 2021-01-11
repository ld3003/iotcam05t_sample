#include "audiodec.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "sample_comm.h"
#include "acodec.h"
#include "audio_aac_adp.h"
#include "audio_dl_adp.h"
#include "hi_resampler_api.h"

#define SAMPLE_DBG(s32Ret)\
    do{\
        printf("s32Ret=%#x,fuc:%s,line:%d\n", s32Ret, __FUNCTION__, __LINE__);\
    }while(0)


int startAO()
{
    HI_S32      s32Ret;
    AO_CHN      AoChn = 0;
    ADEC_CHN    AdChn = 0;
    HI_S32      s32AoChnCnt;
    FILE*        pfd = NULL;
    AIO_ATTR_S stAioAttr;

#ifdef HI_ACODEC_TYPE_TLV320AIC31
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_EXTERN_AO_DEV;
    stAioAttr.enSamplerate   = g_enWorkSampleRate;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = g_enSoundmode;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = AACLC_SAMPLES_PER_FRAME;
    stAioAttr.u32ChnCnt      = g_u32ChnCnt;
    stAioAttr.u32ClkSel      = 1;
    stAioAttr.enI2sType      = AIO_I2STYPE_EXTERN;
#else
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_INNER_AO_DEV;
    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_48000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = AACLC_SAMPLES_PER_FRAME;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 0;
    stAioAttr.enI2sType      = AIO_I2STYPE_INNERCODEC;
#endif


    s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, PT_AAC);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto ADECAO_ERR3;
    }

    s32AoChnCnt = stAioAttr.u32ChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, s32AoChnCnt, &stAioAttr, AUDIO_SAMPLE_RATE_44100, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto ADECAO_ERR2;
    }

    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto ADECAO_ERR1;
    }

    s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto ADECAO_ERR1;
    }

#if 0
    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();

    s32Ret = SAMPLE_COMM_AUDIO_CreatTrdFileAdec(AdChn, pfd);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto ADECAO_ERR0;
    }
#endif

    return 0;

    ADECAO_ERR3:
    ADECAO_ERR2:
    ADECAO_ERR1:
    ADECAO_ERR0:

    printf("AOenc ERROR\n");

    return -1;
}

int put_aac_data(unsigned char*aacdata , int aaclen)
{

    HI_S32 s32Ret;
    AUDIO_STREAM_S stAudioStream;
    HI_U32 u32Len = 640;
    HI_U32 u32ReadLen;
    HI_S32 s32AdecChn = 0;
    HI_U8* pu8AudioStream = NULL;
    //SAMPLE_ADEC_S* pstAdecCtl = (SAMPLE_ADEC_S*)parg;
    //FILE* pfd = pstAdecCtl->pfd;
    //s32AdecChn = pstAdecCtl->AdChn;

    stAudioStream.pStream = aacdata;
    stAudioStream.u32Len = aaclen;

    s32Ret = HI_MPI_ADEC_SendStream(s32AdecChn, &stAudioStream, HI_TRUE);

    return s32Ret;


#if 0

    pu8AudioStream = (HI_U8*)malloc(sizeof(HI_U8) * MAX_AUDIO_STREAM_LEN);
    if (NULL == pu8AudioStream)
    {
        printf("%s: malloc failed!\n", __FUNCTION__);
        return NULL;
    }

    while (HI_TRUE == pstAdecCtl->bStart)
    {
        /* read from file */
        stAudioStream.pStream = aacdata;
        #if 0
        printf("fread(stAudioStream.pStream, 1, u32Len, pfd) \n");
        u32ReadLen = fread(stAudioStream.pStream, 1, u32Len, pfd);
        if (u32ReadLen <= 0)
        {
            s32Ret = HI_MPI_ADEC_SendEndOfStream(s32AdecChn, HI_FALSE);
            if (HI_SUCCESS != s32Ret)
            {
                printf("%s: HI_MPI_ADEC_SendEndOfStream failed!\n", __FUNCTION__);
            }
            (HI_VOID)fseek(pfd, 0, SEEK_SET);/*read file again*/
            continue;
        }
        #endif
        /* here only demo adec streaming sending mode, but pack sending mode is commended */
        stAudioStream.u32Len = aaclen;
        s32Ret = HI_MPI_ADEC_SendStream(s32AdecChn, &stAudioStream, HI_TRUE);
        if (HI_SUCCESS != s32Ret)
        {
            printf("%s: HI_MPI_ADEC_SendStream(%d) failed with %#x!\n", \
                   __FUNCTION__, s32AdecChn, s32Ret);
            break;
        }
    }

    free(pu8AudioStream);
    pu8AudioStream = NULL;
    fclose(pfd);
    pstAdecCtl->bStart = HI_FALSE;
    return NULL;
#endif
}