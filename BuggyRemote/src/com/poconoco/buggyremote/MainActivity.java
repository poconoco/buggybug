package com.poconoco.buggyremote;

import android.app.Activity;
import android.graphics.Rect;
import android.os.Bundle;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.TouchDelegate;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity
    implements BuggySensors.Listener, BuggyBluetooth.Listener
{

    @Override
    protected void onCreate(final Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        _bt = new BuggyBluetooth(this, this);
        _sensors = new BuggySensors(this, this);

        _sensorsThread = new StateUpdateThread();
        _sensorsThread.start();

        // Here bluetooth MAC address of arduino BT module hardcoded,
        // so this app is tied to a single robot model
        _bt.setUUID("00001101-0000-1000-8000-00805F9B34FB");

        setContentView(R.layout.activity_main);

        _bodyShift = new BuggyBodyShift();
        _bodyMovement = new BuggyBodyMovement();

        _bodyMovementJoystick = (ImageView) findViewById(R.id.movementAreaImage);
        _bodyShiftPitchRollJoystick = (ImageView) findViewById(R.id.bodyShiftArea);
        _movementHint = (TextView) findViewById(R.id.movementHint);
        _buttonBluetoothToggle = (Button) findViewById(R.id.buttonBT);
        _buttonSendEngineToggle = (Button) findViewById(R.id.buttonEngine);

        _buttonSendGait1 = (Button) findViewById(R.id.buttonGait1);
        _buttonSendGait2 = (Button) findViewById(R.id.buttonGait2);
        _buttonSendGait3 = (Button) findViewById(R.id.buttonGait3);
        _checkBoxUseSensors = (CheckBox) findViewById(R.id.checkBoxSensors);
        _checkBoxSwitchLinearMovement = (CheckBox) findViewById(R.id.checkLinear);
        _checkBoxSwitchHorizShift = (CheckBox) findViewById(R.id.checkBoxHorizShift);

        _bodyMovementJoystick.setTouchDelegate(
            new MovementJoystickTouchDelegate(null, _bodyMovementJoystick));

        _bodyShiftPitchRollJoystick.setTouchDelegate(
            new ShiftPithRollTouchDelegate(null, _bodyShiftPitchRollJoystick));

        _buttonSendEngineToggle.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(final View v)
            {
                _bt.write(BuggyProtocol.getToggleServoCmd());
            }
        });

        _buttonSendGait1.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(final View v)
            {
                _bt.write(BuggyProtocol.getGaitCmd(1));
            }
        });

        _buttonSendGait2.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(final View v)
            {
                _bt.write(BuggyProtocol.getGaitCmd(2));
            }
        });

        _buttonSendGait3.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(final View v)
            {
                _bt.write(BuggyProtocol.getGaitCmd(3));
            }
        });

        _buttonBluetoothToggle.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(final View v)
            {
                _bt.toggle();
            }
        });

    }

    class MovementJoystickTouchDelegate extends TouchDelegate
    {
        public MovementJoystickTouchDelegate(final Rect bounds, final View delegateView)
        {
            super(bounds, delegateView);
        }

        @Override
        public boolean onTouchEvent(final MotionEvent event)
        {
            float x = event.getX();
            float y = event.getY();

            if (event.getActionMasked() == MotionEvent.ACTION_MOVE
                    || event.getActionMasked() == MotionEvent.ACTION_DOWN)
            {
                if (x < 0)
                    x = 0;
                if (y < 0)
                    y = 0;

                if (x > _bodyMovementJoystick.getWidth())
                    x = _bodyMovementJoystick.getWidth();
                if (y > _bodyMovementJoystick.getHeight())
                    y = _bodyMovementJoystick.getHeight();

                x = x * 2 / _bodyMovementJoystick.getWidth() - 1;
                y = -(y * 2 / _bodyMovementJoystick.getHeight() - 1);
            } else if (event.getAction() == MotionEvent.ACTION_UP)
            {
                x = 0;
                y = 0;
            }

            if (_checkBoxSwitchLinearMovement.isChecked())
                _bodyMovement.encodeLinear(x, y);
            else
                _bodyMovement.encodeWithTurn(x, y);

            _movementHint.setText(_bodyMovement.x + ":" + _bodyMovement.y);
            return true;
        }
    }

    class ShiftPithRollTouchDelegate extends TouchDelegate
    {
        public ShiftPithRollTouchDelegate(final Rect bounds, final View delegateView)
        {
            super(bounds, delegateView);
        }

        @Override
        public boolean onTouchEvent(final MotionEvent event)
        {
            float x = event.getX();
            float y = event.getY();

            if (event.getActionMasked() == MotionEvent.ACTION_MOVE
                    || event.getActionMasked() == MotionEvent.ACTION_DOWN)
            {
                if (x < 0)
                    x = 0;
                if (y < 0)
                    y = 0;

                if (x > _bodyShiftPitchRollJoystick.getWidth())
                    x = _bodyShiftPitchRollJoystick.getWidth();
                if (y > _bodyShiftPitchRollJoystick.getHeight())
                    y = _bodyShiftPitchRollJoystick.getHeight();

                x = -(x * 2 / _bodyShiftPitchRollJoystick.getWidth() - 1);
                y = y * 2 / _bodyShiftPitchRollJoystick.getHeight() - 1;

                _bodyShift.x = (byte) (x * 127);
                if (_checkBoxSwitchHorizShift.isChecked())
                    _bodyShift.y = (byte) (y * 127);
                else
                    _bodyShift.z = (byte) (y * 127);

                _movementHint.setText(_bodyShift.x + ":" + _bodyShift.y + ":"
                        + _bodyShift.z);
            }
            return true;
        }

    }

    @Override
    public void onDestroy()
    {
        _sensors.unregister();
        super.onDestroy();
    }

    @Override
    public void onPause()
    {
        _sensors.unregister();
        _bt.stop();
        super.onPause();
    }

    @Override
    public void onResume()
    {
        _sensors.register();
        super.onResume();
    }

    @Override
    public boolean onCreateOptionsMenu(final Menu menu)
    {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return true;
    }


    private class StateUpdateThread extends Thread
    {
        @Override
        public void run()
        {
            while (true)
            {
                if (_connected)
                {
                    runOnUiThread(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            if (_checkBoxUseSensors.isChecked()
                                    && _bodyShift
                                    .differ(_prevBodyShift))
                            {
                                _prevBodyShift.copyFrom(_bodyShift);
                                _bt.write(BuggyProtocol.getBodyShiftCmd(_bodyShift));
                            }

                            _prevBodyMovement.copyFrom(_bodyMovement);

                            _bt.write(BuggyProtocol.getBodyMovementCmd(_bodyMovement));
                        }
                    });
                }

                try
                {
                    sleep(100);
                } catch (final InterruptedException e)
                {
                    e.printStackTrace();
                }

            }
        }
    };
    @Override
    public void onStateChanged(final BuggyBluetooth.State state, final String stateStr)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                Toast.makeText(getBaseContext(),
                               stateStr,
                               Toast.LENGTH_SHORT).show();
            }
        });

        _connected = state == BuggyBluetooth.State.STATE_CONNECTED;
    }

    @Override
    public void onPitchRollChanged(final float pitch, final float roll)
    {
        _bodyShift.pitch = (byte)(pitch / 90 * 127);
        _bodyShift.roll = (byte)(roll / 90 * 127);
    }

    private BuggyBluetooth _bt;
    private BuggySensors _sensors;
    private boolean _connected = false;

    private StateUpdateThread _sensorsThread;

    private ImageView _bodyMovementJoystick;
    private ImageView _bodyShiftPitchRollJoystick;
    private TextView _movementHint;

    private Button _buttonBluetoothToggle;
    private Button _buttonSendEngineToggle;

    private Button _buttonSendGait1;
    private Button _buttonSendGait2;
    private Button _buttonSendGait3;

    private CheckBox _checkBoxUseSensors;
    private CheckBox _checkBoxSwitchLinearMovement;
    private CheckBox _checkBoxSwitchHorizShift;

    private BuggyBodyShift _bodyShift;
    private BuggyBodyMovement _bodyMovement;
    final BuggyBodyShift _prevBodyShift = new BuggyBodyShift();
    final BuggyBodyMovement _prevBodyMovement = new BuggyBodyMovement();
}

