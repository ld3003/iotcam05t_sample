#include "rtp.h"
#include <unistd.h>
#include "../rtmp/srs_lib_ts_demuxer.hpp"

rtp::rtp(const char * destip, const ushort destport, const ushort localport, double timestamp)
{
    jrtplib::RTPUDPv4TransmissionParams transparams;
    jrtplib::RTPSessionParams sessparams;

    uint32_t ip = inet_addr(destip);
    if (ip == INADDR_NONE)
    {
        std::cerr << "Bad IP address specified" << std::endl;
        return;
    }
    ip = ntohl(ip);

    sessparams.SetOwnTimestampUnit(timestamp);

    sessparams.SetAcceptOwnPackets(false);
    sessparams.SetMaximumPacketSize(63*1024);//the max packetsize is 63*1024
    sessparams.SetUsePollThread(true);
   // sessparams.SetUsePredefinedSSRC(true);  //设置使用预先定义的SSRC
   // sessparams.SetPredefinedSSRC(SSRC);

  // sessparams.SetMinimumRTCPTransmissionInterval(1);


    transparams.SetPortbase(localport);

    sess.SetDefaultMark( false );
    sess.SetDefaultTimestampIncrement( 0 );


    int status = sess.Create(sessparams,&transparams);



    checkerror(status);
    jrtplib::RTPIPv4Address addr(ip,destport);

    status = sess.AddDestination(addr);
    checkerror(status);


}
rtp::rtp(const ushort localport, const double timestamp)
{
    jrtplib::RTPUDPv4TransmissionParams transparams;
    jrtplib::RTPSessionParams sessparams;

    sessparams.SetOwnTimestampUnit(timestamp);

    sessparams.SetAcceptOwnPackets(true);
    sessparams.SetMaximumPacketSize(63*1024);//the max packetsize is 63*1024
    sessparams.SetUsePollThread(true);
    transparams.SetPortbase(localport);

    int status = sess.Create(sessparams,&transparams);
    checkerror(status);

  //  setPayloadtype(33);
}

rtp::~rtp()
{
    sess.BYEDestroy(jrtplib::RTPTime(10,0),0,0);
}

extern "C" 
{int send_aac2ao(unsigned char * aacdat , int len);};
void rtp::readloop()
{

    #if 1
    //int status;

    sess.WaitForIncomingData(RTPTime(1));

    sess.BeginDataAccess();
    status = sess.Poll();
    checkerror(status);

    if (sess.GotoFirstSourceWithData())
    {
        do
        {
            jrtplib::RTPPacket *pack;

            while ((pack = sess.GetNextPacket()) != NULL)
            {
                recvarray.clear();
                //QBuffer buffer(&array);
                //qDebug()<<"adfadfafadfa";
                //buffer.open(QIODevice::WriteOnly);
                //QDataStream out(&buffer);
                //out.writeRawData((char*)pack->GetPayloadData(),pack->GetPayloadLength());

                //printf("pack->GetPayloadLength %d\n",pack->GetPayloadLength());

                send_aac2ao((unsigned char*)pack->GetPayloadData(),pack->GetPayloadLength());

                //emit readyRead(pack->GetPayloadLength());
                sess.DeletePacket(pack);


            }
        } while (sess.GotoNextSourceWithData());
    }

    //printf("######################## \n");

    sess.EndDataAccess();

#ifndef RTP_SUPPORT_THREAD
    status = sess.Poll();
    checkerror(status);
#endif // RTP_SUPPORT_THREAD

#endif
}


void rtp::checkerror(int rtperr)
{
    if (rtperr < 0)
    {
        std::cout << "ERROR: " << jrtplib::RTPGetErrorString(rtperr) << std::endl;
        //exit(-1);
    }
}

#if 1
std::vector<unsigned char> rtp::getPack()
{
    return array;
}

int rtp::sendPack(std::vector<unsigned char> packet)
{
    int status = sess.SendPacket((void *)packet.data(),packet.size(),pltype,false,0);
    checkerror(status);
    return status;
}
int rtp::sendPack(std::vector<unsigned char> packet, int len)
{
    int status = sess.SendPacket((void *)packet.data(),len,pltype,false,0);
    checkerror(status);
    return status;
}
int rtp::sendPackex(std::vector<unsigned char> packet,uint32_t timestampinc)
{
    int status = sess.SendPacket((void *)packet.data(),packet.size(),pltype,false,timestampinc);
    checkerror(status);
    return status;
}


int rtp::sendPack(unsigned char*data, int dlen)
{
    struct timeval tv; 
    for(int i=0;i<dlen;i++)
        array.push_back(data[i]);

    if (array.size() >= (188*7))
    {
        gettimeofday(&tv , NULL);  
        uint timestamp_increse = (uint)(90000.0 / (1000.0 / ((tv.tv_sec - tvpre.tv_sec) * 1000.0 + (tv.tv_usec - tvpre.tv_usec) / 1000.0)));
        memcpy(&tvpre, &tv, sizeof(tvpre));
        sess.SendPacket(array.data(),array.size(),pltype,false,timestamp_increse);
        
        //usleep(5000);
        array.clear();
    }else{
        
    }
    return 0;
    
}

int rtp::pushPack(unsigned char*data, int dlen , uint32_t timestampinc)
{
    for(int i=0;i<dlen;i++)
        array.push_back(data[i]);

    return dlen;
}

void rtp::setPayloadtype(unsigned char type)
{
    pltype=type;
}
#endif
