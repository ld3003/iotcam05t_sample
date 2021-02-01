#include "mppplat.h"
#include "MEDIAQUEUE/mqueue.h"

MQUEUE *mqe[3];

int initMpp()
{

    mqe[0] = new MQUEUE();
    mqe[1] = new MQUEUE();
    mqe[2] = new MQUEUE();
}

int setVideoEnc(int start) { return 0; };
int setAudioEnc(int start) { return 0; };

int setAudioData(unsigned char *audiodata, int len)
{
    MDATA *md = new MDATA();
    md->dlen = len;
    md->data = audiodata;
    mqe[2]->PushData((DATAPOINTER)md);
    return len;
}

int getAudioData(unsigned char **audiodata)
{
    int len;
    DATAPOINTER data;
    MDATA *md;
    printf("1\n");
    printf("getAudioData QUEUE LEN : %d\n", mqe[2]->getSize());
    mqe[2]->PopData(data);
    printf("2\n");
    md = (MDATA *)data;
    printf("3\n");
    *audiodata = md->data;
    len = md->dlen;

    delete md;

    return len;
}