package calibrationapp.spectoccular.com.keyboardtolinux;

import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.ParcelFileDescriptor;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.logging.SimpleFormatter;

/**
 * Created by Oleg Tolstov on 8:46 PM, 1/19/16. KeyboardToLinux
 */
public class UAccessory {
    public interface UAccessoryStatusListener {
        public void onDataRead(byte[] data);
        public void onIOStarted();
        public void onIOClosed();
    }

    private int packetSize = 256;
    private boolean sameEndianess = false;
    private volatile FileInputStream inputStream;
    private volatile FileOutputStream outputStream;
    private volatile UAccessoryStatusListener listener;
    private ParcelFileDescriptor pfd;
    private volatile boolean isOpen = false;

    private volatile boolean isIO = true;

    private UsbAccessory accessory;
    private UsbManager manager;


    public boolean open(Context context, Intent intent){
        accessory = (UsbAccessory)intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
        manager = (UsbManager)context.getSystemService(Context.USB_SERVICE);
        if(accessory == null) return false;
        pfd = manager.openAccessory(accessory);
        if(pfd == null) return false;
        inputStream = new FileInputStream(pfd.getFileDescriptor());
        outputStream = new FileOutputStream(pfd.getFileDescriptor());
        return true;
    }

    public int getPacketSize(){
        return packetSize;
    }

    /**
     * Starts two IO threads, one for reading and one for writing
     */
    public void startIO(int pSize){
        isIO = true;
        packetSize = pSize;
        new Thread(new ReadRunnable(),"Accessory Read Thread").start();
    }

    /**
     * Kills both IO threads
     */
    public void endIO() throws IOException {
        isIO = false;
        isOpen = false;
        if(inputStream!=null) inputStream.close();
        if(outputStream!=null) outputStream.close();
        if(listener!=null) listener.onIOClosed();
    }

    public boolean isOpen(){
        return isOpen;
    }

    private boolean handshake(){
        byte[] buffer = new byte[32];
        byte[] readBuffer = new byte[1024];
        ByteBuffer byteBuffer = ByteBuffer.wrap(buffer);
        byteBuffer.asIntBuffer().put(0,1);
        byteBuffer.asIntBuffer().put(1, packetSize);
        sendData(buffer, buffer.length);
        try {
            MainActivity.DEBUG_VIEW.printConsole("Waiting for Responce");
            inputStream.read(readBuffer);
        } catch (Exception e) {
            e.printStackTrace();
            MainActivity.DEBUG_VIEW.printConsole(e.toString());
            return false;
        }
        int endianessCheck = ByteBuffer.wrap(readBuffer).asIntBuffer().get(0);
        if(endianessCheck == 1){
            sameEndianess = true;
        }else{
            sameEndianess = false;
        }
        MainActivity.DEBUG_VIEW.printConsole("Successfull handshake, Endianess is " + sameEndianess);
        return true;
    }

    /**
     * Thread safe call
     * Puts the data into a que of pending data transfers
     * @param data data to be send
     * @param length length of data that needs to be sent
     */
    public synchronized void sendData(byte[] data, int length){
        try {
            if(outputStream!=null) {
                outputStream.write(data);
                outputStream.flush();
            }
        } catch (IOException e) {
            e.printStackTrace();
            MainActivity.DEBUG_VIEW.printConsole(e.toString());
        }
    }


    public void setDataReadListener(UAccessoryStatusListener listener){
        this.listener = listener;
    }

    private class ReadRunnable implements Runnable{
        @Override
        public void run() {
            MainActivity.DEBUG_VIEW.printConsole("Starting Read Thread");
            if(!handshake()){
                return;
            };
            if(listener!=null){
                listener.onIOStarted();
            }
            isOpen = true;
            byte[] readData = new byte[packetSize];
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
}
