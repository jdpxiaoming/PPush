#include <jni.h>
#include <string>
#include "x264.h"
#include "librtmp/rtmp.h"

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
}