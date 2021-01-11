
#ifndef AUDIO_DEC_
#define AUDIO_DEC_



#ifdef __cplusplus
extern "C" {
#endif

int initavenc();
int startAO();
int put_aac_data(unsigned char*aacdata , int aaclen);

#ifdef __cplusplus
}
#endif

#endif