package com.poe.ppush.media;

import android.app.Activity;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;

import com.poe.ppush.LivePusher;

/**
 * @author poe 2020/04/09 .
 */
public class VideoChannel implements Camera.PreviewCallback, CameraHelper.OnChangedSizeListener {
    private static final String TAG = "VideoChannel";

    private CameraHelper cameraHelper;
    private int mBitrate;
    private int mFps;
    private boolean isLiving;
    LivePusher livePusher;

    public VideoChannel(LivePusher livePusher, Activity activity, int width, int height, int bitrate, int fps, int cameraId) {
        mBitrate = bitrate;
        mFps = fps;
        this.livePusher = livePusher;
        cameraHelper = new CameraHelper(activity, cameraId, width, height);
        cameraHelper.setPreviewCallback(this);
        cameraHelper.setOnChangedSizeListener(this);
    }

    //data   nv21 from camera directory .
    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        Log.i(TAG, "onPreviewFrame: ");
        if (isLiving) {
//            livePusher.native_pushVideo(data);
        }

    }

    @Override
    public void onChanged(int w, int h) {
        //
//        livePusher.native_setVideoEncInfo(w, h, mFps, mBitrate);
    }

    /**
     * 切换摄像头.
     */
    public void switchCamera() {
        cameraHelper.switchCamera();
    }

    /**
     * 设置摄像头预览.
     * @param surfaceHolder
     */
    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        cameraHelper.setPreviewDisplay(surfaceHolder);
    }

    /**
     * 开始推流. 改变推流状态》
     */
    public void startLive() {
        isLiving = true;
    }
}
