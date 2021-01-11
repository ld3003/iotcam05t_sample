/*
Copyright (c) 2014, Pierre-Henri Symoneaux
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of LightDhcpClient nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#include "dhcp.h"
#include "net.h"
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
//#include "Common.h"

#include <net/ethernet.h>

//#include <linux/if_arp.h>


#define BUFF_SIZE 1024

//http://aschauf.landshut.org/fh/linux/udp_vs_raw/ch01s03.html

static int dhcpSetIfAddr(char *ifname, char *Ipaddr, char *mask, char *gateway)
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

void dhcpmain(char *ifname)
{

    //TODO : Add a debug mode
    char *interface = ifname;

    char cliip[32] =  {"0.0.0.0"};
    char maskip[32] = {"0.0.0.0"};
    char gateip[32] =  {"0.0.0.0"};

    #define WAITTIMEOUT 5

//    printf("Interface = %s\n", interface);

    int sock = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));

    //curt = getcurrenttime_second();
	
	//Set receive timeout
	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = WAITTIMEOUT; //10 seconds in case of latency on the network
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    //retrieve ethernet NIC index and HW address
    struct hw_eth_iface iface = find_iface(sock, interface); // TODO : Check interface existency, and print a list of possible NIC

    struct sockaddr_ll target;
    memset(&target, 0, sizeof(target));
    target.sll_family   = PF_PACKET;
    target.sll_protocol = htons(ETH_P_IP);
    target.sll_ifindex  = iface.index;
    target.sll_hatype   = ARPHRD_ETHER;
    target.sll_pkttype  = PACKET_HOST;
    target.sll_halen    = ETH_ALEN;

    memset(target.sll_addr, 0xff, 6);

    char buffer[BUFF_SIZE];

//    struct dhcp_pkt *dhcp = (struct dhcp_pkt*)(buffer + sizeof(struct udpheader) + sizeof(struct ipheader));
    struct dhcp_pkt dhcp;
    int dhcp_len = build_dhcp_discover(&dhcp, iface.hw_addr, iface.addr_len);

    int len = build_ip4_udp_pkt(buffer, BUFF_SIZE, (unsigned char*)&dhcp, dhcp_len, "0.0.0.0", "255.255.255.255", 68, 67, IPPROTO_UDP);

receive:

    //if ((getcurrenttime_second() - curt) >= WAITTIMEOUT)
    //    goto exit;
        
	//Send the packet over the network
    if(sendto(sock, buffer, len, 0, (struct sockaddr *)&target, sizeof(target)) < 0)
    {
        perror("Error while writing to socket");
        goto exit;
    }

	//Now, wait for the server response, and read it


    memset(buffer, 0, BUFF_SIZE);

    int read_len = recvfrom(sock, buffer, BUFF_SIZE, 0, NULL, NULL);

    if(read_len <= 0)
    {
        perror("Cannot read");
        goto exit;
    }

    printf("Data Recieved, length = %d\n", read_len);

    struct ipheader *rip   = (struct ipheader*) buffer;
    struct udpheader *rudp = (struct udpheader*)(buffer + sizeof(struct ipheader));
    struct dhcp_pkt *rdhcp = (struct dhcp_pkt*)(buffer + sizeof(struct udpheader) + sizeof(struct ipheader));

    //Check packet validity
	// if dest port isn't our or packet is not a dhcp one, drop the packet
    if(rip->iph_protocol != IPPROTO_UDP || rudp->udph_destport != htons(68) || !is_dhcp(rdhcp) || rdhcp->op != OP_BOOT_REPLY)
        goto receive;

//    printf("Data Recieved, length = %d\n", read_len);

	//Find the IP attributed to us by the server
    struct in_addr raddr;
    raddr.s_addr = rdhcp->yi_addr;

    printf("IPADDR=%s\n", inet_ntoa(raddr));
    sprintf(cliip,"%s",inet_ntoa(raddr));

	//Now check DHCP options, and process them
    struct dhcp_opt *opt = NULL;
    int offs = 0;
    opt = get_dhcp_option(rdhcp, &offs);
    while(opt != NULL)
    {
//      printf("OPT FOUND offset = %d\n", offs);
        switch(opt->id)
        {
            case OPTION_ROUTER: // If the option is the gateway address
				if(opt->len == 4)
				{
					raddr.s_addr = char_to_ip(opt->values);
					printf("GATEWAY=%s\n", inet_ntoa(raddr));
                    sprintf(gateip,"%s",inet_ntoa(raddr));
				}
                break;

            case OPTION_SUBNET_MASK: // If the option is the netwmask
				if(opt->len == 4)
				{
					raddr.s_addr = char_to_ip(opt->values);
					printf("NETMASK=%s\n", inet_ntoa(raddr));
                    sprintf(maskip,"%s",inet_ntoa(raddr));
				}
                break;

            case OPTION_DNS: // If option is the DNS addresses
				if(opt->len % 4 == 0)
				{
					int i = 0;
					printf("NAMESERVER=");
					int max = opt->len / 4;
					for(i = 0; i < max; i++)
					{
						raddr.s_addr = char_to_ip(opt->values + 4*i);
						printf("%s", inet_ntoa(raddr));
						if(i < max - 1)
							printf(",");
					}
					printf("\n");
				}
                break;

            default:
                break;
        }
        opt = get_dhcp_option(rdhcp, &offs);
    }


    dhcpSetIfAddr(ifname,cliip,maskip,gateip);

    exit:

    close(sock);

    

    return ;
}
