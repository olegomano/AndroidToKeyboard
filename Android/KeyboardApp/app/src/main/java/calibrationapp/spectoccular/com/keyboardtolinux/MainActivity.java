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

import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import calibrationapp.spectoccular.com.keyboardtolinux.USBMode.KeyboardMouseMode;
import calibrationapp.spectoccular.com.keyboardtolinux.USBMode.USBMode;
import calibrationapp.spectoccular.com.keyboardtolinux.Views.DebugView;
import calibrationapp.spectoccular.com.keyboardtolinux.Views.ListElement;

import java.io.IOException;

import calibrationapp.spectoccular.com.keyboardtolinux.Views.DrawerView;
import calibrationapp.spectoccular.com.keyboardtolinux.Views.SettingsView;
import calibrationapp.spectoccular.com.keyboardtolinux.Views.UIMessageInterface;


public class MainActivity extends AppCompatActivity implements UAccessory.UAccessoryStatusListener, UIMessageInterface {
    public static float DP_TO_PIX_RATIO;
    public static DebugView DEBUG_VIEW;
    private AppSettings settings;
    private DrawerLayout root;
    private FrameLayout content;
    private DrawerView drawer;
    private UAccessory usbAccessory;
    private ListElement[] elements;

    private KeyboardMouseMode keyboardMouseMode;
    private SettingsView settingsView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        DP_TO_PIX_RATIO = getResources().getDisplayMetrics().density;;
        DEBUG_VIEW = new DebugView(this);
        usbAccessory = new UAccessory(this);
        usbAccessory.setPacketSize(100000000);
        usbAccessory.setDataReadListener(this);

        settings = new AppSettings(this);
        keyboardMouseMode = new KeyboardMouseMode(settings,usbAccessory,this);
        settingsView = new SettingsView(this, settings);

        elements = new ListElement[]{
                new ListElement("Keyboard and Mouse", UIMessageInterface.SHOW_HID, toDrawable(R.drawable.keyboard_2)),
                new ListElement("Number Pad",UIMessageInterface.SHOW_NUMPAD, toDrawable(R.drawable.numpad_2)),
                new ListElement("Second Display",UIMessageInterface.SHOW_SECOND_DISPLAY,toDrawable(R.drawable.monitor)),
                new ListElement("Settings",UIMessageInterface.SHOW_STATUS,toDrawable(R.drawable.settings))
           //     new ListElement("About",UIMessageInterface.SHOW_ABOUT, R.drawable.)
        };
        drawer = new DrawerView(this,elements);
        drawer.setMessageHandler(this);
        setContentView(R.layout.drawer_layout);
        root = (DrawerLayout) findViewById(R.id.drawer_layout);
        content = (FrameLayout) findViewById(R.id.content_frame);
        ((LinearLayout)findViewById(R.id.left_drawer)).addView(drawer.getView());

    }

    private BitmapDrawable toDrawable(int rid){
        return new BitmapDrawable(getResources(), BitmapFactory.decodeResource(getResources(), rid) ) ;
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
                drawer.setConnectionStatus(DrawerView.CONNECTION_STATUS_DISCONNECTED);
            }
        }else{
            DEBUG_VIEW.printConsole("Do not Have permissions, Requesting");
            drawer.setConnectionStatus(DrawerView.CONNECTION_STATUS_DISCONNECTED);
            usbAccessory.requestPermission();
        }
    }


    @Override
    public void sendMessage(int msg) {
        switch (msg){
            case UIMessageInterface.SHOW_HID:
                getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
                keyboardMouseMode.onSelected();
                content.removeAllViews();
                content.addView(keyboardMouseMode.getView());
                root.closeDrawers();
                break;
            case UIMessageInterface.SHOW_STATUS:
                keyboardMouseMode.onDeselected();
                content.removeAllViews();
                content.addView(settingsView.getView());
                root.closeDrawers();
        }
    }


    @Override
    public void onPause(){
        settings.save();
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
        keyboardMouseMode.keyPressed(keyCode, event);
        return true;
    }

    @Override
    public void onDataRead(byte[] data) {
        DEBUG_VIEW.printConsole(new String(data));

    }

    @Override
    public void onIOStarted() {
        DEBUG_VIEW.printConsole("onIOStarted Called");
        drawer.setConnectionStatus(DrawerView.CONNECTION_STATUS_CONNECTED);
    }

    @Override
    public void onIOClosed() {

    }

    public static int dpToPixel(int dp){
        return (int) (dp*DP_TO_PIX_RATIO + .5f);
    }
}
