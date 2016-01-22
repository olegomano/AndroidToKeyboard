package calibrationapp.spectoccular.com.keyboardtolinux;

import android.annotation.TargetApi;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.io.BufferedOutputStream;
import java.io.EOFException;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.IntBuffer;


public class MainActivity extends AppCompatActivity implements UAccessory.DataReadListener, KeyboardView.OnKeyboardActionListener {
    public static DebugView DEBUG_VIEW;
    private UAccessory usbAccessory;
    private KeyboardView kView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        DEBUG_VIEW = new DebugView(this);
        kView = new KeyboardView(this,null);
        kView.setKeyboard(new Keyboard(this, R.layout.qwerty_keyboard));
        kView.setOnKeyboardActionListener(this);

        setContentView(R.layout.activity_main);
        ((LinearLayout)findViewById(R.id.top_half) ).addView(DEBUG_VIEW.getView());
        ((LinearLayout)findViewById(R.id.bottom_half) ).addView(kView);
        usbAccessory = new UAccessory();
        if(!usbAccessory.open(this,getIntent())){
            DEBUG_VIEW.printConsole("Failed to Open Device");
            return;
        }
        usbAccessory.setDataReadListener(this);
        usbAccessory.startIO();
        DEBUG_VIEW.printConsole("Start");
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
    public void onPress(int primaryCode) {
        DEBUG_VIEW.printConsole("KeyCode: " + primaryCode);
        Log.d("Main","Pressed Key: " + primaryCode);
        byte[] intAsByte = new byte[4];
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
