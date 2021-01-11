#include "wskt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

WSKT::WSKT()
{
}

WSKT::~WSKT()
{
}

WebSocket *WSKT::initWSKT(char *url)
{
    wskt = WebSocket::from_url(url);
    return wskt;
}

int WSKT::run()
{

    for (;;)
    {
        sleep(1);
    }
}
