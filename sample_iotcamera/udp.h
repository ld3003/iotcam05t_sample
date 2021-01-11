#ifndef __udp__
#define __udp__

/*


*/

#include "CThread.h"

class UDPCli : public CThread
{
public:
    UDPCli();
    virtual ~UDPCli();
    int run();
    int mySystem(const char *cmd);

private:
    unsigned char recvudp_conf_buf[128];
};

#endif
