package calibrationapp.spectoccular.com.keyboardtolinux.Views;

import android.graphics.drawable.BitmapDrawable;
import android.view.View;

import calibrationapp.spectoccular.com.keyboardtolinux.USBMode.USBMode;

/**
 * Created by Oleg Tolstov on 3:23 PM, 3/4/16. KeyboardToLinux
 */
public class ListElement {
    public ListElement(String t, int ti, BitmapDrawable bm){
        title = t;
        type= ti;
        drawable = bm;
    }
    public String title;
    public int type;
    public BitmapDrawable drawable;
}
