#ifndef __wifimgr__
#define __wifimgr__

#include "CThread.h"

#define WIFI_WORKMOD_AP 0
#define WIFI_WORKMOD_STA 1

class WIFIMGR : public CThread
{
public:
    WIFIMGR();
    virtual ~WIFIMGR(){};

    int hostapd_ethdhcpd(char *ssid, char *pwd);
    int sta_eth(char *ssid, char *pwd);

    int netState(char *name);
    int writeconf(char *ssid, char *pwd);

    int run();

private:
    int wifiworkmod;
    char staconf[512];
};

#endif
