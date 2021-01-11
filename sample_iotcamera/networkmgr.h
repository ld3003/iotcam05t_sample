#ifndef __NTMGR_H__
#define __NTMGR_H__

//#include "Common.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define THREAD_CLASS_NAME_NTM NTMThread
class THREAD_CLASS_NAME_NTM {
public:
private:
     pthread_t tid;
 private:
     static void * start_thread(void *arg)
	 {
		THREAD_CLASS_NAME_NTM *ptr = (THREAD_CLASS_NAME_NTM *)arg;
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


//int net_detect(char *net_name);
int SetIfAddr(char *ifname, char *Ipaddr, char *mask, char *gateway);
int SetIfAddr2(char *ifname, unsigned int Ipaddr, unsigned int mask, unsigned int gateway);

typedef struct _NETWORK_INFO {
	int Type;
	int Status;
}NETWORK_INFO;

enum {
	NT_STATUS_INIT,
	NT_STATUS_CHECKIF,
	NT_STATUS_CONFIP,
	NT_STATUS_READY,

};

class NetWorkMgr :public THREAD_CLASS_NAME_NTM
{
public:

	NetWorkMgr();
	virtual ~NetWorkMgr();
    int run();
	int getNetWorkType();
	void setNetWorkType(int type);
	NETWORK_INFO getNetWorkInfo();




private:

	unsigned char NTStatus;



};


#endif /* __NTMGR_H__ */



