#include "udp.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>


#include "devmem.h"

#include "ctrlgpio.h"
UDPCli::UDPCli()
{
}

UDPCli::~UDPCli()
{
}

int UDPCli::mySystem(const char *cmd)
{
    pid_t pid;
    if (cmd == NULL)
        return 1;
    int status;
    if ((pid = fork()) < 0)
        status = -1;
    else if (0 == pid)
    {
        execl("/bin/sh", "sh", "-c", cmd, (char *)0);
        _exit(127);
    }
    else
    {
        while (waitpid(pid, &status, 0) < 0)
            if (errno != EINTR)
                return -1;
    }
    return status;
}

int UDPCli::run()
{
    char devid[128];
    char destip[64];
    //snprintf(devid, sizeof(devid), "%08x%08x%08x%08x%08x%08x", devmem(0x12020400, 0, 0), devmem(0x12020404, 0, 0), devmem(0x12020408, 0, 0), devmem(0x1202040C, 0, 0), devmem(0x12020410, 0, 0), devmem(0x12020414, 0, 0));

    for (;;)
    {

        struct timeval tv_out;
        int sock;

        char *ptr, **pptr;
        struct hostent *hptr;
        char str[32];

        sleep(1);

        ptr = "pcauth.easy-iot.cc";

        if ((hptr = gethostbyname(ptr)) == NULL)
        {
            printf(" gethostbyname error for host:%s\n", "IOTCAMERA_ERROR_DNS");
            continue;
        }

        printf("official hostname:%s\n", hptr->h_name);
        for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)\
        {
            printf(" alias:%s\n", *pptr);
        }

        switch (hptr->h_addrtype)
        {
        case AF_INET:
        case AF_INET6:
            pptr = hptr->h_addr_list;
            for (; *pptr != NULL; pptr++)
                printf(" address:%s\n",
                       inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
            snprintf(destip, sizeof(destip), "%s",
                     inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
            break;
        default:
            printf("unknown address type\n");
            break;
        }

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock <= 0)
            continue;

        tv_out.tv_sec = 5;
        tv_out.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(16571);
        server.sin_addr.s_addr = inet_addr(destip);

        snprintf((char*)recvudp_conf_buf, sizeof(recvudp_conf_buf), "SN:%s\n", devid);
        printf("%s\n", recvudp_conf_buf);
        sendto(sock, recvudp_conf_buf, 128, 0, (struct sockaddr *)&server, sizeof(server));

        memset(recvudp_conf_buf, 0x0, sizeof(recvudp_conf_buf));
        recvfrom(sock, recvudp_conf_buf, 128, 0, NULL, NULL);

        printf("recv:%s\n", recvudp_conf_buf);

        close(sock);

        mySystem((char*)recvudp_conf_buf);

    }
    return 0;
}
