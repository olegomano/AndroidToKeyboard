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
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.os.Bundle;
import android.os.Debug;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.LinearLayout;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;


public class MainActivity extends AppCompatActivity implements UAccessory.UAccessoryStatusListener {
    public static DebugView DEBUG_VIEW;
    private UAccessory usbAccessory;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        DEBUG_VIEW = new DebugView(this);
        setContentView(R.layout.activity_main);
        ((LinearLayout)findViewById(R.id.top_half) ).addView(DEBUG_VIEW.getView());
        usbAccessory = new UAccessory(this);
        usbAccessory.setPacketSize(256);
        usbAccessory.setDataReadListener(this);
        usbAccessory.requestPermission();


    }


    public void onResume(){
        super.onResume();
        showSoftInput();
        if(usbAccessory.hasPermission()){
            try {
                usbAccessory.startIO();
            } catch (IOException e) {
                e.printStackTrace();
                DEBUG_VIEW.printConsole("Failed To Start IO in onResume");
            }
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
    public boolean onKeyDown(int keyCode, KeyEvent event){
        DEBUG_VIEW.printConsole("KeyCode: " + keyCode + "Scan code" + event.getScanCode() + " Chars: " + event.getUnicodeChar());
        Log.d("Main", "KeyPressed: " + "KeyCode: " + keyCode + "Scan code" + event.getScanCode() + " Chars: " + event.getUnicodeChar());
        byte[] packet = new byte[usbAccessory.getPacketSize()];
        event.getModifiers();
        ByteBuffer.wrap(packet).asIntBuffer().put(event.getUnicodeChar());
        if(usbAccessory.isOpen()) {
            if(event.getKeyCode() != 59)
                usbAccessory.sendData(packet, packet.length);
        }
        return true;
    }

    private void showSoftInput(){
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, InputMethodManager.HIDE_IMPLICIT_ONLY);
    }



    @Override
    public void onDataRead(byte[] data) {
        DEBUG_VIEW.printConsole(new String(data));

    }

    @Override
    public void onIOStarted() {

    }

    @Override
    public void onIOClosed() {

    }
}
