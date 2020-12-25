package com.poe.ppush;

import androidx.appcompat.app.AppCompatActivity;

import android.hardware.Camera;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;

/**
 * 视频推送demo .
 * @author poe 2020/04/09
 */
public class MainActivity extends AppCompatActivity {

    private LivePusher mLivePusher;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //keep screen on to prevent screen sleep.
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );
        setContentView(R.layout.activity_main);

        SurfaceView surfaceView = findViewById(R.id.surface_view);
        mLivePusher = new LivePusher(this,800,480,800_000,10, Camera.CameraInfo.CAMERA_FACING_FRONT);
        //set display view .
        mLivePusher.setPreviewDisplay(surfaceView.getHolder());
    }


    /**
     * start publish .
     * @param view
     */
    public void startLive(View view) {

        mLivePusher.startLive("");
    }

    /**
     * 前后摄像头切换 .
     * switch camera .
     * @param view
     */
    public void switchCamera(View view) {

    }

    /**
     * 停止直播.
     * stop publish.
     * @param view
     */
    public void stopLive(View view) {

    }
}
