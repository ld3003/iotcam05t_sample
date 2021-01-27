#ifndef __udp_kcp__
#define __udp_kcp__

#ifndef __wifimgr__
#define __wifimgr__

#include "CThread.h"
#include "KCP/ikcp.h"
#include "EVENTLOOP/eventloop.h"


int sendKcpData(const char *buffer, int len);
int udpKcpInit();


#endif

#endif