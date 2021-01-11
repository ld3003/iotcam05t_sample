#ifndef __CTHREAD_H__
#define __CTHREAD_H__

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

class CThread {
public:
private:
     pthread_t tid;
 private:
     static void * start_thread(void *arg)
	 {
		CThread *ptr = (CThread *)arg;
    	ptr->run();  //线程的实体是run
	 }
 public:
     int start()
	 {
			if(pthread_create(&tid,NULL,start_thread,(void *)this) != 0) //´创建一个线程(必须是全局函数)
			{
				return -1;
			}
			return 0;
	 }
     virtual int run() = 0; //基类中的虚函数要么实现，要么是纯虚函数（绝对不允许声明不实现，也不纯虚）
};

#endif