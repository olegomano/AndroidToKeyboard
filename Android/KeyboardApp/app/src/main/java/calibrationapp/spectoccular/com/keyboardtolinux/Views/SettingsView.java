package calibrationapp.spectoccular.com.keyboardtolinux.Views;

import android.content.Context;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.TextView;

import calibrationapp.spectoccular.com.keyboardtolinux.AppSettings;
import calibrationapp.spectoccular.com.keyboardtolinux.R;

/**
 * Created by Oleg Tolstov on 9:20 PM, 3/9/16. KeyboardToLinux
 */
public class SettingsView implements SeekBar.OnSeekBarChangeListener, View.OnTouchListener, View.OnClickListener {
    private View root;
    private TextView scrollValue;
    private TextView dragValue;
    private TextView scaleValue;
    private Button   scaleUp;
    private Button   scaleDown;
    private SeekBar  scrollSeek;
    private SeekBar  dragSeek;
    private Context  mContext;
    private AppSettings mSettings;

    public SettingsView(Context context, AppSettings settings){
        mContext = context;
        mSettings = settings;
        root = View.inflate(context, R.layout.settings_layout,null);
        dragValue = (TextView) root.findViewById(R.id.settings_drag_value);
        scrollValue = (TextView) root.findViewById(R.id.settings_scroll_value);
        scaleValue = (TextView) root.findViewById(R.id.settings_text_scale);
        scaleDown = (Button) root.findViewById(R.id.settings_button_scale_down);
        scaleUp = (Button) root.findViewById(R.id.settings_button_scale_up);
        scaleUp.setOnClickListener(this);
        scaleDown.setOnClickListener(this);

        scrollSeek = (SeekBar) root.findViewById(R.id.settings_scroll_seekbar);
        dragSeek = (SeekBar) root.findViewById(R.id.settings_drag_seekbar);

        dragSeek.setMax(100);
        scrollSeek.setMax(100);

        scrollValue.setText(settings.getMouseScroll() + "");
        scrollSeek.setProgress(settings.getMouseScroll());
        dragValue.setText(settings.getMouseDrag() + "");
        dragSeek.setProgress(settings.getMouseDrag());

        scrollSeek.setOnSeekBarChangeListener(this);
        dragSeek.setOnSeekBarChangeListener(this);
    }

    public View getView(){
        return root;
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        int newVal = progress+1;
        if(newVal > 100) newVal = 100;
        switch  ( seekBar.getId() ){
            case R.id.settings_scroll_seekbar:
                scrollValue.setText(newVal+"");
                mSettings.setMouseScroll(newVal);
                break;
            case R.id.settings_drag_seekbar:
                dragValue.setText(newVal+"");
                mSettings.setMouseDrag(newVal);
                break;
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        return true;
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.settings_button_scale_down:
                break;
            case R.id.settings_button_scale_up:
                break;
        }
    }
}
