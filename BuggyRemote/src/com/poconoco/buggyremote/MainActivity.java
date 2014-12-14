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
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

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

        // FIXME: hardcoded UUID
        _bt.setUUID("00001101-0000-1000-8000-00805F9B34FB");

        setContentView(R.layout.activity_main);
        this.findViewById(android.R.id.content).setKeepScreenOn(true);

        _bodyMovementJoystick = (ImageView)findViewById(R.id.movementAreaImage);
        _bodyShiftPitchRollJoystick = (ImageView)findViewById(R.id.bodyShiftArea);
        _movementHint = (TextView)findViewById(R.id.movementHint);
        _buttonBluetoothToggle = (Button)findViewById(R.id.buttonBT);
        _buttonSendEngineToggle = (Button)findViewById(R.id.buttonEngine);
        _buttonSendMandibleToggle = (Button)findViewById(R.id.buttonMandiblesEngine);

        _buttonSendGait1 = (Button)findViewById(R.id.buttonGait1);
        _buttonSendGait2 = (Button)findViewById(R.id.buttonGait2);
        _buttonSendGait3 = (Button)findViewById(R.id.buttonGait3);
        _checkBoxUseSensors = (CheckBox)findViewById(R.id.checkBoxSensors);
        _checkBoxSwitchLinearMovement = (CheckBox)findViewById(R.id.checkLinear);
        _checkBoxSwitchHorizShift = (CheckBox)findViewById(R.id.checkBoxHorizShift);
        _checkBoxSwitchMandibleControl = (CheckBox)findViewById(R.id.checkBoxMandibleControl);

        _seekMandibleHeight = (SeekBar)findViewById(R.id.seekMandibleHeight);
        _seekMandibleHeight.setProgress(50);

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

        _buttonSendMandibleToggle.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(final View v)
            {
                _bt.write(BuggyProtocol.getToggleMandibleCmd());
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

        _seekMandibleHeight.setOnSeekBarChangeListener(new OnSeekBarChangeListener()
        {
            @Override
            public void onStopTrackingTouch(final SeekBar seekBar)
            {}

            @Override
            public void onStartTrackingTouch(final SeekBar seekBar)
            {}

            @Override
            public void onProgressChanged(final SeekBar seekBar,
                                          final int progress,
                                          final boolean fromUser)
            {
                _mandibles.encodeHeight(((double)progress - 50) / 50);
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
                y = y * 2 / _bodyMovementJoystick.getHeight() - 1;

                if (_checkBoxSwitchMandibleControl.isChecked())
                    _mandibles.encodeLeft(x, y);
            }
            else if (event.getAction() == MotionEvent.ACTION_UP)
            {
                x = 0;
                y = 0;
            }

            if (! _checkBoxSwitchMandibleControl.isChecked())
            {
                if (_checkBoxSwitchLinearMovement.isChecked())
                    _bodyMovement.encodeLinear(x, y);
                else
                    _bodyMovement.encodeWithTurn(x, y);
            }

            _movementHint.setText((int)(x * 100) + ":" + (int)(y * 100));
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

                x = x * 2 / _bodyShiftPitchRollJoystick.getWidth() - 1;
                y = y * 2 / _bodyShiftPitchRollJoystick.getHeight() - 1;

                if (_checkBoxSwitchMandibleControl.isChecked())
                {
                    _mandibles.encodeRight(x, y);
                }
                else
                {
                    _bodyShift.x = (byte) (- x * 127);
                    if (_checkBoxSwitchHorizShift.isChecked())
                        _bodyShift.y = (byte) (y * 127);
                    else
                        _bodyShift.z = (byte) (y * 127);
                }

                _movementHint.setText((int)(x * 100) + ":" + (int)(y * 100));
            }

            return true;
        }

    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
        _sensors.unregister();
    }

    @Override
    public void onPause()
    {
        super.onPause();
        _bt.stop();
        _sensors.unregister();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        _sensors.register();
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

                            if (_mandibles.differ(_prevMandibles))
                            {
                                _prevMandibles.copyFrom(_mandibles);
                                _bt.write(BuggyProtocol.getMandiblesCmd(_mandibles));
                            }
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
                _movementHint.setText(stateStr);
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
    private Button _buttonSendMandibleToggle;

    private Button _buttonSendGait1;
    private Button _buttonSendGait2;
    private Button _buttonSendGait3;

    private CheckBox _checkBoxUseSensors;
    private CheckBox _checkBoxSwitchLinearMovement;
    private CheckBox _checkBoxSwitchHorizShift;
    private CheckBox _checkBoxSwitchMandibleControl;

    private SeekBar _seekMandibleHeight;

    private final BuggyBodyShift _bodyShift = new BuggyBodyShift();
    private final BuggyBodyMovement _bodyMovement = new BuggyBodyMovement();
    private final BuggyMandibles _mandibles = new BuggyMandibles();

    private final BuggyBodyShift _prevBodyShift = new BuggyBodyShift();
    private final BuggyBodyMovement _prevBodyMovement = new BuggyBodyMovement();
    private final BuggyMandibles _prevMandibles = new BuggyMandibles();
}

