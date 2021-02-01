#ifndef __mq_h__
#define __mq_h__

#include <iostream>
#include <queue>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM 180

// #define DEBUGOPEN

typedef long DATAPOINTER;
class MQUEUE;

class MDATA
{
public:
    MDATA()
    {
        refcount = 0;
        mtype = 0;
    };
    ~MDATA(){};
    int release()
    {
        refcount--;
        if (refcount == 0)
        {
            delete this;
        }
    }
    virtual int process(MQUEUE *mq, MDATA *md) { return 0; };
    int refcount;
    int mtype;
    unsigned char *data;
    int dlen;
};

class MQUEUE
{
private:
    std::queue<DATAPOINTER> q;
    int cap;
    pthread_mutex_t mutex;
    pthread_cond_t full;
    pthread_cond_t empty;

private:
    void LockQueue() //队列加锁
    {
        pthread_mutex_lock(&mutex);
    }
    void UnlockQueue() //队列解锁
    {
        pthread_mutex_unlock(&mutex);
    }
    void ProductWait() //队列满，生产者等待
    {
        pthread_cond_wait(&full, &mutex);
    }
    void ConsumeWait() //队列空，消费者等待
    {
        pthread_cond_wait(&empty, &mutex);
    }
    void NotifyProduct() //队列不为满时，通知生产者
    {
        pthread_cond_signal(&full);
    }
    void NotifyConsume() //队列不为空时，通知消费者
    {
        pthread_cond_signal(&empty);
    }
    bool IsEmpty()
    {
        return (q.size() == 0 ? true : false);
    }
    bool IsFull()
    {
        return (q.size() == cap ? true : false);
    }

public:
    MQUEUE(int _cap = NUM) : cap(_cap) //构造函数
    {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&full, NULL);
        pthread_cond_init(&empty, NULL);
    }
    int getSize()
    {
        return q.size();
    }
    void PushData(const DATAPOINTER &data)
    {
        LockQueue();
        while (IsFull()) //队列满
        {
            NotifyConsume();
#ifdef DEBUGOPEN

#endif
            std::cout << "queue full,notify consume data,product stop!!" << std::endl;
            ProductWait();
        }
        //队列不满,生产者插入数据，通知消费者队列中已经有数据了
        q.push(data);
        NotifyConsume();
        UnlockQueue();
    }
    void PopData(DATAPOINTER &data)
    {
        LockQueue();
        while (IsEmpty()) //队列为空
        {
            NotifyProduct();
#ifdef DEBUGOPEN
            std::cout << "queue empty,notify product data,consume stop!!" << std::endl;
#endif
            ConsumeWait();
        }
        //队列不为空
        data = q.front();
        q.pop();
        NotifyProduct();
        UnlockQueue();
    }
    ~MQUEUE()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&full);
        pthread_cond_destroy(&empty);
    }

    pthread_t tid;
};

int mdata_push(MQUEUE *mq, MDATA *md);

#endif
