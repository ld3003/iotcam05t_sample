
#include "queue.h"

/*构造一个空队列*/
Queue *InitQueue()
{
    Queue *pqueue = (Queue *)malloc(sizeof(Queue));
    if (pqueue != NULL)
    {
        pqueue->front = NULL;
        pqueue->rear = NULL;
        pqueue->size = 0;
        pthread_mutex_init(&pqueue->q_lock, NULL);
        pthread_cond_init(&pqueue->cond, NULL);
    }
    return pqueue;
}

/*销毁一个队列*/
void DestroyQueue(Queue *pqueue)
{
    if (!pqueue)
        return;
    ClearQueue(pqueue);
    pthread_mutex_destroy(&pqueue->q_lock);
    pthread_cond_destroy(&pqueue->cond);
    free(pqueue);
    pqueue = NULL;
}

/*清空一个队列*/
void ClearQueue(Queue *pqueue)
{
    while (!IsEmpty(pqueue))
    {
        DeQueue(pqueue);
    }
}

/*判断队列是否为空*/
int IsEmpty(Queue *pqueue)
{
    if (pqueue->front == NULL && pqueue->rear == NULL && pqueue->size == 0)
        return 1;
    else
        return 0;
}

/*返回队列大小*/
int GetSize(Queue *pqueue)
{
    return pqueue->size;
}

/*返回队头元素*/
PNode GetFront(Queue *pqueue, Frame *frame)
{
    pthread_mutex_lock(&pqueue->q_lock);
    /*
	if(!IsEmpty(pqueue))
	{
		*frame = pqueue->front->frame;
	}else {
		pthread_cond_wait(&pqueue->cond, &pqueue->q_lock);
	}*/
    while (IsEmpty(pqueue))
        pthread_cond_wait(&pqueue->cond, &pqueue->q_lock);
    *frame = pqueue->front->frame;
    pthread_mutex_unlock(&pqueue->q_lock);
    return pqueue->front; //---->此处有bug，队列为空时，在锁释放后，pqueue->front可能被入队操作赋值，出现frame等于NULL，而pqueue->front不等于NULL
}

/*返回队尾元素*/

PNode GetRear(Queue *pqueue, Frame *frame)
{
    if (!IsEmpty(pqueue))
    {
        *frame = pqueue->rear->frame;
    }
    return pqueue->rear;
}

/*将新元素入队*/
PNode EnQueue(Queue *pqueue, Frame frame)
{
    PNode pnode = (PNode)malloc(sizeof(Node));
    if (pnode != NULL)
    {
        pnode->frame = frame;
        pnode->next = NULL;

        pthread_mutex_lock(&pqueue->q_lock);
        if (IsEmpty(pqueue))
        {
            pqueue->front = pnode;
        }
        else
        {
            pqueue->rear->next = pnode;
        }
        pqueue->rear = pnode;
        pqueue->size++;
        pthread_cond_signal(&pqueue->cond);
        pthread_mutex_unlock(&pqueue->q_lock);
    }
    return pnode;
}

/*队头元素出队*/
PNode DeQueue(Queue *pqueue)
{
    PNode pnode = pqueue->front;
    pthread_mutex_lock(&pqueue->q_lock);
    if (!IsEmpty(pqueue))
    {
        pqueue->size--;
        pqueue->front = pnode->next;
        free(pnode);
        if (pqueue->size == 0)
            pqueue->rear = NULL;
    }
    pthread_mutex_unlock(&pqueue->q_lock);
    return pqueue->front;
}

/*遍历队列并对各数据项调用visit函数*/
void QueueTraverse(Queue *pqueue, void (*visit)())
{
    PNode pnode = pqueue->front;
    int i = pqueue->size;
    while (i--)
    {
        visit(pnode->frame);
        pnode = pnode->next;
    }
}
