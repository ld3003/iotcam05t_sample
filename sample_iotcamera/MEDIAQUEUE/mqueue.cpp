#include "mqueue.h"
int mdata_push(MQUEUE *mq, MDATA *md)
{
    md->refcount++;
    mq->PushData((DATAPOINTER)md);
    return 0;
}

