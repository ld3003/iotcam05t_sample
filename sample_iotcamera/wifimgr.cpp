#include "wifimgr.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 1024

struct pidlist
{
    int pid[32];
    int cnt;
};
//从名称获取pid
static void getPidByName(char *task_name, struct pidlist *plist)
{
    DIR *dir;
    struct dirent *ptr;
    FILE *fp;
    char filepath[50];      //大小随意，能装下cmdline文件的路径即可
    char cur_task_name[50]; //大小随意，能装下要识别的命令行文本即可
    char buf[BUF_SIZE];
    dir = opendir("/proc"); //打开路径
    plist->cnt = 0;
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL) //循环读取路径下的每一个文件/文件夹
        {
            //如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (DT_DIR != ptr->d_type)
                continue;

            sprintf(filepath, "/proc/%s/status", ptr->d_name); //生成要读取的文件的路径
            fp = fopen(filepath, "r");                         //打开文件
            if (NULL != fp)
            {
                if (fgets(buf, BUF_SIZE - 1, fp) == NULL)
                {
                    fclose(fp);
                    continue;
                }
                sscanf(buf, "%*s %s", cur_task_name);

                //如果文件内容满足要求则打印路径的名字（即进程的PID）
                if (!strcmp(task_name, cur_task_name))
                {
                    sscanf(ptr->d_name, "%d", &plist->pid[plist->cnt]);
                    plist->cnt++;
                    printf("PID:  %s %d %d\n", ptr->d_name, plist->pid[plist->cnt - 1], plist->cnt);
                }

                fclose(fp);
            }
        }
        closedir(dir); //关闭路径
    }
}

static void getNameByPid(pid_t pid, char *task_name)
{
    char proc_pid_path[BUF_SIZE];
    char buf[BUF_SIZE];

    sprintf(proc_pid_path, "/proc/%d/status", pid);
    FILE *fp = fopen(proc_pid_path, "r");
    if (NULL != fp)
    {
        if (fgets(buf, BUF_SIZE - 1, fp) == NULL)
        {
            fclose(fp);
        }
        fclose(fp);
        sscanf(buf, "%*s %s", task_name);
    }
}

static void killdhcpc()
{
    int i = 0;

    char task_name[50];
    struct pidlist plist;

    getPidByName("udhcpc", &plist);

    for (i = 0; i < plist.cnt; i++)
    {
        int retval = kill(plist.pid[i], SIGKILL);

        if (retval)
        {
            puts("kill udhcpc failed.");
            perror("kill");
        }
    }
}

static void killdhcpd()
{
    int i = 0;

    char task_name[50];
    struct pidlist plist;

    getPidByName("udhcpd", &plist);

    for (i = 0; i < plist.cnt; i++)
    {
        int retval = kill(plist.pid[i], SIGKILL);

        if (retval)
        {
            puts("kill udhcpd failed.");
            perror("kill");
        }
    }
}

static void killwpasupp()
{
    int i = 0;

    char task_name[50];
    struct pidlist plist;

    getPidByName("wpa_supplicant", &plist);

    for (i = 0; i < plist.cnt; i++)
    {
        int retval = kill(plist.pid[i], SIGKILL);

        if (retval)
        {
            puts("kill wpa_supplicant failed.");
            perror("kill");
        }
    }
}

static void killhostapd()
{
    int i = 0;

    char task_name[50];
    struct pidlist plist;

    getPidByName("hostapd", &plist);

    for (i = 0; i < plist.cnt; i++)
    {
        int retval = kill(plist.pid[i], SIGKILL);

        if (retval)
        {
            puts("kill hostapd failed.");
            perror("kill");
        }
    }
}

WIFIMGR::WIFIMGR()
{
    wifiworkmod = WIFI_WORKMOD_AP;
    //printf("111111111111111111 %d %d\n", wifiworkmod, WIFI_WORKMOD_AP);
}

int WIFIMGR::run()
{

    for (;;)
    {
        if (wifiworkmod == WIFI_WORKMOD_AP)
        {
            if (netState("wlan0") && netState("eth0"))
            {
                hostapd_ethdhcpd(0, 0);
                for (;;)
                {
                    sleep(0xFFFFFF);
                }
            }
            else
            {
                system("ifconfig eth0 up");
                system("ifconfig wlan0 up");
            }
            //
        }

        sleep(1);
    }

    return 0;
}


int WIFIMGR::hostapd_ethdhcpd(char *ssid, char *pwd)
{
    killdhcpc();
    killdhcpd();
    killwpasupp();
    killhostapd();

    system("ifconfig eth0 up");
    system("ifconfig wlan0 up");
    sleep(1);
    system("/SDCARD/wifiap.sh");
    sleep(1);
    system("/SDCARD/ethdhcpd.sh");

    return 0;
}

int WIFIMGR::sta_eth(char *ssid, char *pwd)
{
    killdhcpc();
    killdhcpd();
    killwpasupp();
    killhostapd();

    writeconf(ssid, pwd);

    system("ifconfig eth0 up");
    system("ifconfig wlan0 up");
    sleep(1);
    system("/SDCARD/wificonn.sh");
    sleep(1);
    system("/SDCARD/ethdhcpc.sh");
}

int WIFIMGR::netState(char *name)
{

    int skfd = 0;
    struct ifreq ifr;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd < 0)
    {
        printf("%s:%d Open socket error!\n", __FILE__, __LINE__);
        return NULL;
    }

    strcpy(ifr.ifr_name, name);

    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        printf("%s:%d IOCTL error!\n", __FILE__, __LINE__);
        printf("Maybe ethernet inferface %s is not valid!", ifr.ifr_name);
        close(skfd);
        return NULL;
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

int WIFIMGR::writeconf(char *ssid, char *pwd)
{

    int fd, n;
    char *path = "/SDCARD/conf/wpa.conf";
    char apconf[512];
    snprintf(apconf, sizeof(apconf), "ctrl_interface=/var/run/wpa_supplicant\n\nupdate_config=1\nnetwork=\{\nssid=\"%s\"\nkey_mgmt=WPA-PSK\nproto=RSN WPA WPA2\npairwise=TKIP CCMP \ngroup=TKIP CCMP \npsk=\"%s\"\n}",
             ssid, pwd);

    printf("WIFI STA CONF[%s]:\n%s \n", path, apconf);

    fd = open(path, O_RDWR | O_CREAT | O_TRUNC);
    if (fd)
    {
        n = write(fd, apconf, strlen(apconf)); /*写入文件*/
        close(fd);
    }
}
