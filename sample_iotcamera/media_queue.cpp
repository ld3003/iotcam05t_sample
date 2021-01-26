#include "media_queue.h"

static MEDIA_QUEUE *MediaQueue;

struct MediaQueueBuf *alloaMediaQueueBuf(int size)
{
    struct MediaQueueBuf *mqb;

    mqb = (struct MediaQueueBuf *)malloc(sizeof(struct MediaQueueBuf));
    if (mqb)
    {
        mqb->BufLen = size;
        mqb->Buf = (unsigned char *)malloc(size);
        if (mqb->Buf)
        {
            return mqb;
        }
        else
        {
            free(mqb);
            mqb = 0;
        }
    }
    return mqb;
}

void freeMediaQueueBuf(struct MediaQueueBuf *buf)
{
    if (buf)
    {
        if (buf->Buf)
        {
            free(buf->Buf);
        }
        free(buf->Buf);
        buf = 0;
    }
}

int initMediaQueue()
{
    MediaQueue = (MEDIA_QUEUE *)malloc(sizeof(struct MEDIA_QUEUE));
    MediaQueue->ainq = InitQueue();
    MediaQueue->vinq = InitQueue();
    MediaQueue->adecq = InitQueue();

    return 0;
}

int pushVideoData(unsigned char *vData, int vDataLen)
{
    struct MediaQueueBuf *mqb = alloaMediaQueueBuf(vDataLen);
    if (mqb)
    {
        EnQueue(MediaQueue->vinq, (unsigned int)mqb);
    }

    return 0;
}
int pushAudioData(unsigned char *vData, int vDataLen)
{
    return 0;
}