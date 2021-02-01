#ifndef __mppplat_h__
#define __mppplat_h__

#ifdef __cplusplus
extern "C"
{
#endif

    int initMpp();
    int setVideoEnc(int start);
    int setAudioEnc(int start);

    int setAudioData(unsigned char *audiodata, int len);
    int getAudioData(unsigned char **audiodata);

#ifdef __cplusplus
}
#endif

#endif
