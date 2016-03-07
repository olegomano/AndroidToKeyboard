package calibrationapp.spectoccular.com.keyboardtolinux.Views;

import calibrationapp.spectoccular.com.keyboardtolinux.USBMode.USBMode;

/**
 * Created by Oleg Tolstov on 3:23 PM, 3/4/16. KeyboardToLinux
 */
public class ListElement {
    public ListElement(String t, USBMode m,int ti){
        title = t;
        mode = m;
        type= ti;
    }
    public String title;
    public USBMode mode;
    public int type;
}
