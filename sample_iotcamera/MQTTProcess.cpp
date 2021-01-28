#include "mqtt/mqtt.h"
#include "JSON/cJSON.h"
#include "MQTTProcess.h"
#include "EVENTLOOP/eventloop.h"

static char *get_mqtt_uuid()
{
	static char uuidstr[16];
	static unsigned int uuidcnt = 0;
	snprintf(uuidstr, sizeof(uuidstr), "DUID%d", uuidcnt++);
	return uuidstr;
}

static char *get_xintiao_msgbody(char *uuid)
{
	
	return 0;
}

class MQTTData : public Event
{
public:
	char from[128];
	char msg[256];
	MQTTData(){};
	~MQTTData(){};
	virtual int process(EventLoop *evl , Event *ev)
	{
		printf("%s %s\n", from, msg);
		cJSON *pSub;
		cJSON *pJson;
		if (NULL == msg)
		{
			return 0;
		}
		pJson = cJSON_Parse(msg);
		if (NULL == pJson)
		{
			return 0;
		}

		pSub = cJSON_GetObjectItem(pJson, "MSGID");
		if (NULL != pSub)
		{
			printf("UUID:%s\n", pSub->valuestring);
			char *str = get_xintiao_msgbody(pSub->valuestring);
			pubmsg(str);
			free(str);
		}

		cJSON_Delete(pJson);
	}
};

class MQTTXinTiaoEVENT : public Event
{
public:
	MQTTXinTiaoEVENT(){};
	~MQTTXinTiaoEVENT(){};
	virtual int process()
	{
		char *str = get_xintiao_msgbody(get_mqtt_uuid());
		pubmsg(str);
		free(str);
	}
};

int sendxintiao()
{
	return 0;
}

void startrtmp(char *url)
{

}
void stoprtmp()
{

}


static int getmsgcode(char *msg)
{
	cJSON *pJson;
	cJSON *pSub;
	if (NULL == msg)
	{
		return 0;
	}
	pJson = cJSON_Parse(msg);
	if (NULL == pJson)
	{
		printf("pJson is NULL\r\n");
		return 0;
	}

	pSub = cJSON_GetObjectItem(pJson, "MSGCODE");

	if (pSub == NULL)
	{
		printf("pSub is NULL\r\n");
		return -1;
	}
	else
	{
		printf("%s %d MSGCODE=:%d\n", __FILE__, __LINE__, pSub->valueint);
		return pSub->valueint;
	}
	cJSON_Delete(pJson);
}
static int heartbeat_rtn_process(char *msg)
{

	printf("%s %d \r\n", __FILE__, __LINE__);
	char url[1024] = {0};
	int stream, storage, audio_enable, video_enable, resolution, record_enable, light_eanble, frame_rate, bitrate, quality, sample_rate;

	cJSON *pJson;
	cJSON *pSub;
	if (NULL == msg)
	{
		return 0;
	}
	pJson = cJSON_Parse(msg);
	if (NULL == pJson)
	{
		printf("pJson is NULL\r\n");
		return 0;
	}
	pSub = cJSON_GetObjectItem(pJson, "URL");
	if (pSub == NULL)
	{
		printf("pSub is NULL\r\n");
	}
	else
	{
		sprintf(url, "%s", pSub->valuestring);
		printf("url:%s\n", url);
	}

	pSub = cJSON_GetObjectItem(pJson, "STREAM");
	if (pSub == NULL)
	{
		printf("pSub is NULL\r\n");
	}
	else
	{
		stream = pSub->valueint;
		printf("STREAM = :%d\n", stream);
	}

	pSub = cJSON_GetObjectItem(pJson, "STORAGE");
	if (pSub == NULL)
	{
		printf("pSub is NULL\r\n");
	}
	else
	{
		storage = pSub->valueint;
		printf("STORAGE = :%d\n", storage);
	}

	pSub = cJSON_GetObjectItem(pJson, "AUDIO_ENABLE");
	if (pSub == NULL)
	{
		printf("pSub is NULL\r\n");
	}
	else
	{
		audio_enable = pSub->valueint;
		printf("AUDIO_ENABLE = :%d\n", audio_enable);
	}

	pSub = cJSON_GetObjectItem(pJson, "VIDEO_ENABLE");
	if (pSub == NULL)
	{
		printf("pSub is NULL\r\n");
	}
	else
	{
		audio_enable = pSub->valueint;
		printf("VIDEO_ENABLE = :%d\n", video_enable);
	}

	pSub = cJSON_GetObjectItem(pJson, "RESOLUTION");
	if (pSub == NULL)
	{
		printf("pSub is NULL\r\n");
	}
	else
	{
		resolution = pSub->valueint;
		printf("VIDEO_ENABLE = :%d\n", resolution);
	}

	pSub = cJSON_GetObjectItem(pJson, "RECORD_ENABLE");
	if (pSub == NULL)
	{
		printf("pSub is NULL\r\n");
	}
	else
	{
		record_enable = pSub->valueint;
		printf("RECORD_ENABLE = :%d\n", record_enable);
	}

		pSub = cJSON_GetObjectItem(pJson, "LIGHT_ENABLE");
	if (pSub == NULL)
	{
		printf("pSub is NULL\r\n");
	}
	else
	{
		light_eanble = pSub->valueint;
		printf("LIGHT_ENABLE = :%d\n", light_eanble);
	}

	cJSON_Delete(pJson);



	return 0;
}
static int down_event_process(char *msg)
{
	return 0;
}


