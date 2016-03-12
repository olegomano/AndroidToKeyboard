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
public class DrawerView extends ArrayAdapter<ListElement> implements AdapterView.OnItemClickListener {
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
    public DrawerView(Context context, ListElement[] elements){
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
            convertView = new LinearLayout(getContext());
            ( (LinearLayout)convertView ).setOrientation(LinearLayout.HORIZONTAL);
            convertView.setPadding(MainActivity.dpToPixel(25),0,0,0);

            ImageView icon = new ImageView(getContext());
            icon.setPadding(MainActivity.dpToPixel(2),MainActivity.dpToPixel(2),MainActivity.dpToPixel(2),MainActivity.dpToPixel(2));
            icon.setTag(1);
            LinearLayout.LayoutParams iconLayoutParams = new LinearLayout.LayoutParams(
                    0,
                    LinearLayout.LayoutParams.MATCH_PARENT,.1f);

            View textView = new TextView(getContext());
            textView.setTag(2);
            textView.setMinimumHeight(MainActivity.dpToPixel(52));
            ((TextView) textView).setTextSize(18);
            ( (TextView)textView ).setTextColor(Color.BLACK);
            ( (TextView)textView ).setPadding(MainActivity.dpToPixel(19), 0, 0, 0);
            ( (TextView)textView ).setGravity(Gravity.CENTER_VERTICAL);
            LinearLayout.LayoutParams textLayoutParams = new LinearLayout.LayoutParams(
                    0,
                    LinearLayout.LayoutParams.MATCH_PARENT,.9f);


            ( (LinearLayout)convertView ).addView(icon,iconLayoutParams);
            ( (LinearLayout)convertView ).addView(textView,textLayoutParams);

        }
        LinearLayout asLayout = ( (LinearLayout)convertView );
        for(int i = 0; i < asLayout.getChildCount(); i++){
            if( (int) (asLayout.getChildAt(i).getTag() ) == 1){
                ((ImageView) asLayout.getChildAt(i)).setImageDrawable(getItem(position).drawable);
            }else if((int) (asLayout.getChildAt(i).getTag() ) == 2){
                ((TextView) asLayout.getChildAt(i)).setText(getItem(position).title);
            }
        }

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
