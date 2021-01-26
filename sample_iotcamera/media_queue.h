#ifndef __media_queue_h__
#define __media_queue_h__

#include "queue.h"

struct MEDIA_QUEUE
{

    Queue *vinq;
    Queue *ainq;
    Queue *adecq;
};
struct MediaQueueBuf
{
    unsigned char *Buf;
    int BufLen;
};

struct MediaQueueBuf *alloaMediaQueueBuf(int size);
void freeMediaQueueBuf(struct MediaQueueBuf *buf);

int initMediaQueue();
int pushVideoData(unsigned char *vData, int vDataLen);
int pushAudioData(unsigned char *vData, int vDataLen);

#endif