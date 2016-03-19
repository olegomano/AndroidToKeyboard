package calibrationapp.spectoccular.com.keyboardtolinux.USBMode;

import android.content.Context;
import android.graphics.Bitmap;
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
    private ImageView imageView;
    private byte[] mPacket = new byte[UAccessory.WRITE_PACKET_SIZE_FREE];
    private ByteBuffer bb;

    public SSmode(AppSettings apps, UAccessory mDev, Context context) {
        super(apps, mDev, context);
        imageView = new ImageView(context);
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
        int flag;
        IntBuffer buffer = ByteBuffer.wrap(data).asIntBuffer();
        if(device.getEndianess()){
            flag = buffer.get(0);
        }else{
            flag = Integer.reverseBytes( buffer.get(0) );
        }
        MainActivity.DEBUG_VIEW.printConsole("SSMode got data" + flag);
        switch (flag){
            case 0:
                int fbWidth;
                int fbHeight;
                if(device.getEndianess()){
                    fbWidth = buffer.get(1);
                    fbHeight = buffer.get(2);
                }else{
                    fbWidth = Integer.reverseBytes(buffer.get(1));
                    fbHeight = Integer.reverseBytes(buffer.get(2));
                }
                MainActivity.DEBUG_VIEW.printConsole("FrameBuffer W: " + fbWidth + ", H: " +fbHeight);
                break;
        }
    }

    public void onSelected(){
        isCurrent = true;
        /*
        int userScreenW = settings.getFBW();
        int userScreenH = settings.getFBH();
        bb.asIntBuffer().put(CNTRL_RES_REQUEST);
        bb.asIntBuffer().put(1,userScreenW);
        bb.asIntBuffer().put(2, userScreenH);
        //device.sendCntrlMsg(mPacket);
        */
    }

    public void onDeselected(){
        /*
        isCurrent = false;
        bb.asIntBuffer().put(CNTRL_CNTRL_STP_SS);
        //device.sendCntrlMsg(mPacket);
        */
    }
}
