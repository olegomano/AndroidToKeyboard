package calibrationapp.spectoccular.com.keyboardtolinux.USBMode;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;

import calibrationapp.spectoccular.com.keyboardtolinux.AppSettings;
import calibrationapp.spectoccular.com.keyboardtolinux.MainActivity;
import calibrationapp.spectoccular.com.keyboardtolinux.UAccessory;
import calibrationapp.spectoccular.com.keyboardtolinux.Views.DebugView;

/**
 * Created by Oleg Tolstov on 5:06 PM, 3/12/16. KeyboardToLinux
 */
public class SSmode extends USBMode {
    private static final int REQUEST_FB_DIMS = 1;
    private static final int REQUEST_SS_START = 2;
    private static final int REQUEST_SS_END = 3;
    private static final int SS_DATA = 4;
    private static final int SS_SYN = 5;

    private int totalPackets;
    private ImageView imageView;
    private Bitmap screenBitmap;
    private byte[] screenBitmapBytes;
    private int[]  screenBitmapColors;
    private byte[] mPacket = new byte[UAccessory.WRITE_PACKET_SIZE];
    private ByteBuffer bb;

    public SSmode(AppSettings apps, UAccessory mDev, Context context) {
        super(apps, mDev, context);
        imageView = new ImageView(context);
        imageView.getImageMatrix().postRotate(90);
        bb = ByteBuffer.wrap(mPacket);
    }

    @Override
    public View getView() {
        return imageView;
    }

    @Override
    public byte flag() {
        return USBMode.SS_MODE;
    }

    public void onDataReceived(byte[] data){
        byte flag = data[1];
        IntBuffer buffer = ByteBuffer.wrap(data).asIntBuffer();
        /*
        MainActivity.DEBUG_VIEW.printConsole("SSMode got data \n" + Integer.toHexString( data[0] )
                                                                  + " " + Integer.toHexString( data[1] )
                                                                  + " " + Integer.toHexString( data[2] )
                                                                  + " " + Integer.toHexString( data[3] )
                                                                  + " " + Integer.toHexString( buffer.get(1) )
                                                                  + " " + Integer.toHexString( buffer.get(2) ) );
        */
        switch (flag){
            case REQUEST_FB_DIMS:
                int fbWidth = 0;
                int fbHeight = 0;
                int packetCount = 0;
                if(device.getEndianess()){
                    fbWidth = buffer.get(1);
                    fbHeight = buffer.get(2);
                    packetCount = buffer.get(3);
                }else{
                    fbWidth = Integer.reverseBytes(buffer.get(1));
                    fbHeight = Integer.reverseBytes(buffer.get(2));
                    packetCount = Integer.reverseBytes(buffer.get(3));
                }
                screenBitmapBytes = new byte[fbWidth*fbHeight*4];
                screenBitmapColors = new int[fbWidth*fbHeight];
                screenBitmap = Bitmap.createBitmap(fbWidth,fbHeight, Bitmap.Config.ARGB_8888);
                totalPackets = packetCount;
                MainActivity.DEBUG_VIEW.printConsole("FrameBuffer W: " + fbWidth + ", H: " +fbHeight + "P Count: " + totalPackets);

                mPacket[1] = flag();
                bb.asIntBuffer().put(1,REQUEST_SS_START);
                device.sendData(mPacket);
                break;
            case SS_DATA:
                int packetNumber;
                if(device.getEndianess()){
                    packetNumber = buffer.get(1);
                }else{
                    packetNumber = Integer.reverseBytes(buffer.get(1));
                }
                int dataStart = 8;
                int dataSize = device.getPacketSize() - dataStart;
                int bitmapStart = dataSize * packetNumber;
                //Log.d("SSMode","Packet " + packetNumber);
                //MainActivity.DEBUG_VIEW.printConsole("Recieved Frame: " + packetNumber + " " + totalPackets);
                if(packetNumber < 0 || packetNumber > totalPackets) return;
                if(packetNumber < totalPackets - 1){
                    System.arraycopy(data,dataStart, screenBitmapBytes,bitmapStart,dataSize);
                }else if(packetNumber == totalPackets -1 ){
                    int remainingData = screenBitmapBytes.length - bitmapStart;
                    System.arraycopy(data, dataStart, screenBitmapBytes, bitmapStart, remainingData);
                }
                break;
            case SS_SYN:
                new Handler(mContext.getMainLooper()).post(new SetImageRunnable());
                break;
        }
    }

    private class SetImageRunnable implements Runnable{
        @Override
        public void run() {
            ByteBuffer screenBitmapBuffer = ByteBuffer.wrap(screenBitmapBytes);
            screenBitmapBuffer.asIntBuffer().get(screenBitmapColors);
            screenBitmap.setPixels(screenBitmapColors, 0, screenBitmap.getWidth(), 0, 0, screenBitmap.getWidth(), screenBitmap.getHeight());
            Drawable drawable = new BitmapDrawable(mContext.getResources(), screenBitmap );
            imageView.setImageDrawable(drawable);
            MainActivity.DEBUG_VIEW.printConsole("Got Full Screen Grab");
        }
    }

    public void onSelected(){
        isCurrent = true;
        int userScreenW = settings.getFBW();
        int userScreenH = settings.getFBH();
        mPacket[1] = flag();
        bb.asIntBuffer().put(1,REQUEST_FB_DIMS);
        bb.asIntBuffer().put(2,userScreenW);
        bb.asIntBuffer().put(3, userScreenH);
        bb.asIntBuffer().put(4,settings.getDownScale());
        device.sendData(mPacket);
        //device.sendCntrlMsg(mPacket);

    }

    public void onDeselected(){
        mPacket[1] = flag();
        bb.asIntBuffer().put(1,REQUEST_SS_END);
        device.sendData(mPacket);
        /*
        isCurrent = false;
        bb.asIntBuffer().put(CNTRL_CNTRL_STP_SS);
        //device.sendCntrlMsg(mPacket);
        */
    }
}
