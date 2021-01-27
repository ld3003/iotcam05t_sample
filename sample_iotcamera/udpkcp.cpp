
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#include "KCP/ikcp.h"

#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>

#define SERVER_PORT 8888
#define BUFF_LEN 512
#define SERVER_IP "47.104.166.126"

static int client_fd;
static ikcpcb *kcp1;
static unsigned int lastrecvtim = 0;
static pthread_mutex_t kcpThreadLock;

/* get system time */
static inline void itimeofday(long *sec, long *usec)
{
#if defined(__unix)
    struct timeval time;
    gettimeofday(&time, NULL);
    if (sec)
        *sec = time.tv_sec;
    if (usec)
        *usec = time.tv_usec;
#else
    static long mode = 0, addsec = 0;
    BOOL retval;
    static IINT64 freq = 1;
    IINT64 qpc;
    if (mode == 0)
    {
        retval = QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
        freq = (freq == 0) ? 1 : freq;
        retval = QueryPerformanceCounter((LARGE_INTEGER *)&qpc);
        addsec = (long)time(NULL);
        addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
        mode = 1;
    }
    retval = QueryPerformanceCounter((LARGE_INTEGER *)&qpc);
    retval = retval * 2;
    if (sec)
        *sec = (long)(qpc / freq) + addsec;
    if (usec)
        *usec = (long)((qpc % freq) * 1000000 / freq);
#endif
}

/* get clock in millisecond 64 */
static inline IINT64 iclock64(void)
{
    long s, u;
    IINT64 value;
    itimeofday(&s, &u);
    value = ((IINT64)s) * 1000 + (u / 1000);
    return value;
}

static inline IUINT32 iclock()
{
    return (IUINT32)(iclock64() & 0xfffffffful);
}

/* sleep in millisecond */
static inline void isleep(unsigned long millisecond)
{
#ifdef __unix /* usleep( time * 1000 ); */
    struct timespec ts;
    ts.tv_sec = (time_t)(millisecond / 1000);
    ts.tv_nsec = (long)((millisecond % 1000) * 1000000);
    /*nanosleep(&ts, NULL);*/
    usleep((millisecond << 10) - (millisecond << 4) - (millisecond << 3));
#elif defined(_WIN32)
    Sleep(millisecond);
#endif
}

void *udp_msg_sender(void *argp)
{
    int fd = (int)argp;
    socklen_t len;
    int hr;
    char buffer[BUFF_LEN];
    struct sockaddr_in clent_addr;

    while (1)
    {
        memset(buffer, 0, BUFF_LEN);
        hr = recvfrom(fd, buffer, BUFF_LEN, 0, (struct sockaddr *)&clent_addr, &len);

        if (hr == -1)
        {
            //printf("recieve data fail!\n");
            continue;
        }

        //printf("ikcp_input: hr: %d\n", hr);
        // 如果 p1收到udp，则作为下层协议输入到kcp1
        pthread_mutex_lock(&kcpThreadLock);
        ikcp_input(kcp1, buffer, hr);

        while (1)
        {
            hr = ikcp_recv(kcp1, buffer, 1024);
            // 没有收到包就退出
            if (hr <= 0)
                break;
            else
            {
                lastrecvtim = 0;
                printf("recvkcp data");
            }
        }

        pthread_mutex_unlock(&kcpThreadLock);
    }
}

// 模拟网络：模拟发送一个 udp包
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    //union { int id; void *ptr; } parameter;
    //parameter.ptr = user;

    struct sockaddr_in ser_addr;
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    ser_addr.sin_port = htons(SERVER_PORT);

    //printf("%s call udp sendto\n", __func__);printf("%s call udp sendto\n", __func__);
    sendto(client_fd, buf, len, 0, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in));

    return 0;
}

int initKCP()
{
    kcp1 = ikcp_create(1, (void *)0);

    // 设置kcp的下层输出，这里为 udp_output，模拟udp网络输出函数
    //kcp1->output = udp_output;
    ikcp_setoutput(kcp1, udp_output);

    // 配置窗口大小：平均延迟200ms，每20ms发送一个包，
    // 而考虑到丢包重发，设置最大收发窗口为128
    ikcp_wndsize(kcp1, 2048, 2048);

    // 启动快速模式
    // 第二个参数 nodelay-启用以后若干常规加速将启动
    // 第三个参数 interval为内部处理时钟，默认设置为 10ms
    // 第四个参数 resend为快速重传指标，设置为2
    // 第五个参数 为是否禁用常规流控，这里禁止
    ikcp_nodelay(kcp1, 1, 10, 2, 1);
    kcp1->rx_minrto = 10;
    kcp1->fastresend = 1;

    return 0;
}

void *handle_update(void *argp)
{
    pthread_mutex_lock(&kcpThreadLock);
    while (1)
    {
        lastrecvtim += 1;
        isleep(1);
        ikcp_update(kcp1, iclock());
        if (lastrecvtim > 5000)
        {

            ikcp_release(kcp1);
            initKCP();
        }
    }
    pthread_mutex_unlock(&kcpThreadLock);
}

int sendKcpData(const char *buffer, int len)
{
    int ret = 0;
    pthread_mutex_lock(&kcpThreadLock);
    ret = ikcp_send(kcp1, buffer, len);
    return ret;
}

int udpKcpInit()
{
    int ret;
    struct sockaddr_in ser_addr;

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0)
    {
        printf("create socket fail!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(0);

    ret = bind(client_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
    if (ret < 0)
    {
        printf("socket bind fail!\n");
        return -1;
    }

    // 创建两个端点的 kcp对象，第一个参数 conv是会话编号，同一个会话需要相同
    // 最后一个是 user参数，用来传递标识
    initKCP();

    pthread_t recvdata_id;
    pthread_t update_id;
    pthread_create(&recvdata_id, NULL, udp_msg_sender, (void *)client_fd);
    pthread_create(&update_id, NULL, handle_update, NULL);

#if 0
	pthread_join(recvdata_id, NULL);
	pthread_join(update_id, NULL);

	close(client_fd);
#endif

    return 0;
}