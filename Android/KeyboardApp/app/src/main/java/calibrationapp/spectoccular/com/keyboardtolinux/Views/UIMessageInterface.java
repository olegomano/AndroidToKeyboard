package calibrationapp.spectoccular.com.keyboardtolinux.Views;

/**
 * Created by Oleg Tolstov on 9:40 PM, 3/2/16. KeyboardToLinux
 */
public interface UIMessageInterface{
    public static final int SHOW_HID = 1;
    public static final int SHOW_SECOND_DISPLAY = 2;
    public static final int SHOW_STATUS = 3;
    public static final int SHOW_ABOUT = 4;
    public static final int SHOW_NUMPAD = 5;
    public void sendMessage(int msg);
}

