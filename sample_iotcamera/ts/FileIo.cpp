#include "FileIo.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "Mux.h"

FILE *pVideo_H264_File = NULL;
FILE *pAudio_Aac_File = NULL;
FILE *pVideo_Audio_Ts_File = NULL;



FILE *OpenFile(char *FileName, char *OpenMode)
{
	return 0;
}

void CloseFile(FILE *pFile)
{
}

int ReadFile(FILE *pFile, unsigned char *Buffer, int BufferSize)
{

	return 0;
}



int WriteFile(TSPKG_SESSION *session, FILE *pFile, char *Buffer, int BufferSize)
{
	int WriteSize = 0;
	//printf("TS WriteFile %d\n",BufferSize);
#if 0
    WriteSize = fwrite(Buffer, 1, BufferSize, pFile);
#endif
	//mrtp->sendPack((unsigned char *)Buffer, BufferSize);
	if (session->writets != 0)
	{
		session->writets((unsigned char *)Buffer, BufferSize);
	}

#if 0
	for (int i = 0; i < TSDATA_CHN_NUM; i++)
	{
		write(tsdatapipfd[i], (unsigned char *)Buffer, BufferSize);
	}
#endif
	return BufferSize;
}
