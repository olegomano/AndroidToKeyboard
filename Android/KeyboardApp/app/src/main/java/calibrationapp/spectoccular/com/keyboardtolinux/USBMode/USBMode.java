package calibrationapp.spectoccular.com.keyboardtolinux.USBMode;

import android.content.Context;
import android.view.View;

import calibrationapp.spectoccular.com.keyboardtolinux.UAccessory;

/**
 * Created by Oleg Tolstov on 10:17 PM, 3/2/16. KeyboardToLinux
 */
public abstract class USBMode {
    protected UAccessory device;
    protected boolean isCurrent = false;
    protected Context mContext;
    public USBMode(UAccessory mDev, Context context){
        device = mDev;
        mContext = context;
    }

    public void onSelected(){
        isCurrent = true;
    }

    public void onDeselected(){
        isCurrent = false;
    }

    public abstract View getView();
}
