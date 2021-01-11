#pragma once

#include "Video.h"
#include "Audio.h"
#include "Ts.h"

extern unsigned char m_One_Frame_Buf[MAX_ONE_FRAME_SIZE];
extern TsPes m_video_tspes;
extern TsPes m_audio_tspes;
extern unsigned int WritePacketNum; //一共的包数目
//extern QByteArray btosend;
//extern rtp *m_prtp;
extern clock_t consumestart, consumeend;
extern clock_t last_vframe_clk, last_aframe_clk;
int Write_Pat(unsigned char *buf);
int Write_Pmt(unsigned char *buf);
int Take_Out_Pes(TSPKG_SESSION *session, TsPes *tspes, unsigned long time_pts, unsigned int frametype, unsigned int *videoframetype);									   //0 视频 ，1音频
int Take_Out_Pes(TSPKG_SESSION *session, TsPes *tspes, unsigned long time_pts, unsigned int frametype, unsigned int *videoframetype, unsigned char *pData, int iDataSize); //0 视频 ，1音频

int WriteAdaptive_flags_Head(Ts_Adaptation_field *ts_adaptation_field, unsigned int Videopts); //填写自适应段标志帧头的
int WriteAdaptive_flags_Tail(Ts_Adaptation_field *ts_adaptation_field);						   //填写自适应段标志帧尾的

int WriteBuf2File(unsigned int framerate);
int WriteBuf2TsFile(TSPKG_SESSION *session, unsigned int framerate, unsigned long pts  ,int iStreamType, unsigned char *pData, int iDataSize, unsigned long lTimeStamp);
int WriteBuf2TsFilebyclk(float framerate,  int iStreamType, unsigned char *pData, int iDataSize, unsigned long lTimeStamp);

int PES2TS(TsPes *ts_pes, unsigned int Video_Audio_PID, Ts_Adaptation_field *ts_adaptation_field_Head, Ts_Adaptation_field *ts_adaptation_field_Tail,
		   unsigned long Videopts, unsigned long Adudiopts);

int CreateAdaptive_Ts(Ts_Adaptation_field *ts_adaptation_field, unsigned char *buf, unsigned int AdaptiveLength);

int WriteH264Buff2File(unsigned int framerate);
