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

import android.content.Context;
import android.inputmethodservice.Keyboard;
import android.os.Bundle;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
import android.view.KeyEvent;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import calibrationapp.spectoccular.com.keyboardtolinux.USBMode.KeyboardMouseMode;
import calibrationapp.spectoccular.com.keyboardtolinux.Views.ListElement;

import java.io.IOException;

import calibrationapp.spectoccular.com.keyboardtolinux.Views.Drawer;
import calibrationapp.spectoccular.com.keyboardtolinux.Views.ListElement;
import calibrationapp.spectoccular.com.keyboardtolinux.Views.UIMessageInterface;


public class MainActivity extends AppCompatActivity implements UAccessory.UAccessoryStatusListener, UIMessageInterface {
    public static float DP_TO_PIX_RATIO;
    public static DebugView DEBUG_VIEW;
    private DrawerLayout root;
    private FrameLayout content;
    private Drawer drawer;
    private UAccessory usbAccessory;
    private ListElement[] elements;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        DP_TO_PIX_RATIO = getResources().getDisplayMetrics().density;;
        DEBUG_VIEW = new DebugView(this);

        usbAccessory = new UAccessory(this);
        usbAccessory.setPacketSize(100000000
        );
        usbAccessory.setDataReadListener(this);

        elements = new ListElement[]{
                new ListElement("Keyboard and Mouse", new KeyboardMouseMode(usbAccessory, this),UIMessageInterface.SHOW_HID),
                new ListElement("Number Pad", null,UIMessageInterface.SHOW_NUMPAD),
                new ListElement("Second Display", null,UIMessageInterface.SHOW_SECOND_DISPLAY),
                new ListElement("Settings", null,UIMessageInterface.SHOW_STATUS),
                new ListElement("About", null, UIMessageInterface.SHOW_ABOUT)
        };
        drawer = new Drawer(this,elements);
        drawer.setMessageHandler(this);
        setContentView(R.layout.drawer_layout);
        root = (DrawerLayout) findViewById(R.id.drawer_layout);
        content = (FrameLayout) findViewById(R.id.content_frame);
        ((LinearLayout)findViewById(R.id.left_drawer)).addView(drawer.getView());

    }



    public void onResume(){
        super.onResume();
        if(usbAccessory.hasPermission()){
            try {
                DEBUG_VIEW.printConsole("Already Have Permissions, Starting IO");
                usbAccessory.startIO();
            } catch (IOException e) {
                e.printStackTrace();
                DEBUG_VIEW.printConsole("Failed To Start IO in onResume");
                drawer.setConnectionStatus(Drawer.CONNECTION_STATUS_DISCONNECTED);
            }
        }else{
            DEBUG_VIEW.printConsole("Do not Have permissions, Requesting");
            drawer.setConnectionStatus(Drawer.CONNECTION_STATUS_DISCONNECTED);
            usbAccessory.requestPermission();
        }
    }


    @Override
    public void sendMessage(int msg) {
        switch (msg){
            case UIMessageInterface.SHOW_HID:
                getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
                elements[0].mode.onSelected();
                content.removeAllViews();
                content.addView(elements[0].mode.getView());
                root.closeDrawers();
                break;
        }
    }


    @Override
    public void onPause(){
        try {
            usbAccessory.endIO();
        } catch (IOException e) {
            e.printStackTrace();
        }
        super.onPause();
    }

    @Override
    public void onDestroy(){
        usbAccessory.cleanUp();
        super.onDestroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        ((KeyboardMouseMode)elements[0].mode).keyPressed(keyCode,event);
        return true;
    }

    @Override
    public void onDataRead(byte[] data) {
        DEBUG_VIEW.printConsole(new String(data));

    }

    @Override
    public void onIOStarted() {
        DEBUG_VIEW.printConsole("onIOStarted Called");
        drawer.setConnectionStatus(Drawer.CONNECTION_STATUS_CONNECTED);
    }

    @Override
    public void onIOClosed() {

    }

    public static int dpToPixel(int dp){
        return (int) (dp*DP_TO_PIX_RATIO + .5f);
    }
}
