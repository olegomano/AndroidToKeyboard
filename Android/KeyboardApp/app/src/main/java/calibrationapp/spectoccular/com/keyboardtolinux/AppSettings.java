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
    private static final String FB_SCALE = "FRAME_BUFFER_SCALE";
    private SharedPreferences settings;
    private int mouseScrollMod = 1;
    private int mouseDragMod = 1;
    private int framebufferResW = 1280;
    private int framebufferResH = 720;
    private int downScale = 1;


    public AppSettings(Context c){
        settings = c.getSharedPreferences(SCROLL_MOD_KEY, Context.MODE_PRIVATE);
        mouseScrollMod = settings.getInt(SCROLL_MOD_KEY, 1);
        mouseDragMod = settings.getInt(DRAG_MOD_KEY,1);
        framebufferResH = settings.getInt(FB_H, 720);
        framebufferResW = settings.getInt(FB_W,1280);
        downScale = settings.getInt(FB_SCALE,1);
        Log.d("AppSettings","Loaded Settings:  Drag, Scroll:" + mouseDragMod + " , " + mouseScrollMod);
    }

    public int getMouseDrag(){
        return mouseDragMod;
    }

    public int getMouseScroll(){
        return mouseScrollMod;
    }

    public void setMouseScroll(int f){
        mouseScrollMod = f;
    }

    public void setMouseDrag(int f){
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
        editor.putInt(SCROLL_MOD_KEY, mouseScrollMod);
        editor.putInt(DRAG_MOD_KEY, mouseDragMod);
        editor.putInt(FB_H, framebufferResH);
        editor.putInt(FB_W, framebufferResW);
        editor.putInt(FB_SCALE,downScale);
        editor.apply();
    }
}
