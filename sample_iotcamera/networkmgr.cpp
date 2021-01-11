

#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <time.h>
#include <netinet/if_ether.h>
#include <linux/filter.h>
#include <linux/if_packet.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <error.h>
#include <net/route.h>
#include "dhclient.h"
#include "networkmgr.h"

static int net_detect(char *net_name)
{
    int skfd = 0;
    struct ifreq ifr;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd < 0)
    {
        printf("%s:%d Open socket error!\n", __FILE__, __LINE__);
        return -1;
    }

    strcpy(ifr.ifr_name, net_name);

    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        printf("%s:%d IOCTL error!\n", __FILE__, __LINE__);
        printf("Maybe ethernet inferface %s is not valid!", ifr.ifr_name);
        close(skfd);
        return -1;
    }

    close(skfd);

    if (ifr.ifr_flags & IFF_RUNNING)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int SetIfAddr(char *ifname, char *Ipaddr, char *mask, char *gateway)
{
    int fd;
    int rc;
    struct ifreq ifr;
    struct sockaddr_in *sin;
    struct rtentry rt;

    printf("SetIfAddr %s %s %s\n", Ipaddr, mask, gateway);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket   error");
        return -1;
    }
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifname);
    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    sin->sin_family = AF_INET;

    //ipaddr
    if (inet_aton(Ipaddr, &(sin->sin_addr)) < 0)
    {
        perror("inet_aton   error");
        return -2;
    }

    if (ioctl(fd, SIOCSIFADDR, &ifr) < 0)
    {
        perror("ioctl   SIOCSIFADDR   error");
        return -3;
    }

    //netmask
    if (inet_aton(mask, &(sin->sin_addr)) < 0)
    {
        perror("inet_pton   error");
        return -4;
    }
    if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
    {
        perror("ioctl");
        return -5;
    }

    //gateway
    memset(&rt, 0, sizeof(struct rtentry));
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    if (inet_aton(gateway, &sin->sin_addr) < 0)
    {
        printf("inet_aton error\n");
    }
    memcpy(&rt.rt_gateway, sin, sizeof(struct sockaddr_in));
    ((struct sockaddr_in *)&rt.rt_dst)->sin_family = AF_INET;
    ((struct sockaddr_in *)&rt.rt_genmask)->sin_family = AF_INET;
    rt.rt_flags = RTF_GATEWAY;
    if (ioctl(fd, SIOCADDRT, &rt) < 0)
    {
        printf("ioctl(SIOCADDRT) error in set_default_route\n");
        close(fd);
        return -1;
    }
    close(fd);
    return rc;
}
//设置ip地址
int SetIfAddr2(char *ifname, unsigned int Ipaddr, unsigned int mask, unsigned int gateway)
{
    char A[32];
    char B[32];
    char C[32];
    struct in_addr ipadr, maskadr, gateadr;
    ipadr.s_addr = Ipaddr;
    printf("ipadr=%s\n", inet_ntoa(ipadr));
    snprintf(A, sizeof(A), "%s", inet_ntoa(ipadr));

    maskadr.s_addr = mask;
    printf("maskadr=%s\n", inet_ntoa(maskadr));
    snprintf(B, sizeof(B), "%s", inet_ntoa(maskadr));

    gateadr.s_addr = gateway;
    printf("gateadr=%s\n", inet_ntoa(gateadr));
    snprintf(C, sizeof(C), "%s", inet_ntoa(gateadr));

    SetIfAddr(ifname, A, B, C);
}

NetWorkMgr::NetWorkMgr()
{
    NTStatus = NT_STATUS_INIT;
    //
}

NetWorkMgr::~NetWorkMgr()
{
    //
}

int NetWorkMgr::run()
{

    for (;;)
    {
        switch (NTStatus)
        {
        case NT_STATUS_INIT:
            NTStatus = NT_STATUS_CHECKIF;
            break;
        case NT_STATUS_CHECKIF:
            if (net_detect("eth1") != 1)
            {
                system("ifconfig eth1 up");
                sleep(1);
            }
            else
            {
                NTStatus = NT_STATUS_CONFIP;
            }
            break;
        case NT_STATUS_CONFIP:
            system("echo nameserver 114.114.114.114 > /etc/resolv.conf");
            //SetIfAddr("eth1", "192.168.0.11", "255.255.255.0", "192.168.0.1");
            dhcpmain("eth1");
            NTStatus = NT_STATUS_READY;
            break;

        case NT_STATUS_READY:
            sleep(1);
            break;
        }
    }
    return 0;
}
