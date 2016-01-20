package calibrationapp.spectoccular.com.keyboardtolinux;

import android.annotation.TargetApi;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
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


public class MainActivity extends AppCompatActivity implements UAccessory.DataReadListener {
    public static DebugView DEBUG_VIEW;
    private UAccessory usbAccessory;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        DEBUG_VIEW = new DebugView(this);
        setContentView(DEBUG_VIEW.getView());
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
        usbAccessory.sendData(data,data.length);
    }
}
