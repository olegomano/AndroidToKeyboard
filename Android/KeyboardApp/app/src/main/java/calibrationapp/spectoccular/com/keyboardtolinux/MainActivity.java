package calibrationapp.spectoccular.com.keyboardtolinux;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.os.Bundle;
import android.os.Debug;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.LinearLayout;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;


public class MainActivity extends AppCompatActivity implements UAccessory.UAccessoryStatusListener, KeyboardView.OnKeyboardActionListener {
    public static DebugView DEBUG_VIEW;
    private UAccessory usbAccessory;
    private KeyboardView kView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        DEBUG_VIEW = new DebugView(this);
        kView = new KeyboardView(this,null);
        kView.setKeyboard(new Keyboard(this, R.layout.my_keyboard));
        kView.setOnKeyboardActionListener(this);
        setContentView(R.layout.activity_main);
        ((LinearLayout)findViewById(R.id.top_half) ).addView(DEBUG_VIEW.getView());
        showSoftInput();
        usbAccessory = new UAccessory();
        if(!usbAccessory.open(this,getIntent())){
            DEBUG_VIEW.printConsole("Failed to Open Device");
            return;
        }

        usbAccessory.setDataReadListener(this);
        usbAccessory.startIO(256);
        DEBUG_VIEW.printConsole("Start");

    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event){
        DEBUG_VIEW.printConsole("KeyCode: " + keyCode + "Scan code" + event.getScanCode() + " Chars: " + event.getUnicodeChar());
        Log.d("Main","KeyPressed: " + "KeyCode: " + keyCode + "Scan code" + event.getScanCode() + " Chars: " + event.getUnicodeChar());
        byte[] packet = new byte[usbAccessory.getPacketSize()];
        event.getModifiers();
        ByteBuffer.wrap(packet).asIntBuffer().put(event.getUnicodeChar());
        if(usbAccessory.isOpen()) {
            if(event.getKeyCode() != 59)
                usbAccessory.sendData(packet, packet.length);
        }
        return true;
    }

    private void showSoftInput(){
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.toggleSoftInput(InputMethodManager.SHOW_FORCED,InputMethodManager.HIDE_IMPLICIT_ONLY);
    }


    @Override
    public void onStop(){
        super.onStop();
        try {
            usbAccessory.endIO();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onDataRead(byte[] data) {
        DEBUG_VIEW.printConsole(new String(data));

    }

    @Override
    public void onIOStarted() {

    }

    @Override
    public void onIOClosed() {

    }

    @Override
    public void onPress(int primaryCode) {
        DEBUG_VIEW.printConsole("KeyCode: " + primaryCode);
        Log.d("Main","Pressed Key: " + primaryCode);
        byte[] intAsByte = new byte[usbAccessory.getPacketSize()];
        ByteBuffer bBuffer = ByteBuffer.wrap(intAsByte);
        IntBuffer iBuffer = bBuffer.asIntBuffer();
        iBuffer.put(primaryCode);
        for(int i = 0; i < intAsByte.length; i++){
            Log.d("Main",intAsByte[i] + "");
        }
        usbAccessory.sendData(intAsByte,intAsByte.length);
    }

    @Override
    public void onRelease(int primaryCode) {

    }

    @Override
    public void onKey(int primaryCode, int[] keyCodes) {

    }

    @Override
    public void onText(CharSequence text) {

    }

    @Override
    public void swipeLeft() {

    }

    @Override
    public void swipeRight() {

    }

    @Override
    public void swipeDown() {

    }

    @Override
    public void swipeUp() {

    }
}
