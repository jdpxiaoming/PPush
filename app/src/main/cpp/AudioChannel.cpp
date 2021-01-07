//
// Created by poe on 2021/1/5.
//
#include <cstring>
#include "pty.h"
#include "AudioChannel.h"
#include "faac.h"
#include "librtmp/rtmp.h"

//编码一帧音频数据.
void AudioChannel::encodeData(int8_t* data) {
    //编码一帧数据放入native的buffer里面.
    int bytelen = faacEncEncode(audioCodec , reinterpret_cast<int32_t *>(data), inputSamples , buffer , maxOutputBytes);

    //0xAF 0x00 解码信息数据
    //0xAF 0x01 音频数据
    if(bytelen > 0){

        RTMPPacket *packet = new RTMPPacket ;
        int bodySize = 2 + bytelen;
        RTMPPacket_Alloc(packet , bodySize);
        //拼接音频数据
        packet->m_body[0] = 0xAF;
        //编码出的数据都是0x01
        packet->m_body[1] = 0x01;
        //编码之后的aac数据.
        memcpy(&packet->m_body[2] , buffer , bytelen);
        //设置编码的基础同步信息.
        packet->m_hasAbsTimestamp = 0;//相对时间
        //数据包大小
        packet->m_nBodySize = bodySize;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO ;
        packet->m_nChannel = 0x11;//随机给防止与系统冲突即可.
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
        //回到给native-lib存放到队列。
        audioCallback(packet);
    }
}

//初始化音频编码器. faac编码器
void AudioChannel::setAudioEncInfo(int sampleInHZ, int channels) {
    //打开编码器
    audioCodec = faacEncOpen(sampleInHZ , channels , &inputSamples , &maxOutputBytes);
    //设置参数.
    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioCodec);
    //采用主流的faac编码版本mpeg4.
    config->mpegVersion = MPEG4;
    //1c 标准
    config->aacObjectType = LOW;
    //16位
    config->inputFormat = FAAC_INPUT_16BIT;
    //编码出原始数据 既不是adts 也不是adif.
    config->outputFormat = 0;
    //配置完毕，set 回去
    faacEncSetConfiguration(audioCodec , config);
    //实例化缓冲区用于底层的音频存放.
    buffer = new u_char [maxOutputBytes];
}

int AudioChannel::getInputSamples() {

    return inputSamples;
}

void AudioChannel::setAudioCallback(AudioChannel::AudioCallback audioCallback) {
    this->audioCallback = audioCallback;
}
