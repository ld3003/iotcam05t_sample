#ifndef __ethmgr__
#define __ethmgr__

#include "CThread.h"


class ETHMGR : public CThread
{
public:
    ETHMGR();
    virtual ~ETHMGR(){};
    int netState();
    int run();

private:

    int wifiworkmod;

};

#endif
