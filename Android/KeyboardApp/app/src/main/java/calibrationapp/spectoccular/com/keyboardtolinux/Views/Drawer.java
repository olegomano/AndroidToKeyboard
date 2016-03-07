package calibrationapp.spectoccular.com.keyboardtolinux.Views;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import calibrationapp.spectoccular.com.keyboardtolinux.MainActivity;
import calibrationapp.spectoccular.com.keyboardtolinux.R;
import calibrationapp.spectoccular.com.keyboardtolinux.USBMode.USBMode;

/**
 * Created by Oleg Tolstov on 12:23 PM, 3/2/16. KeyboardToLinux
 */
public class Drawer extends ArrayAdapter<ListElement> implements AdapterView.OnItemClickListener {
    public static final int CONNECTION_STATUS_CONNECTED = 1;
    public static final int CONNECTION_STATUS_DISCONNECTED = 2;
    private Context mContext;

    private BitmapDrawable connectedIcon;
    private BitmapDrawable disconnectedIcon;

    private View root;

    private ImageView statusIcon;
    private LinearLayout titleBarParent;
    private TextView  statusName;
    private ListView  menuOptions;
    private UIMessageInterface messageHandler;


    /*
    public Drawer(View v){

    }
    */
    public Drawer(Context context, ListElement[] elements){
        super(context, 0);
        addAll(elements);
        mContext = context;
        root = View.inflate(context, R.layout.drawer_menu_layout,null);
        statusIcon = (ImageView) root.findViewById(R.id.drawer_connection_status_icon);
        titleBarParent = (LinearLayout) root.findViewById(R.id.drawer_status_bar_parent);
        statusName = (TextView) root.findViewById(R.id.drawer_connection_status_text);
        menuOptions = (ListView) root.findViewById(R.id.drawer_list_view);
        menuOptions.setOnItemClickListener(this);
        menuOptions.setAdapter(this);
        loadIcons();

    }

    public void setMessageHandler(UIMessageInterface intrfc){
        messageHandler = intrfc;
    }

    public void setConnectionStatus(int status){
        switch (status){
            case CONNECTION_STATUS_CONNECTED:
                statusName.setText("Connected");
                titleBarParent.setBackgroundColor(Color.CYAN);
                statusIcon.setImageDrawable(connectedIcon);
                break;
            case CONNECTION_STATUS_DISCONNECTED:
                statusName.setText("Not Connected");
                titleBarParent.setBackgroundColor(Color.RED);
                statusIcon.setImageDrawable(disconnectedIcon);
                break;
        }
        titleBarParent.postInvalidate();
    }

    public View getView(){
        return root;
    }

    public View getView(int position, View convertView, ViewGroup parent){
        if(convertView == null){
            convertView = new TextView(getContext());
            convertView.setMinimumHeight(MainActivity.dpToPixel(52));
            ( (TextView)convertView ).setTextSize(18);
            ( (TextView)convertView ).setTextColor(Color.BLACK);
            ( (TextView)convertView ).setPadding(MainActivity.dpToPixel(19), 0, 0, 0);
            ( (TextView)convertView ).setGravity(Gravity.CENTER_VERTICAL);
        }
        ( (TextView)convertView ).setText(getItem(position).title);
        return convertView;
    }

    private void loadIcons(){
        connectedIcon = new BitmapDrawable(mContext.getResources(), BitmapFactory.decodeResource(mContext.getResources(),R.drawable.usb_connected) );
        disconnectedIcon = new BitmapDrawable( mContext.getResources(), BitmapFactory.decodeResource(mContext.getResources(),R.drawable.usb_disconnected));
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        messageHandler.sendMessage(getItem(position).type);
    }
}
