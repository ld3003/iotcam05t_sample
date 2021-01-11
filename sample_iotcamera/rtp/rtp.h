#ifndef RTP_H
#define RTP_H

#include "jrtplib3/rtpsession.h"
#include "jrtplib3/rtppacket.h"
#include "jrtplib3/rtpudpv4transmitter.h"
#include "jrtplib3/rtpipv4address.h"
#include "jrtplib3/rtpsessionparams.h"
#include "jrtplib3/rtperrors.h"
#include "jrtplib3/rtpconfig.h"
#include "jrtplib3/rtpsourcedata.h"

#include <iostream>
#include <vector>
//#include <QHostAddress>

#define SSRC 200
enum PAYLOAD_TYPE
{
    encode_pcmu = 0,
    encode_reserved,
    encode_reserved1,
    encode_gsm,
    encode_g723,
    encode_dvi4_8000,
    encode_dvi4_16000,
    encode_lpc,
    encode_pcma,
    encode_g722,
    encode_l16_channel2,
    encode_l16_channel1,
    encode_qcelp,
    encode_cn,
    encode_mpa,
    encode_g728,
    encode_dvi4_11025,
    encode_dvi4_22050,
    encode_g729,
    encode_reserved2,
    encode_unassigned1,
    encode_unassigned2,
    encode_unassigned3,
    encode_unassigned4,
    encode_unassigned5,
    encode_celb,
    encode_jpeg,
    encode_unassigned6,
    encode_nv,
    encode_unassigned7,
    encode_unassigned8,
    encode_h261,
    encode_mpv,
    encode_mp2t,
    encode_h263,

};
using namespace jrtplib;
class MyRTPSession : public jrtplib::RTPSession
{

protected:
};

class rtp
{

public:
    rtp(const char *destip, const ushort destport, const ushort localport, const double timestamp = (1.0 / 8000.0));
    rtp(const ushort localport = 5050, const double timestamp = (1.0 / 8000.0));
    ~rtp();
    void checkerror(int rtperr);
    std::vector<unsigned char> getPack();

    int sendPack(std::vector<unsigned char> packet);
    int sendPack(std::vector<unsigned char> packet, int len);
    int sendPack(unsigned char *data, int dlen);
    int sendPackex(std::vector<unsigned char> packet, uint32_t timestampinc);
    void setPayloadtype(unsigned char type);
    int pushPack(unsigned char *data, int dlen, uint32_t timestampinc);

    void readloop();

private:
    MyRTPSession sess;
    std::vector<unsigned char> array;
    std::vector<unsigned char> recvarray;
    unsigned char pltype;
    int status;
    struct timeval tvpre;
};

#endif // RTP_H
