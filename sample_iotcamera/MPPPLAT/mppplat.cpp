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
    return len;
}

int getAudioData(unsigned char **audiodata)
{
    int len;
    DATAPOINTER data;
    MDATA *md;
    mqe[2]->PopData(data);
    md = (MDATA *)data;
    *audiodata = md->data;
    len = md->dlen;
    delete md;
}