//这里应该知己传入json串，然后区分MQTT心跳应答 还是媒体事件
static void state_process(char *from, char *msg)
{

	#if 0

	int msgcode = getmsgcode(msg);

	printf("msgcode = %d \r\n", msgcode);

	if (msgcode == -1)
		return;

	if (msgcode == MSGCODE_HEARTBEAT_RTN)
	{ //处理心跳应答json

		heartbeat_rtn_process(msg);
	}
	else if (msgcode == MSGCOD_DOWN_EVENT)
	{ //处理主动下发的媒体参数变更事件
		down_event_process(msg);
	}

	return;

	char *url;
	int stream, storage;
	printf("file = %s,line = %d,url = %s,stream = %d,storage = %d\r\n", __FILE__, __LINE__, url, stream, storage);
	int state = rt_data.stream_state;

	switch (state)
	{
	case STREAM_STATE_STOP: //stopping
		if (stream == STREAM_STATE_RTMP)
		{ //应该先释放一次资源
			//stoprtmp();
			printf("start rtmp\r\n");
			startrtmp(url);
			//rt_data.stream_state=1;
		}
		else if (stream == STREAM_STATE_RTP)
		{
			printf("start rtp\r\n");
		}
		else if (stream == STREAM_STATE_STOP)
		{
			printf("do nothing\r\n");
		}
		break;
	case STREAM_STATE_RTMP: //rtmping
		if (stream == STREAM_STATE_STOP)
		{
			printf("stop rtmp\r\n");
			stoprtmp();
		}
		else if (stream == STREAM_STATE_RTMP)
		{
			printf("check if rtmp url changed\r\n");
			/* code */
		}
		else if (stream == STREAM_STATE_RTP)
		{
			printf("stop rtmp,start rtp\r\n");
		}

		break;
	case STREAM_STATE_RTP: //rtping
		break;
	}

	#endif
}

/*本地MQTT接收到数据的回调函数*/
int mqttArrived_Fun(char *from, char *msg)
{
	printf("%s %d %s %s\r\n", __FILE__,__LINE__,from, msg);
	state_process(from,msg);
	return 0;
}

int MQTTPROCESS::run()
{

	for (;;)
	{

		if (mqttcd.conn == 1)
		{
			//如果连接正常，将发送心跳包
			sendxintiao();
			sleep(10);
		}
		else
		{

			sleep(1);
		}
	}
	return 0;
}
