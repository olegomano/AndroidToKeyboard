package calibrationapp.spectoccular.com.keyboardtolinux.USBMode;

import android.content.Context;
import android.view.View;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;

import calibrationapp.spectoccular.com.keyboardtolinux.AppSettings;
import calibrationapp.spectoccular.com.keyboardtolinux.UAccessory;

/**
 * Created by Oleg Tolstov on 10:17 PM, 3/2/16. KeyboardToLinux
 */
public abstract class USBMode {
    public static final byte KBM_MODE = 1;
    public static final byte SS_MODE = 2;

    protected UAccessory device;
    protected boolean isCurrent = false;
    protected Context mContext;
    protected AppSettings settings;
    public USBMode(AppSettings apps, UAccessory mDev, Context context){
        device = mDev;
        mContext = context;
        settings = apps;
    }

    public void onSelected(){
        isCurrent = true;
    }

    public void onDeselected(){
        isCurrent = false;
    }

    public void onDataReceived(byte[] data){

    }

    public abstract View getView();
    public abstract byte flag();

}
