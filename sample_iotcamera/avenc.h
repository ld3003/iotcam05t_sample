#ifndef Avenc_h__
#define Avenc_h__

#ifdef __cplusplus
extern "C"
{ //因为cpp文件默认定义了该宏),则采用C语言方式进行编译
#endif

    struct MEDIA_WRITE
    {
        int (*write_a)(unsigned char *buf, int len, int pts);
        int (*write_v)(unsigned char *buf, int len, int pts);
        int chn;
    };

    int regiontmp();

    /* av 控制函数 ， 用于开启或者停止编码*/
    int av_ctrl(int enable);
    int av_start();
    int av_init();

    /* 第三个编码通道的操作函数，可以将要编码的数据发送至该通道进行编码*/
    //HI_S32 SEND_TO_CHN2(unsigned char *data, int len);
    //HI_S32 INIT_ENCODE_CHM__NUM2();

    void pause_write(unsigned char flag); //暂停编码器向应用层写数据 ，但是编码器仍然继续编码 ，用于应用层无锁切换

    int write_audio(unsigned char *buf, int len);
    int write_video(int chn, unsigned char *buf, int len);

    int initavenc();

    int register_mw(struct MEDIA_WRITE *mw);
    int unregister_mw(struct MEDIA_WRITE *mw);

#ifdef __cplusplus
}
#endif

#endif