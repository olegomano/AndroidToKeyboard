package calibrationapp.spectoccular.com.keyboardtolinux;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

/**
 * Created by Oleg Tolstov on 9:26 PM, 3/9/16. KeyboardToLinux
 */
public class AppSettings {
    private static final String SCROLL_MOD_KEY = "MOUSE_SCROLL_MOD";
    private static final String DRAG_MOD_KEY = "MOUSE_DRAG_MOD";
    private static final String FB_W = "FRAME_BUFFER_W";
    private static final String FB_H = "FRAME_BUFFER_H";
    private SharedPreferences settings;
    private float mouseScrollMod = 1;
    private float mouseDragMod = 1;
    private int framebufferResW = 1280;
    private int framebufferResH = 720;


    public AppSettings(Context c){
        settings = c.getSharedPreferences(SCROLL_MOD_KEY, Context.MODE_PRIVATE);
        mouseScrollMod = settings.getFloat(SCROLL_MOD_KEY, 1);
        mouseDragMod = settings.getFloat(DRAG_MOD_KEY,1);
       // framebufferResH = settings.getInt(FB_H, 720);
       // framebufferResW = settings.getInt(FB_W,1280);
        Log.d("AppSettings","Loaded Settings:  Drag, Scroll:" + mouseDragMod + " , " + mouseScrollMod);

    }

    public int getMouseDrag(){
        return (int) mouseDragMod;
    }

    public int getMouseScroll(){
        return (int) mouseScrollMod;
    }

    public void setMouseScroll(float f){
        mouseScrollMod = f;
    }

    public void setMouseDrag(float f){
        mouseDragMod = f;
    }

    public int getFBW(){
        return framebufferResW;
    }

    public int getFBH(){
        return framebufferResH;
    }

    public void save(){
        Log.d("AppSettings","Saving Settings:  Drag, Scroll " + mouseDragMod + " , " + mouseScrollMod);
        SharedPreferences.Editor editor = settings.edit();
        editor.putFloat(SCROLL_MOD_KEY, mouseScrollMod);
        editor.putFloat(DRAG_MOD_KEY, mouseDragMod);
        editor.putInt(FB_H, framebufferResH);
        editor.putInt(FB_W, framebufferResW);
        editor.apply();
    }
}
