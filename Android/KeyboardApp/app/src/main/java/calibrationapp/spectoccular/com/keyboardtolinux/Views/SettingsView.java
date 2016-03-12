package calibrationapp.spectoccular.com.keyboardtolinux.Views;

import android.content.Context;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import calibrationapp.spectoccular.com.keyboardtolinux.AppSettings;
import calibrationapp.spectoccular.com.keyboardtolinux.R;

/**
 * Created by Oleg Tolstov on 9:20 PM, 3/9/16. KeyboardToLinux
 */
public class SettingsView implements SeekBar.OnSeekBarChangeListener {
    private float min = .1f;
    private float max = 10f;
    private View root;
    private TextView scrollValue;
    private TextView dragValue;
    private SeekBar  scrollSeek;
    private SeekBar  dragSeek;
    private Context mContext;
    private AppSettings mSettings;

    public SettingsView(Context context, AppSettings settings){
        mContext = context;
        mSettings = settings;
        root = View.inflate(context, R.layout.settings_layout,null);
        dragValue = (TextView) root.findViewById(R.id.settings_drag_value);
        scrollValue = (TextView) root.findViewById(R.id.settings_scroll_value);
        scrollSeek = (SeekBar) root.findViewById(R.id.settings_scroll_seekbar);
        dragSeek = (SeekBar) root.findViewById(R.id.settings_drag_seekbar);
        dragSeek.setMax(100);
        scrollSeek.setMax(100);

        scrollValue.setText(settings.getMouseScroll() + "");
        scrollSeek.setProgress((int) settings.getMouseScroll());
        dragValue.setText(settings.getMouseDrag() + "");
        dragSeek.setProgress((int) settings.getMouseDrag());

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
}
