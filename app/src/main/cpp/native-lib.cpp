#include <jni.h>
#include <string>
#include <pthread.h>
#include "x264.h"
#include "librtmp/rtmp.h"
#include "VideoChannel.h"
#include "macro.h"
#include "safe_queue.h"
#include "AudioChannel.h"

//防止用户重复点击标志位.
int isStart = 0;
int readyPushing = 0;//准备开始推流。
VideoChannel* videoChannel;
AudioChannel* audioChannel;
pthread_t *pid;//开启推流线程id。
uint32_t start_time;//音视频同步需要的记录一个开始时间
SafeQueue<RTMPPacket *>  packets;//缓存数据队列,等待推送的数据集合.


void callback(RTMPPacket* packet){
    if(packet){
        //设置时间戳
        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
        //加入队列.
        packets.put(packet);
    }

}

/**
 * 释放packet.
 * @param packet
 */
void releasePackets(RTMPPacket*& packet){
    if(packet){
        RTMPPacket_Free(packet);
        delete packet;
        packet = 0;
    }

}

/**
 * 开启推流循环
 * @param args
 * @return
 */
void *start(void *args){
    char *url = static_cast<char *>(args);
    LOGE("开始推流 %s failed!",url);
     //开启RTMP网络连接
     RTMP *rtmp = 0;
     rtmp = RTMP_Alloc();

     if(!rtmp){
        LOGE("alloc rtm failed!");
        return NULL;
     }

     RTMP_Init(rtmp);
     int ret = RTMP_SetupURL(rtmp , url);

     if(!ret){
         LOGE("RTMP_SetupURL %s failed!",url);
         return NULL;
     }

     //设置超时为5s
     rtmp->Link.timeout = 5;

     //打开写开关
     RTMP_EnableWrite(rtmp);
     //连接服务器
//     RTMPPacket * packet;
     ret = RTMP_Connect(rtmp , 0/*packet*/);
//     RTMPPacket_Free(packet);
    if(!ret){
        LOGE("连接服务器 %s failed!",url);
        return NULL;
    }

    //连接流
    ret = RTMP_ConnectStream(rtmp , 0);

    if(!ret){
        LOGE("连接流 %s failed!",url);
        return NULL;
    }


    //记录推流的开始时间
    start_time = RTMP_GetTime();
    //    准备推流
    readyPushing = 1;
    //打开工作状态.
    packets.setWork(1);

    RTMPPacket *packet = 0;

    while (readyPushing){

        //取出一帧数据.
        packets.get(packet);
        LOGE("取出一帧数据");
        //判断是否可以继续推流.
        if(!readyPushing){
            break;
        }

        //如果没有流，则继续遍历
        if(!packet){
            continue;
        }

        //设置流类型，视频流 or 音频流
        packet->m_nInfoField2 = rtmp->m_stream_id;

        //send packets.
        RTMP_SendPacket(rtmp , packet , 1);

        //free the packet .
        releasePackets(packet);
    }

    //init params .
    isStart = 0;
    readyPushing  = 0;
    //stop the queue by hand.
    packets.setWork(0);
    packets.clear();
    if(rtmp){
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }

    delete url;
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_poe_ppush_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_poe_ppush_LivePusher_stringFromJNI(JNIEnv *env, jobject thiz) {
    std::string hello = "Hello from C++";

    x264_picture_t* x264Picture = new x264_picture_t;
    RTMP_Alloc();


    return env->NewStringUTF(hello.c_str());
}extern "C"
JNIEXPORT void JNICALL
Java_com_poe_ppush_LivePusher_native_1init(JNIEnv *env, jobject thiz) {
    videoChannel = new VideoChannel;
    videoChannel->setVideoCallback(callback);
    audioChannel = new AudioChannel;
    audioChannel->setAudioCallback(callback);

}extern "C"
JNIEXPORT void JNICALL
Java_com_poe_ppush_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject thiz, jint width,
                                                      jint height, jint fps, jint bitrate) {
    if(!videoChannel){
        return;
    }

    videoChannel->setVideoEncInfo(width,height,fps,bitrate);
}extern "C"
/**
 * 打开流地址.
 * @param env
 * @param thiz
 * @param path
 */
JNIEXPORT void JNICALL
Java_com_poe_ppush_LivePusher_native_1start(JNIEnv *env, jobject thiz, jstring path_) {
    //java字符串转为c的char字符集合.
    const char *path = env->GetStringUTFChars(path_, 0);

    if(isStart){
        return;
    }
    //标志位设为1表示正在开启.
    isStart = 1;

    //把url保存到native内存中
    char *url = new char[strlen(path)+1];
    strcpy(url,path);

    //开启推流线程
    pthread_create(pid, 0,start, url);
    //用完回收。
    env->ReleaseStringUTFChars(path_ , path);

}extern "C"
JNIEXPORT void JNICALL
Java_com_poe_ppush_LivePusher_native_1pushVideo(JNIEnv *env, jobject thiz, jbyteArray data_) {

     if(!videoChannel || !readyPushing){
         return;
     }
     //获取推流的数据包.
    jbyte* data =  env->GetByteArrayElements(data_ , NULL);
     //进行数据处理videoChannel.
     videoChannel->encodeData(data);

     env->ReleaseByteArrayElements(data_,data, 0 );
}extern "C"
JNIEXPORT void JNICALL
Java_com_poe_ppush_LivePusher_native_1pushAudio(JNIEnv *env, jobject thiz, jbyteArray buffer_) {
    // pcm -> aac
    // FAAC

    //获取推流的数据包.
    jbyte* data =  env->GetByteArrayElements(buffer_ , NULL);

    //进行数据处理audioChannel.
    if(!audioChannel || !readyPushing ){
        LOGE("audio channel is null!");
        return;
    }

    audioChannel->encodeData(data);

    env->ReleaseByteArrayElements(buffer_,data, 0 );

}extern "C"
JNIEXPORT void JNICALL
Java_com_poe_ppush_LivePusher_native_1setAudioEncInfo(JNIEnv *env, jobject thiz, jint sample_,
                                                      jint channels) {
    if(audioChannel){
        audioChannel->setAudioEncInfo(sample_ , channels);
    }
}extern "C"
JNIEXPORT jint JNICALL
Java_com_poe_ppush_LivePusher_getInputSamples(JNIEnv *env, jobject thiz) {

    if(audioChannel){
        return audioChannel->getInputSamples();
    }

    return -1;
}