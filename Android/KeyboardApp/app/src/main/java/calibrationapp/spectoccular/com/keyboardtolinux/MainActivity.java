package calibrationapp.spectoccular.com.keyboardtolinux;

import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity {
    private TextView accessoryName;
    private UsbAccessory accessory;
    private OutputStream toAccessory;
    private InputStream  fromAccessory;
    private boolean threadRunning = true;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.assessory_layout);
        accessoryName = (TextView) findViewById(R.id.accessory_name);
        UsbManager usbManager = (UsbManager) getSystemService(USB_SERVICE);
        UsbAccessory[] mComputer = usbManager.getAccessoryList();
        if(mComputer == null){
            Log.d("DEBUG","NO ACCESSORIES FOUND");
            return;
        }
        for(int i = 0; i < mComputer.length; i++){
            Log.d("DEBUG","FOUND ACCESSORY: " + mComputer[i].toString());
        }
        accessoryName.setText(mComputer[0].toString());
        accessory = mComputer[0];
        ParcelFileDescriptor mFileDescriptor = usbManager.openAccessory(accessory);
        toAccessory = new FileOutputStream(mFileDescriptor.getFileDescriptor());
        fromAccessory = new FileInputStream(mFileDescriptor.getFileDescriptor());
        accessoryName.setText(toAccessory.toString());
        ThreadRunable runable = new ThreadRunable();
        runable.send = toAccessory;
        runable.get = fromAccessory;
        new Thread(runable,"Acessory Send Thread").start();

    }

    private class ThreadRunable implements Runnable{
        OutputStream send;
        InputStream get;
        @Override
        public void run() {
            while(threadRunning){
                byte[] read = new byte[1024];
                if(get == null){
                    threadRunning = false;
                    System.exit(1);
                }
                try {
                    get.read(read);
                } catch (IOException e) {
                    threadRunning = false;
                    System.exit(1);
                }
            }

        }
    }

    @Override
    public void onStop(){
        super.onStop();
        threadRunning = false;
    }
}
