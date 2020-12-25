//
// Created by poe on 2020/12/23.
//

#ifndef PPUSH_VIDEOCHANNEL_H
#define PPUSH_VIDEOCHANNEL_H

#include <x264.h>
#include <cstdint>
#include "librtmp/rtmp.h"

class VideoChannel {
    typedef void (*VideoCallback)(RTMPPacket *packet);
public:
    void setVideoEncInfo(int width, int height, int fps, int bitrate);

    void encodeData(int8_t *data);
    /**
     * 设置监听回调.
     * @param callback
     */
    void setVideoCallback(VideoCallback callback);

private:
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int ySize;//yuv ->y  4:20;
    int uvSize;//yuv ->u or v. 1:1
    x264_t* videoCodec;
    //一帧
    x264_picture_t * pic_in;

    VideoCallback  videoCallback;

    void sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_length, int pps_length);

    void sendFrame(int type , uint8_t *payload, int i_payload);
};


#endif //PPUSH_VIDEOCHANNEL_H
