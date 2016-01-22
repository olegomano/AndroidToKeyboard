package calibrationapp.spectoccular.com.keyboardtolinux;

import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Stack;

/**
 * Created by Oleg Tolstov on 8:46 PM, 1/19/16. KeyboardToLinux
 */
public class UAccessory {

    public interface DataReadListener{
        public void onDataRead(byte[] data);
    }

    private volatile FileInputStream inputStream;
    private volatile FileOutputStream outputStream;
    private volatile DataReadListener listener;

    private volatile boolean isIO = true;

    private UsbAccessory accessory;
    private UsbManager manager;

    private ArrayList<byte[]> dataBuffer1 = new ArrayList<>();
    private ArrayList<byte[]> dataBuffer2 = new ArrayList<>();

    private int writeBuffer = 0;

    public boolean open(Context context, Intent intent){
        accessory = (UsbAccessory)intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
        manager = (UsbManager)context.getSystemService(Context.USB_SERVICE);
        if(accessory == null) return false;
        ParcelFileDescriptor fd = manager.openAccessory(accessory);
        if(fd == null) return false;
        inputStream = new FileInputStream(fd.getFileDescriptor());
        outputStream = new FileOutputStream(fd.getFileDescriptor());
        return true;
    }

    /**
     * Starts two IO threads, one for reading and one for writing
     */
    public void startIO(){
        isIO = true;
        new Thread(new ReadRunnable(),"Accessory Read Thread").start();
        new Thread(new WriteRunnable(),"Accessory Write Thread").start();
    }

    /**
     * Kills both IO threads
     */
    public void endIO() throws IOException {
        isIO = false;
        if(inputStream!=null) inputStream.close();
        if(outputStream!=null) outputStream.close();
    }

    /**
     * Thread safe call
     * Puts the data into a que of pending data transfers
     * @param data data to be send
     * @param length length of data that needs to be sent
     */
    public synchronized void sendData(byte[] data, int length){
        try {
            String toSend = "";
            for(int i = 0; i < data.length; i++){
                toSend+=data[i];
            }
            String sentBytes = "";
            byte[] stringBytes = toSend.getBytes();
            for(int i = 0; i < stringBytes.length;i++){
                sentBytes+=stringBytes[i]+",";
            }
            MainActivity.DEBUG_VIEW.printConsole(sentBytes);
            Log.d("UAccessory", sentBytes);
            Log.d("UAccessory",toSend);
            if(outputStream!=null) {
                outputStream.write(data);
                outputStream.flush();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        /*
        switch (writeBuffer){
            case 0: dataBuffer1.add(data); break;
            case 1: dataBuffer2.add(data); break;
        }
        */

    }

    private synchronized void swapBuffers(){
        writeBuffer++;
        writeBuffer = writeBuffer%2;
        switch (writeBuffer){
            case 0: dataBuffer1.clear();break;
            case 1: dataBuffer2.clear();break;
        }
    }

    public void setDataReadListener(DataReadListener listener){
        this.listener = listener;
    }

    private class ReadRunnable implements Runnable{
        @Override
        public void run() {
            byte[] readData = new byte[1024];
            MainActivity.DEBUG_VIEW.printConsole("Starting Read Thread");
            while (isIO){
                try {
                    int readBytes = inputStream.read(readData);
                    listener.onDataRead(readData);
                } catch (IOException e) {
                    e.printStackTrace();
                    MainActivity.DEBUG_VIEW.printConsole(e.toString());
                }

            }
        }
    }


    private class WriteRunnable implements Runnable{
        @Override
        public void run(){
            while (isIO){
                switch (writeBuffer){
                    case 0:
                        for(int i = 0; i < dataBuffer2.size(); i++){
                            try {
                                String sentbytes = "";
                                for(int b = 0; b < dataBuffer2.get(i).length; b++){
                                    sentbytes+=dataBuffer2.get(i)[b]+" ";
                                }
                                outputStream.write(sentbytes.getBytes());
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        }
                        break;
                    case 1:
                        for(int i = 0; i < dataBuffer1.size(); i++){
                            try {
                                outputStream.write(dataBuffer1.get(i));
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        }
                        break;
                }
                swapBuffers();
            }
        }
    }
}
