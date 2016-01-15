package calibrationapp.spectoccular.com.keyboardtolinux;

import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    private TextView accessoryName;
    private UsbAccessory accessory;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.assessory_layout);
        accessoryName = (TextView) findViewById(R.id.accessory_name);
        UsbManager usbManager = (UsbManager) getSystemService(USB_SERVICE);
        UsbAccessory[] mComputer = usbManager.getAccessoryList();
        if(mComputer == null){
            Log.d("DEBUG","NO ACCESSORIES FOUND");
            return;
        }
        for(int i = 0; i < mComputer.length; i++){
            Log.d("DEBUG","FOUND ACCESSORY: " + mComputer[i].toString());
        }
        accessoryName.setText(mComputer[0].toString());
        accessory = mComputer[0];
        
    }
}
