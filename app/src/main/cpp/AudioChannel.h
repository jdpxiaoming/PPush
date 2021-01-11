//
// Created by poe on 2021/1/5.
//

#ifndef PPUSH_AUDIOCHANNEL_H
#define PPUSH_AUDIOCHANNEL_H


#include <faac.h>
#include <jni.h>
#include "librtmp/rtmp.h"

class AudioChannel {
    //给音频数据一个回调方法
    typedef void (*AudioCallback)(RTMPPacket *packet);
public:
    RTMPPacket *getAudioTag();
    void encodeData(int8_t *data);

    void setAudioEncInfo(int sampleInHZ , int channels);

    jint getInputSamples();

    void setAudioCallback(AudioCallback audioCallback);
private:
    AudioCallback  audioCallback;//音频回调接口.
    int mChannels;  //通道数
    faacEncHandle audioCodec;//编码器参数
    u_long inputSamples;    //缓冲区大小
    u_long maxOutputBytes;  //最大缓冲区大小
    u_char *buffer = 0 ;
};


#endif //PPUSH_AUDIOCHANNEL_H
