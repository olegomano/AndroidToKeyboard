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
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import org.w3c.dom.Text;

import java.util.Stack;

/**
 * Created by Oleg Tolstov on 9:21 PM, 1/19/16. KeyboardToLinux
 */
public class DebugView implements Runnable{
    private View rootView;
    private ListView errorView;
    private TextView outputConsole;

    private Handler mainThreadHandler;

    private Stack<String> errorLog = new Stack<>();
    private Stack<String> consoleLog = new Stack<>();

    public DebugView(Context context){
        mainThreadHandler = new Handler(Looper.getMainLooper());
        rootView = View.inflate(context,R.layout.console_layout,null);
     //   errorView = (ListView) rootView.findViewById(R.id.debug_msgs);
        outputConsole = (TextView) rootView.findViewById(R.id.console);
    }

    public View getView(){
        return rootView;
    }

    public synchronized void printError(String error){
 //       errorLog.push(error);
        mainThreadHandler.post(this);
    }
    private volatile boolean pendingPrint = false;
    public synchronized void printConsole(String console){
        consoleLog.push(console);
        pendingPrint = true;
        mainThreadHandler.post(this);

    }

    @Override
    public synchronized void run() {
        if(!pendingPrint) return;
        if(!consoleLog.empty()) {
            outputConsole.setText(consoleLog.pop());
        }
        consoleLog.clear();
        pendingPrint = false;
    }
}
