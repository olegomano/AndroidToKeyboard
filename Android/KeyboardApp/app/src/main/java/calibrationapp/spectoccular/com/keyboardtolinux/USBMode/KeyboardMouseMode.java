package calibrationapp.spectoccular.com.keyboardtolinux.USBMode;

import android.content.Context;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.LinearLayout;

import java.nio.ByteBuffer;

import calibrationapp.spectoccular.com.keyboardtolinux.AppSettings;
import calibrationapp.spectoccular.com.keyboardtolinux.MainActivity;
import calibrationapp.spectoccular.com.keyboardtolinux.UAccessory;

/**
 * Created by Oleg Tolstov on 3:11 PM, 3/4/16. KeyboardToLinux
 */
public class KeyboardMouseMode extends USBMode implements View.OnTouchListener{
    private static final int KEY_EVENT = 1;
    private static final int MOUSE_MOVE_EVENT = 2;
    private static final int MOUSE_CLICK_EVENT = 3;
    private static final int MOUSE_SCROLL_EVENT = 4;
    private LinearLayout mView;

    byte[] sendDataBuffer = new byte[UAccessory.WRITE_PACKET_SIZE];
    private ByteBuffer bb;

    public KeyboardMouseMode(AppSettings settings,UAccessory mDev, Context context) {
        super(settings,mDev,context);
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
        bb.asIntBuffer().put(0,MOUSE_CLICK_EVENT);
        device.sendData(sendDataBuffer,sendDataBuffer.length);
    }

    private void onDrag(float dx, float dy){
        Log.d("KeyBoardMouseMode","Drag " + dx + " " + dy);
        int dxInt = (int) (dx*settings.getMouseDrag()*.02f);
        int dyInt = (int) (dy*settings.getMouseDrag()*.02f);
        bb.asIntBuffer().put(0, MOUSE_MOVE_EVENT);
        bb.asIntBuffer().put(1,dxInt);
        bb.asIntBuffer().put(2,dyInt);
        device.sendData(sendDataBuffer,sendDataBuffer.length);
    }

    private void onScroll(float ds){
        Log.d("KeyBoardMouseMode","Scrolling " + ds);
        int dsInt = (int) (ds * settings.getMouseScroll() *.02f);
        bb.asIntBuffer().put(MOUSE_SCROLL_EVENT);
        bb.asIntBuffer().put(1,dsInt);
        device.sendData(sendDataBuffer,sendDataBuffer.length);
    }


    private class TouchEventHolder{
        boolean isDown = false;
        int mIndex;
        float xPos;
        float yPos;
        float dx;
        float dy;
    }

    private TouchEventHolder[] multiTouchEvents = { new TouchEventHolder(), new TouchEventHolder() };
    private float minDisplace = 25;
    private boolean isDrag = false;
    private long clickThreashHold = 2500;

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        Log.d("KeyboardMouseMode","TouchEvent");
        if (event.getPointerCount() == 1){
            if(event.getAction() == MotionEvent.ACTION_DOWN){
                TouchEventHolder eEvent = getEmptyEvent();
                eEvent.isDown = true;
                eEvent.mIndex = event.getActionIndex();
                eEvent.xPos = event.getX();
                eEvent.yPos = event.getY();
            }
            else if(event.getAction() == MotionEvent.ACTION_UP){
                TouchEventHolder eEvent = getEventByIndex(event.getActionIndex());
                float dx = event.getX() - eEvent.xPos;
                float dy = event.getY() - eEvent.yPos;
                eEvent.xPos = event.getX();
                eEvent.yPos = event.getY();
                eEvent.dx = dx;
                eEvent.dy = dy;
                if(dx*dx + dy*dy < minDisplace){
                    Log.d("KeyboardMouseMode","Event Down time: " + event.getEventTime());
                    if(event.getDownTime() > clickThreashHold && !isDrag){
                        onClick();
                    }
                }
                eEvent.isDown = false;
                isDrag = false;
                return false;
            }else if(event.getAction() == MotionEvent.ACTION_MOVE){
                TouchEventHolder eEvent = getEventByIndex(event.getActionIndex());
                float dx = event.getX() - eEvent.xPos;
                float dy = event.getY() - eEvent.yPos;
                if(dx*dx + dy*dy < minDisplace){ //event didnt move enouf to be considered a real drag
                    return true;
                }
                eEvent.xPos = event.getX();
                eEvent.yPos = event.getY();
                eEvent.dx = dx;
                eEvent.dy = dy;
                isDrag = true;
                onDrag(dx,dy);
                return true;
            }
        }else{
            TouchEventHolder eEvent;
            switch (event.getActionMasked()){
                case MotionEvent.ACTION_DOWN:
                    eEvent = getEmptyEvent();
                    eEvent.isDown = true;
                    eEvent.mIndex = event.getActionIndex();
                    eEvent.xPos = event.getX();
                    eEvent.yPos = event.getY();
                    eEvent.dx = 0;
                    eEvent.dy = 0;
                    break;
                case MotionEvent.ACTION_MOVE:
                    eEvent = getEventByIndex(event.getActionIndex());
                    float dx = event.getX() - eEvent.xPos;
                    float dy = event.getY() - eEvent.yPos;
                    eEvent.dx = dx;
                    eEvent.dy = dy;
                    eEvent.xPos = event.getX();
                    eEvent.yPos = event.getY();
                    TouchEventHolder other = getOtherEvent(event.getActionIndex());
                    float otherDx = other.dx;
                    float otherDy = other.dy;
                    onScroll( (dy + otherDy)/2 );
                    break;
                case MotionEvent.ACTION_UP:
                    eEvent = getEventByIndex(event.getActionIndex());
                    eEvent.isDown = false;
                    break;
            }

        }
        return true;
    }

    private TouchEventHolder getEmptyEvent(){
        if(multiTouchEvents[0].isDown) return multiTouchEvents[1];
        return multiTouchEvents[0];
    }

    private TouchEventHolder getEventByIndex(int index){
        if(multiTouchEvents[0].mIndex == index) return multiTouchEvents[0];
        if(multiTouchEvents[1].mIndex == index) return multiTouchEvents[1];
        return null;
    }

    private TouchEventHolder getOtherEvent(int index){
        if(multiTouchEvents[0].mIndex == index) return multiTouchEvents[1];
        return multiTouchEvents[0];

    }
}
