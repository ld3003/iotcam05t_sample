//这是生产者消费者模型的代码

#include "pcm.h"


//消费者
void *consumer(void *arg)
{
    BlockQueue *bqp = (BlockQueue *)arg;
    DATAPOINTER data;
    for (;;)
    {
        bqp->PopData(data);
        std::cout << "Event data done: " << data << std::endl;
        QDataBase *qd = (QDataBase *)data;
        qd->process();
        delete(qd);
    }
}

//生产者
void *producter(void *arg)
{
    BlockQueue *bqp = (BlockQueue *)arg;
    srand((unsigned long)time(NULL));
    for (;;)
    {
        int data = rand() % 1024;
        bqp->PushData(data);
        std::cout << "Product data done: " << data << std::endl;
        // sleep(1);
    }
}


static BlockQueue bq;
static pthread_t c;
int create_consumer()
{

    pthread_create(&c, NULL, consumer, (void *)&bq); //消费者；
    //pthread_create(&p, NULL, producter, (void *)&bq); //生产者；
    return 0;
}

int producter_push(QDataBase *data)
{
    bq.PushData((DATAPOINTER)data);
    return 0;
}