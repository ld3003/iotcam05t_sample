#include "ethmgr.h"

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

ETHMGR::ETHMGR()
{
}

int ETHMGR::run()
{
    killdhcpc();
    killdhcpd();
    killwpasupp();
    killhostapd();

    system("ifconfig eth0 up");
    sleep(3);
    system("/SDCARD/ethsrv.sh");

    for (;;)
    {
        sleep(0xFFFFFF);
    }
    return 0;
}


int ETHMGR::netState()
{

    int skfd = 0;
    struct ifreq ifr;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd < 0)
    {
        printf("%s:%d Open socket error!\n", __FILE__, __LINE__);
        return NULL;
    }

    strcpy(ifr.ifr_name, "eth0");

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
