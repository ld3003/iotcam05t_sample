#ifndef __rtsp_server_h__
#define __rtsp_server_h__

#include "CThread.h"

#include "rtsp/RtspServer.h"
#include "rtsp/NetInterface.h"
#include <thread>
#include <memory>
#include <iostream>
#include <string>
#include "rtsp/Pipe.h"
#include "rtsp/RingBuffer.h"
#include "rtsp_server.h"

int rtspserver();
void rtspsendFrame(unsigned char *frame, int len);

class RTSPServer : public CThread
{
public:
    RTSPServer();
    virtual ~RTSPServer();
    int run();
    int rtspsendFrame(unsigned char *frame, int len);

private:
    int initRtspSrv();

    xop::MediaSessionId mSessionId;
    xop::MediaSession *mSession;
    xop::RtspServer *mRtspServer;
    xop::EventLoop *mEventLoop;

    int clients;
};

#endif