#include "eventloop.h"




//消费者
void *consumer(void *arg)
{
    EventLoop *evl = (EventLoop *)arg;
    DATAPOINTER data;
    for (;;)
    {
        evl->PopData(data);
        #ifdef DEBUGOPEN
        std::cout << "Event data done: " << data << std::endl;
        #endif
        Event *ev = (Event *)data;
        ev->process(evl,ev);
        delete (ev);
    }
}

//生产者
void *producter(void *arg)
{
    EventLoop *evl = (EventLoop *)arg;
    srand((unsigned long)time(NULL));
    for (;;)
    {
        int data = rand() % 1024;
        evl->PushData(data);
        #ifdef DEBUGOPEN
        std::cout << "Product data done: " << data << std::endl;
        #endif
        // sleep(1);
    }
}


EventLoop *create_eventloop_thread()
{
    EventLoop *evl = new EventLoop();
    if (evl)
        pthread_create(&evl->tid, NULL, consumer, (void *)evl); //消费者；
    return evl;
}

int event_push(EventLoop *evl, Event *ev)
{
    evl->PushData((DATAPOINTER)ev);
    return 0;
}
