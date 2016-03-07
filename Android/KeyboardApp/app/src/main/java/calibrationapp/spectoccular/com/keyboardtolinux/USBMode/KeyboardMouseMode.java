package calibrationapp.spectoccular.com.keyboardtolinux.USBMode;

import android.content.Context;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.LinearLayout;

import java.nio.ByteBuffer;

import calibrationapp.spectoccular.com.keyboardtolinux.MainActivity;
import calibrationapp.spectoccular.com.keyboardtolinux.UAccessory;

/**
 * Created by Oleg Tolstov on 3:11 PM, 3/4/16. KeyboardToLinux
 */
public class KeyboardMouseMode extends USBMode implements View.OnTouchListener{
    private static final int KEY_EVENT = 1;
    private static final int MOUSE_MOVE_EVENT = 2;
    private static final int MOUSE_CLICK_EVENT = 3;

    private LinearLayout mView;
    private ByteBuffer bb;

    byte[] sendDataBuffer = new byte[UAccessory.WRITE_PACKET_SIZE];
    public KeyboardMouseMode(UAccessory mDev, Context context) {
        super(mDev,context);
        bb = ByteBuffer.wrap(sendDataBuffer);
        mView = new LinearLayout(context);
        mView.addView(MainActivity.DEBUG_VIEW.getView());
        MainActivity.DEBUG_VIEW.getView().setOnTouchListener(this);
        mView.setOnTouchListener(this);
    }

    @Override
    public View getView() {
        return mView;
    }

    public void keyPressed(int keyCode, KeyEvent event){
        MainActivity.DEBUG_VIEW.printConsole("KeyCode: " + keyCode + "Scan code" + event.getScanCode() + " Chars: " + event.getUnicodeChar());
        Log.d("Main", "KeyPressed: " + "KeyCode: " + keyCode + "Scan code" + event.getScanCode() + " Chars: " + event.getUnicodeChar());
        bb.asIntBuffer().put(0, KEY_EVENT);
        bb.asIntBuffer().put(1,event.getUnicodeChar());
        event.getModifiers();
        if(device.isOpen()) {
            if(event.getKeyCode() != 59)
                device.sendData(sendDataBuffer,sendDataBuffer.length);
        }
    }

    public void onSelected(){
        super.onSelected();
        showSoftInput();
    }

    private void showSoftInput(){
        InputMethodManager imm = (InputMethodManager) mContext.getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, InputMethodManager.HIDE_IMPLICIT_ONLY);
    }

    private void onClick(){
        Log.d("KeyBoardMouseMode","Click");

    }

    private void onDrag(float dx, float dy){
        Log.d("KeyBoardMouseMode","Drag " + dx + " " + dy);
        int dxInt = (int) dx;
        int dyInt = (int) dy;
        bb.asIntBuffer().put(0, MOUSE_MOVE_EVENT);
        bb.asIntBuffer().put(1,dxInt);
        bb.asIntBuffer().put(2,dyInt);
        device.sendData(sendDataBuffer,sendDataBuffer.length);
    }

    private void onScroll(float ds){

    }


    private float minDisplace = 9;
    private float prevTouchX;

    private float prevTouchY;
    private boolean isDrag = false;
    private long clickThreashHold = 1500;
    @Override
    public boolean onTouch(View v, MotionEvent event) {
        Log.d("KeyboardMouseMode","TouchEvent");
        if (event.getPointerCount() == 1){
            if(event.getAction() == MotionEvent.ACTION_DOWN){
                prevTouchX =  event.getX();
                prevTouchY =  event.getY();
            }
            else if(event.getAction() == MotionEvent.ACTION_UP){
                if(event.getDownTime() > clickThreashHold && !isDrag){
                    onClick();
                }
                isDrag = false;
                return false;
            }else if(event.getAction() == MotionEvent.ACTION_MOVE){
                float dx = event.getX() - prevTouchX;
                float dy = event.getY() - prevTouchY;
                if(dx*dx + dy*dy < minDisplace){ //event didnt move enouf to be considered a real drag
                    return true;
                }
                prevTouchX = event.getX();
                prevTouchY = event.getY();
                isDrag = true;
                onDrag(dx,dy);
                return true;
            }
        }else{

        }
        return true;
    }
}
