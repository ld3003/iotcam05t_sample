#ifndef __mppplat_h__
#define __mppplat_h__

#include "MEDIAQUEUE/mqueue.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int initMpp(){};
    int setVideoEnc(int start) { return 0; };
    int setAudioEnc(int start) { return 0; };
    int registerMq(MQUEUE *mq);

#ifdef __cplusplus
}
#endif

#endif
