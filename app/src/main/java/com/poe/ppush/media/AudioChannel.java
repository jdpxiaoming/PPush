package com.poe.ppush.media;


import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import com.poe.ppush.LivePusher;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioChannel {
    private LivePusher mLivePusher;
    private AudioRecord mAudioRecorder; //采集音频工具类.
    private int mChannels = 2;//默认双通道
    private int mChannelConfigs;
    private int minBufferSize;//最小的缓存数组.
    private ExecutorService mExecutorService;
    private boolean isLiving = false;//tag是否正在直播音频.
    private int inputSamples ;//编码器返回的最小缓冲区.


    public AudioChannel(LivePusher livePusher) {
        mLivePusher = livePusher;
        //通过线程去读取麦克风里面的数据.
        mExecutorService = Executors.newSingleThreadExecutor();

        if(mChannels == 2){
            //双通道.
            mChannelConfigs = AudioFormat.CHANNEL_IN_STEREO;
        }else{
            //单通道 .
            mChannelConfigs = AudioFormat.CHANNEL_IN_MONO;
        }
        //初始化音频编码器
        mLivePusher.native_setAudioEncInfo(44100 , mChannels);

        //16位 2个字节.
        inputSamples = mLivePusher.getInputSamples();

        //minBufferSize,因为是双通道说以*2.
        minBufferSize = AudioRecord.getMinBufferSize(44100,
                mChannelConfigs,AudioFormat.ENCODING_PCM_16BIT)*2;
        mAudioRecorder = new AudioRecord(MediaRecorder.AudioSource.MIC,
                44100,
                mChannelConfigs,
                AudioFormat.ENCODING_PCM_16BIT,
                minBufferSize>inputSamples?inputSamples:minBufferSize
                );

    }

    public void setChannels(int channels){
        this.mChannels = channels;
    }


    public void startLive(){
        isLiving =true;

        mExecutorService.submit(new AudioTask());
    }

    private class AudioTask implements  Runnable {
        @Override
        public void run() {

            mAudioRecorder.startRecording();
            //去读麦克风的数据
            byte[] buffer = new byte[minBufferSize];

            while (isLiving){
                int len = mAudioRecorder.read(buffer , 0 , buffer.length);
                mLivePusher.native_pushAudio(buffer);
            }
        }
    }
}




