package com.poe.ppush;

import android.app.Activity;
import android.util.Log;
import android.view.SurfaceHolder;

import com.poe.ppush.media.AudioChannel;
import com.poe.ppush.media.VideoChannel;

/**
 * 推送工具类.
 * @author poe 2020/04/09.
 */
public class LivePusher {
    private static final String TAG = "LivePusher";

    private AudioChannel audioChannel;
    private VideoChannel videoChannel;
    static {
        System.loadLibrary("native-lib");
    }

    /**
     * 初始化.init the push parameters.
     * @param activity
     * @param width video width .
     * @param height video height .
     * @param bitrate push video bitrate .
     * @param fps   frame per seconds .
     * @param cameraId the camera id .  0:background 1:foreground .
     */
    public LivePusher(Activity activity, int width, int height, int bitrate, int fps, int cameraId) {
        native_init();
        Log.i(TAG,stringFromJNI());
        videoChannel = new VideoChannel(this, activity, width, height, bitrate, fps, cameraId);
        audioChannel = new AudioChannel(this);

    }
    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        videoChannel.setPreviewDisplay(surfaceHolder);
    }
    public void switchCamera() {

        videoChannel.switchCamera();
    }

    public void startLive(String path) {
        native_start(path);
        videoChannel.startLive();
    }
    public native void native_init();
    public native void native_setVideoEncInfo(int width, int height, int fps, int bitrate);
    public native void native_start(String path);
    public native void native_pushVideo(byte[] data);

    public native String stringFromJNI();
}
