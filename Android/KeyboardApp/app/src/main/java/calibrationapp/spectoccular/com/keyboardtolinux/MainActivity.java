package calibrationapp.spectoccular.com.keyboardtolinux;

import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.TextView;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;


public class MainActivity extends AppCompatActivity {
    private TextView accessoryName;
    private TextView fromDevice;
    private UsbAccessory accessory;
    private OutputStream toAccessory;
    private InputStream  fromAccessory;
    private boolean threadRunning = true;
    private Handler mainThreadHandler;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mainThreadHandler = new Handler(Looper.getMainLooper());
        setContentView(R.layout.assessory_layout);
        accessoryName = (TextView) findViewById(R.id.accessory_name);
        fromDevice = (TextView) findViewById(R.id.fromAccessory);
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
        new Thread(runable,"Acessory Send Thread").start();

    }
    private String pendingText = "";
    private class SetOutputText implements Runnable{
        @Override
        public void run() {
            fromDevice.setText(pendingText);
        }
    }

    private class ThreadRunable implements Runnable{
        @Override
        public void run() {
            while(threadRunning){
                byte[] read = new byte[1024];
                if(fromAccessory == null){
                    threadRunning = false;
                    System.exit(1);
                }
                try {
                    fromAccessory.read(read);
                    pendingText = new String(read);
                    mainThreadHandler.post(new SetOutputText());
                    Thread.sleep(300);
                } catch (IOException e) {
                    threadRunning = false;
                    System.exit(1);
                } catch (InterruptedException e) {
                    e.printStackTrace();
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
