
#ifndef __WSKT__
#define __WSKT__

#include "CThread.h"
#include "wsclient.h"

using easywsclient::WebSocket;

class WSKT : public CThread
{
public:
    WSKT();
    virtual ~WSKT();
    int run();
    WebSocket *initWSKT(char *url);

private:
    int sfd;
    WebSocket *wskt;
};

#endif
