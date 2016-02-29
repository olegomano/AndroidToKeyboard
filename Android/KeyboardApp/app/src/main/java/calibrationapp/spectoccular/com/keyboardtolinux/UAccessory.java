/*
	This file is part of KeyboardToLinux.

    KeyboardToLinux is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    KeyboardToLinux is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with KeyboardToLinux.  If not, see <http://www.gnu.org/licenses/>.
*/
package calibrationapp.spectoccular.com.keyboardtolinux;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by Oleg Tolstov on 8:46 PM, 1/19/16. KeyboardToLinux
 */
public class UAccessory extends BroadcastReceiver {
    private static final int TYPE_CONF = 1;
    private static final int TYPE_DATA = 2;
    private static final int TYPE_CLS = 3;

    public interface UAccessoryStatusListener {
        public void onDataRead(byte[] data);
        public void onIOStarted();
        public void onIOClosed();
    }

    private int headerSize = 4;
    private int packetSize = 256;

    private volatile byte[] packetRead;
    private volatile ByteBuffer packetReadBB;
    private volatile byte[] packetWrite;
    private volatile ByteBuffer packetWriteBB;

    private boolean sameEndianess = false;
    private volatile FileInputStream inputStream;
    private volatile FileOutputStream outputStream;
    private volatile UAccessoryStatusListener listener;
    private ParcelFileDescriptor pfd;

    private volatile boolean hasPermission = false;
    private volatile boolean isOpen = false;
    private volatile boolean isIO = false;

    private PendingIntent mPermissionIntent;
    private IntentFilter mIntentFilter;
    private UsbAccessory accessory;
    private UsbManager manager;
    private Context context;

    private static final String USB_ACTION = "usb_action_broadcast";

    public UAccessory(Context context){
        this.context = context;
        manager = (UsbManager)context.getSystemService(Context.USB_SERVICE);
        mPermissionIntent = PendingIntent.getBroadcast(context, 0, new Intent(USB_ACTION), 0);
        mIntentFilter = new IntentFilter(USB_ACTION);
        context.registerReceiver(this, mIntentFilter);
    }

    public boolean requestPermission(){
        if(manager.getAccessoryList()!=null) {
            accessory = manager.getAccessoryList()[0]; //WE ONLY GOT 1 USB PORT
            manager.requestPermission(accessory, mPermissionIntent);
            return true;
        }
        return false;
    }

    public boolean hasPermission(){
        return hasPermission;
    }

    public void cleanUp(){
        if(isIO){
            try {
                endIO();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        isOpen = false;
        hasPermission = false;
        try {
            inputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            outputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            pfd.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }


    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        MainActivity.DEBUG_VIEW.printConsole("Got intent: " + intent);
        if(action.compareTo(USB_ACTION) == 0){
            accessory = (UsbAccessory) intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
            if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                if(accessory != null){
                    hasPermission = true;
                    Log.d("UAccessory", "permission gained for accessory " + accessory);
                    MainActivity.DEBUG_VIEW.printConsole("Gained Permissions " + accessory);
                    pfd = manager.openAccessory(accessory);
                    if(pfd != null) {
                        isOpen = true;
                        inputStream = new FileInputStream(pfd.getFileDescriptor());
                        outputStream = new FileOutputStream(pfd.getFileDescriptor());
                        try {
                            startIO();
                        } catch (IOException e) {
                            e.printStackTrace();
                            MainActivity.DEBUG_VIEW.printConsole("Failed to Start IO: \n " + e.toString());
                        }
                    }else{
                        MainActivity.DEBUG_VIEW.printConsole("PFD is null");
                    }
                }
            }
            else {
                hasPermission = false;
                MainActivity.DEBUG_VIEW.printConsole("Failed to gain Permissions " + accessory);
            }

        }
    }


    public int getPacketSize(){
        return packetSize;
    }

    public void setPacketSize(int pSize){
        if(!isIO){
            return;
        }
        sendData(TYPE_CONF, null, 0);
    }

    /**
     * Starts thread for reading data
     */
    public synchronized void startIO() throws IOException {
        if(isIO) return;
        isIO = true;
        packetRead = new byte[packetSize + headerSize];
        packetRead = new byte[packetSize + headerSize];
        packetReadBB = ByteBuffer.wrap(packetRead);
        packetWrite = new byte[packetSize + headerSize];
        packetWriteBB = ByteBuffer.wrap(packetWrite);
        if(!handshake())return;
        //new Thread(new ReadRunnable(),"Accessory Read Thread").start();
    }

    /**
     * Kills read thread
     */
    public synchronized void endIO() throws IOException {
        sendData(TYPE_CLS,null,0); //tells accessory that you are ending communications`
        isIO = false;
    }


    public boolean isOpen(){
        return isOpen;
    }

    private boolean handshake() throws IOException {
        byte[] writeBuffer = new byte[32];
        byte[] readBuffer = new byte[1024];
        ByteBuffer byteBuffer = ByteBuffer.wrap(writeBuffer);
        byteBuffer.asIntBuffer().put(0, 1);
        byteBuffer.asIntBuffer().put(1, packetSize);
        outputStream.write(writeBuffer);
        outputStream.flush();

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
        sendData(TYPE_DATA,data,length);
    }


    private void sendData(int type, byte[] data, int length){
        try {
            if(outputStream!=null) {
                packetWriteBB.asIntBuffer().put(type);
                if(data!=null){
                    System.arraycopy(data,0,packetWrite,headerSize,data.length);
                }
                outputStream.write(packetWrite);
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
            if(listener!=null){
                listener.onIOStarted();
            }
            while (isIO){
                try {
                    if(inputStream.available() > 0) {
                        int readBytes = inputStream.read(packetRead);
                    }
                    listener.onDataRead(packetRead);
                } catch (IOException e) {
                    e.printStackTrace();
                    MainActivity.DEBUG_VIEW.printConsole(e.toString());
                }

            }
        }
    }
}
