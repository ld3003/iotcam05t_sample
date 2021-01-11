#pragma once

#include "Information.h"
#include "Ts.h"

#define INPUTH264FILENAME "G:\\test\\video_file.h264"
#define INPUTAACFILENAME "G:\\test\\audio_file.aac"
#define OUTPUTTSFILENAME "avclient.ts"

extern FILE *pVideo_H264_File;
extern FILE *pAudio_Aac_File;
extern FILE *pVideo_Audio_Ts_File;

FILE *OpenFile(char *FileName, char *OpenMode);                                   //打开文件
void CloseFile(FILE *pFile);                                                      //关闭文件
int ReadFile(FILE *pFile, unsigned char *Buffer, int BufferSize);                 //读取操作
int WriteFile(struct TSPKG_SESSION *session, FILE *pFile, char *Buffer, int BufferSize); //文件写操作
