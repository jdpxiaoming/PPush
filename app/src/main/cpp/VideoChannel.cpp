//
// Created by poe on 2020/12/23.
//
#include <cstring>
#include "VideoChannel.h"
#include "include/x264.h"
#include "macro.h"

/**
 * 发送非关键帧.
 * @param type
 * @param payload
 * @param i_payload
 */
void VideoChannel::sendFrame(int type, uint8_t *payload, int i_payload) {

    if(payload[2] == 0x00){
        i_payload  -= 4;
        payload += 4;
    } else {
        i_payload -= 3;
        payload += 3;
    }

    //看表
    int bodySize = 9 +i_payload;
    RTMPPacket  *packet = new RTMPPacket ();

    RTMPPacket_Alloc(packet , bodySize);


    packet->m_body[0] = 0x27;
    if(type == NAL_SLICE_IDR){
        packet->m_body[0] = 0x17;
        LOGE("关键帧");
    }

    //类型
    packet->m_body[1] = 0x01;
    //时间戳
    packet->m_body[2] = 0x00;
    packet->m_body[3] = 0x00;
    packet->m_body[4] = 0x00;

    //数据长度4个字节
    packet->m_body[5] = (i_payload >> 24)  && 0xff;
    packet->m_body[6] = (i_payload >> 16)  && 0xff;
    packet->m_body[7] = (i_payload >> 8)  && 0xff;
    packet->m_body[8] = (i_payload >> 0)  && 0xff;


    //图片数据
    memcpy(&packet->m_body[9] , payload , i_payload);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 0x10;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    videoCallback(packet);
}

void VideoChannel::setVideoEncInfo(int width, int height, int fps, int bitrate) {

    this->mWidth = width;
    this->mHeight = height;
    this->mFps = fps;
    this->mBitrate = bitrate;
    this->ySize = width * height;
    this->uvSize = ySize/4;

    //初始化x264的编码器.
    x264_param_t param;
    x264_param_default_preset(&param , "ultrafast" , "zerolatency");

    //baseline 3.2 复杂度
    param.i_level_idc = 32;
    //图像模式 输入NV21 输出I420
    param.i_csp = X264_CSP_I420;

    param.i_width = width;
    param.i_height = height;
    //无B帧，增加首开速度.
    param.i_bframe = 0;

    //i_rc_method 表示码率控制，CQP（恒定质量） ，CRF(恒定码率), ABR (平均码率)
    param.rc.i_rc_method = X264_RC_ABR ;
    //码率（比特率Kbps）
    param.rc.i_bitrate = bitrate/1000;
    //瞬时最大码率
    param.rc.i_vbv_max_bitrate = bitrate/1000*1.2;
    //设置了i_vbv_max_bitrate必须设置下面的buffer，码率控制区大小，单位Kbps
    param.rc.i_vbv_buffer_size = bitrate/1000;

    //fps = num/den
    param.i_fps_num = fps;
    param.i_fps_den = 1;

    //时间基 1/fps.
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;

    //用fps码率来控制，而不是用时间来控制
    param.b_vfr_input = 0;
    //桢距离（关键帧） 2s一个关键帧
    param.i_keyint_max = fps * 2;
    //是否赋值fps和pps放在每个I帧前面
    param.b_repeat_headers = 1;
    //多线程 0:开启多线程 1：单线程
    param.i_threads = 1;

    //设置编码质量
    x264_param_apply_profile(&param , "baseline");
    //打开解码器。
    videoCodec = x264_encoder_open(&param);

    pic_in = new x264_picture_t ();
    x264_picture_alloc(pic_in, X264_CSP_I420 , width , height);

}

/**
 * 开始数据处理.
 * @param data  NV21 bytes.
 */
void VideoChannel::encodeData(int8_t *data) {

    //解码 NV21 yuvI420
    //转码 通过pic_in

    //4行y
    memcpy(pic_in->img.plane[0] , data ,ySize);

    /**
     * NV21:
     *  v u v u v u v u
     *  v u v u v u v u
     *
     * I420：
     *  u u u u u u u u
     *  v v v v v v v v
     */
    for (int i = 0; i < uvSize ; ++i) {
        //u数据 从传入的data数据ysize之后开始取值.
        *(pic_in->img.plane[1]+i) = *(data + ySize+ i*2 +1);
        //v数据 下一行
        *(pic_in->img.plane[2]+i) = *(data + ySize+i*2);

    }

    //这个时候能直接发送吗？
    //不行必须进行x264编码后才能发送

    //打散成多个NALU
    x264_nal_t *pp_nal;
    //编码出来几个数据
    int pi_nal;
    x264_picture_t  pic_out;
    x264_encoder_encode(videoCodec , &pp_nal, &pi_nal ,pic_in , &pic_out );

    int sps_length;
    int pps_length;
    uint8_t sps[100];
    uint8_t pps[100];
    //针对每一帧的多个NALU单元做推送
    for(int i=0; i< pi_nal ; i++){
        if(pp_nal[i].i_type == NAL_SPS){//关键帧的第一个NAL头
            sps_length = pp_nal[i].i_payload -4;
            memcpy(sps, pp_nal[i].p_payload+4, sps_length);
        }else if(pp_nal[i].i_type == NAL_PPS){//图片参数集
            pps_length = pp_nal[i].i_payload -4;
            memcpy(pps , pp_nal[i].p_payload+4, pps_length);
            //关键帧带有的信息头部. 
            sendSpsPps(sps , pps , sps_length , pps_length);
        }else{
            //发送非关键帧(关键帧和非关键帧一起发送.)
            sendFrame(pp_nal[i].i_type , pp_nal[i].p_payload , pp_nal[i].i_payload);
        }
    }
}

/**
 * 发送sps和pps信息.
 * 格式 RtMP_PACKET.
 * @param sps
 * @param pps
 * @param sps_length
 * @param pps_length
 */
void VideoChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_length, int pps_length) {

    //sps,pps -> packet.
    int bodySize = 13 + sps_length +3 + pps_length;
    RTMPPacket *packet = new RTMPPacket();
    RTMPPacket_Alloc(packet , bodySize);

    //开始组装RTMP数据格式
    int i = 0;
    //固定头
    packet->m_body[i++] = 0x17;
    //类型
    packet->m_body[i++] = 0x00;
    //composition time 0x000000
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    //版本
    packet->m_body[i++] = 0x01;
    //编码规格
    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];
    packet->m_body[i++] = 0xFF;
    //整个sps
    packet->m_body[i++] = 0xE1;
    //sps长度
    packet->m_body[i++] = (sps_length >> 8) & 0xff;//低8bits.
    packet->m_body[i++] = sps_length & 0xff;        //高8bits.
    memcpy(&packet->m_body[i], sps, sps_length);
    i += sps_length;
    //pps
    packet->m_body[i++] = 0x01;
    packet->m_body[i++] = (pps_length >> 8) & 0xff;//低8bits
    packet->m_body[i++] = (pps_length) & 0xff;      //高8bits.
    memcpy(&packet->m_body[i] , pps , pps_length);

    //设置packet类型和管道
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 10;

    //sps ps 没有时间戳
    packet->m_nTimeStamp = 0;

    //不适用绝对时间
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;

    //回调给native-lib保存到队列里面.

    if(videoCallback){
        videoCallback(packet);
    }
}

void VideoChannel::setVideoCallback(VideoChannel::VideoCallback callback) {

    if(!callback) {
        return;
    }

    this->videoCallback  = callback;
}
