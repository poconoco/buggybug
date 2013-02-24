package com.poconoco.buggyremote;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

import android.os.Bundle;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;
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

public class MainActivity extends Activity {

	private class BodyMovement
	{
		public byte x;
		public byte y;
		public byte turn;
		public byte speed;
		
		// joystick* are -1..1
		public void encodeLinear(double joystickX, double joystickY)
		{
			double length = Math.sqrt(joystickX * joystickX + joystickY * joystickY);
			
			if (length < 0.0001)
			{
				speed = 0;
				x = 0;
				y = 0;
				turn = 0;
				return;
			}
			
			speed = (byte) (length * 127);
			// Math is written not in optimal way, but it's easier to understand it 
			x = (byte) (joystickX * ( 1.0 / length) * 127.0);
			y = (byte) (joystickY * ( 1.0 / length) * 127.0);

			final double slowTreshold = 0.1;
			if (length < slowTreshold)
			{
				double breaking = 1 - (slowTreshold - length) / slowTreshold;

				x = (byte)((double)x * breaking);
				y = (byte)((double)y * breaking);
			}
			
			turn = 0;
		}
		
		// joystick* are -1..1
		public void encodeWithTurn(double joystickX_, double joystickY_)
		{
			double joystickX = joystickX_;
			double joystickY = joystickY_;
			
//			if (joystickY > 0)
				joystickX = 0;
			
			double length = Math.sqrt(joystickX * joystickX + joystickY * joystickY);
			
			if (length < 0.0001)
			{
				speed = 0;
				x = 0;
				y = 0;
				turn = 0;
				return;
			}
			
			speed = (byte) (length * 127);
			// Math is written not in optimal way, but it's easier to understand it 
			x = (byte) (joystickX * ( 1.0 / length) * 127.0);
			y = (byte) (joystickY * ( 1.0 / length) * 127.0);

			final double slowTreshold = 0.1;
			if (length < slowTreshold)
			{
				double breaking = 1 - (slowTreshold - length) / slowTreshold;

				x = (byte)((double)x * breaking);
				y = (byte)((double)y * breaking);
				return;
			}
			
			//if (joystickY > 0)
				turn = (byte)(joystickX_ * 127);
			//else
				//turn = 0;
		}
		
		public void copyFrom(BodyMovement src)
		{
			x = src.x;
			y = src.y;
			turn = src.turn;
			speed = src.speed;
		}
		
		public boolean differ(BodyMovement arg)
		{
			if (arg == null)
				return true;
			
			return x != arg.x
				       || y != arg.y
				       || turn != arg.turn
				       || speed != arg.speed;
			
		}
	}
	
	private class BodyShift
	{
		public byte x = 0;
		public byte y = 0;
		public byte z = 0;
		public byte pitch = 0;
		public byte roll = 0;
		public byte yaw = 0;
		
		public void copyFrom(BodyShift src)
		{
			x = src.x;
			y = src.y;
			z = src.z;
			pitch = src.pitch;
			roll = src.roll;
			yaw = src.yaw;
		}
		
		public boolean differ(BodyShift arg)
		{
			if (arg == null) 
				return true;
			
			final byte angleMargin = 0;
			final byte linearMargin = 0;
			
			return ! (withinMargin(x, arg.x, linearMargin) &&
				      withinMargin(y, arg.y, linearMargin) &&
				      withinMargin(z, arg.z, linearMargin) &&
				      withinMargin(pitch, arg.pitch, angleMargin) &&
				      withinMargin(roll, arg.roll, angleMargin) &&
				      withinMargin(yaw, arg.yaw, angleMargin));
		}
		
		private boolean withinMargin(byte a1, byte a2, byte margin)
		{
			return Math.abs(a1 - a2) <= margin;
				
		}
	};
	
    protected static final int REQUEST_ENABLE_BT = 12; // Magic number :)

    // Constants that indicate the current connection state
    public static final int STATE_NONE = 0;       // we're doing nothing
    public static final int STATE_LISTEN = 1;     // now listening for incoming connections
    public static final int STATE_CONNECTING = 2; // now initiating an outgoing connection
    public static final int STATE_CONNECTED = 3;  // now connected to a remote device

    
	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        bodyShift = new BodyShift();
        bodyMovement = new BodyMovement();
        
        movementController = (ImageView)findViewById(R.id.movementAreaImage);
        bodyShiftArea = (ImageView)findViewById(R.id.bodyShiftArea);
        movementHint = (TextView)findViewById(R.id.movementHint);
        buttonBT = (Button)findViewById(R.id.buttonBT);
        buttonEngine = (Button)findViewById(R.id.buttonEngine);
                
        buttonGait1 = (Button)findViewById(R.id.buttonGait1);
        buttonGait2 = (Button)findViewById(R.id.buttonGait2);
        checkBoxSensors = (CheckBox)findViewById(R.id.checkBoxSensors);
        checkBoxLinear = (CheckBox)findViewById(R.id.checkLinear);
        checkBoxHorizShift = (CheckBox)findViewById(R.id.checkBoxHorizShift);
        
        movementController.setTouchDelegate(new TouchDelegate(null, movementController)
        {
			@Override
        	public boolean onTouchEvent(MotionEvent event)
        	{
				//bodyShift.x = (byte)(((double)event.getX() / movementController.getWidth()) * 256 - 128);
				//bodyShift.y = (byte)(((double)event.getY() / movementController.getHeight()) * 256 - 128);
				
				float x = event.getX();
				float y = event.getY();
				
				if (event.getActionMasked() == MotionEvent.ACTION_MOVE 
						|| event.getActionMasked() == MotionEvent.ACTION_DOWN)
				{
					if (x < 0) 
						x = 0;
					if (y < 0)
						y = 0;
					
					if (x > movementController.getWidth())
						x = movementController.getWidth();
					if (y > movementController.getHeight())
						y = movementController.getHeight();
					
					x = (x * 2 / movementController.getWidth()) - 1;
					y = -((y * 2 / movementController.getHeight()) - 1);
				}
				else if (event.getAction() == MotionEvent.ACTION_UP)
				{
					x = 0;
					y = 0;
				}
				
				if (checkBoxLinear.isChecked())
					bodyMovement.encodeLinear(x,y);
				else
					bodyMovement.encodeWithTurn(x,y);
				
        		movementHint.setText(bodyMovement.x + ":" + bodyMovement.y);
        		return true;
        	}
        });
        
        bodyShiftArea.setTouchDelegate(new TouchDelegate(null, bodyShiftArea)
        {
			@Override
        	public boolean onTouchEvent(MotionEvent event)
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
					
					if (x > bodyShiftArea.getWidth())
						x = bodyShiftArea.getWidth();
					if (y > bodyShiftArea.getHeight())
						y = bodyShiftArea.getHeight();
					
					x = -((x * 2 / bodyShiftArea.getWidth()) - 1);
					y =  ((y * 2 / bodyShiftArea.getHeight()) - 1);
				
					bodyShift.x = (byte)(x * 127);
					if (checkBoxHorizShift.isChecked())
						bodyShift.y = (byte)(y * 127);
					else
						bodyShift.z = (byte)(y * 127);
	
	        		movementHint.setText(bodyShift.x + ":" + bodyShift.y + ":" + bodyShift.z);
				}
        		return true;
        	}
        });
        
        buttonEngine.setOnClickListener(new OnClickListener() 
        {
			@Override
			public void onClick(View v) 
			{
				write(" ");
			}
		});
        
        buttonGait1.setOnClickListener(new OnClickListener() 
        {
			@Override
			public void onClick(View v) 
			{
				byte cmd[] = {'g', 1, 'G'}; 
				write(cmd);
			}
		});

        buttonGait2.setOnClickListener(new OnClickListener() 
        {
			@Override
			public void onClick(View v) 
			{
				byte cmd[] = {'g', 2, 'G'}; 
				write(cmd);
			}
		});
        
        buttonBT.setOnClickListener(new OnClickListener() 
        {
			@Override
			public void onClick(View v) 
			{
				mAdapter = BluetoothAdapter.getDefaultAdapter();
				if (mAdapter == null) 
				{
					Toast.makeText(getBaseContext(), "No bluetooth on device", Toast.LENGTH_SHORT).show();
					return;
				}
				
				if (!mAdapter.isEnabled()) 
				{
				    Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
				    startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
				    return;
				}
				
				if (mState == STATE_NONE)
					connect(mAdapter.getRemoteDevice("00:12:10:12:10:04"));
				
				if (mState == STATE_CONNECTED)
					stop();
			}
		});
        
        mSensorsThread = new SensorsThread();
        mSensorsThread.registerListeners();
        mSensorsThread.start();
    }
	
    @Override
    public void onDestroy() {
    	mSensorsThread.unregisterListeners();
        super.onDestroy();
    }
    

    @Override
    public void onPause() {
    	mSensorsThread.unregisterListeners();
		stop();
        super.onPause();
    }
 
    @Override
    public void onResume() {
    	mSensorsThread.registerListeners();
        super.onResume();
    }
	
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return true;
    }
    
    /**
     * Start the chat service. Specifically start AcceptThread to begin a
     * session in listening (server) mode. Called by the Activity onResume() */
    public synchronized void start() {
        Log.d(TAG, "start");

        // Cancel any thread attempting to make a connection
        if (mConnectThread != null) {
                mConnectThread.cancel();
                mConnectThread = null;
        }

        // Cancel any thread currently running a connection
        if (mConnectedThread != null) {
                mConnectedThread.cancel();
                mConnectedThread = null;
        }

        setState(STATE_NONE);
    }
    
    
    /**
     * Start the ConnectThread to initiate a connection to a remote device.
     * @param device  The BluetoothDevice to connect
     */
    public synchronized void connect(BluetoothDevice device) {
        Log.d(TAG, "connect to: " + device);

        // Cancel any thread attempting to make a connection
        if (mState == STATE_CONNECTING) {
            if (mConnectThread != null) {mConnectThread.cancel(); mConnectThread = null;}
        }

        // Cancel any thread currently running a connection
        if (mConnectedThread != null) {mConnectedThread.cancel(); mConnectedThread = null;}

        // Start the thread to connect with the given device
        mConnectThread = new ConnectThread(device);
        mConnectThread.start();
        setState(STATE_CONNECTING);
    }

    /**
     * Start the ConnectedThread to begin managing a Bluetooth connection
     * @param socket  The BluetoothSocket on which the connection was made
     * @param device  The BluetoothDevice that has been connected
     */
    public synchronized void connected(BluetoothSocket socket, BluetoothDevice device) {
        Log.d(TAG, "connected");

        // Cancel the thread that completed the connection
        if (mConnectThread != null) {
                mConnectThread.cancel();
                mConnectThread = null;
        }

        // Cancel any thread currently running a connection
        if (mConnectedThread != null) {
                mConnectedThread.cancel();
                mConnectedThread = null;
        }

        // Start the thread to manage the connection and perform transmissions
        mConnectedThread = new ConnectedThread(socket);
        mConnectedThread.start();

        /*
        // Send the name of the connected device back to the UI Activity
        Message msg = mHandler.obtainMessage(BlueTerm.MESSAGE_DEVICE_NAME);
        Bundle bundle = new Bundle();
        bundle.putString(BlueTerm.DEVICE_NAME, device.getName());
        msg.setData(bundle);
        mHandler.sendMessage(msg);
        */

        setState(STATE_CONNECTED);
    }

    
    /**
     * Stop all threads
     */
    public synchronized void stop() {
        Log.d(TAG, "stop");

        if (mConnectThread != null) {
                mConnectThread.cancel();
                mConnectThread = null;
        }

        if (mConnectedThread != null) {
                mConnectedThread.cancel();
                mConnectedThread = null;
        }

        setState(STATE_NONE);
    }

    public void write(String str)
    {
    	write(str.getBytes());
    }
    
    /**
     * Write to the ConnectedThread in an unsynchronized manner
     * @param out The bytes to write
     * @see ConnectedThread#write(byte[])
     */
    public void write(byte[] out) {
        // Create temporary object
        ConnectedThread r;
        // Synchronize a copy of the ConnectedThread
        synchronized (this) {
            if (mState != STATE_CONNECTED) return;
            r = mConnectedThread;
            r.write(out);
        }
        // Perform the write unsynchronized
        //r.write(out);
    }
   
    /**
     * Indicate that the connection attempt failed and notify the UI Activity.
     */
    private void connectionFailed() {
        setState(STATE_NONE);

        /*
        // Send a failure message back to the Activity
        Message msg = mHandler.obtainMessage(BlueTerm.MESSAGE_TOAST);
        Bundle bundle = new Bundle();
        bundle.putString(BlueTerm.TOAST, "Unable to connect device");
        msg.setData(bundle);
        mHandler.sendMessage(msg);
        */
    }

    /**
     * Indicate that the connection was lost and notify the UI Activity.
     */
    private void connectionLost() {
        setState(STATE_NONE);

        /*
        // Send a failure message back to the Activity
        Message msg = mHandler.obtainMessage(BlueTerm.MESSAGE_TOAST);
        Bundle bundle = new Bundle();
        bundle.putString(BlueTerm.TOAST, "Device connection was lost");
        msg.setData(bundle);
        mHandler.sendMessage(msg);
        */
    }
   
    /**
     * This thread runs while attempting to make an outgoing connection
     * with a device. It runs straight through; the connection either
     * succeeds or fails.
     */
    private class ConnectThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final BluetoothDevice mmDevice;

        public ConnectThread(BluetoothDevice device) {
            mmDevice = device;
            BluetoothSocket tmp = null;

            // Get a BluetoothSocket for a connection with the
            // given BluetoothDevice
            try {
                tmp = device.createRfcommSocketToServiceRecord(SerialPortServiceClass_UUID);
            } catch (IOException e) {
                Log.e(TAG, "create() failed", e);
            }
            mmSocket = tmp;
        }

        public void run() {
            Log.i(TAG, "BEGIN mConnectThread");
            setName("ConnectThread");

            // Always cancel discovery because it will slow down a connection
            mAdapter.cancelDiscovery();

            // Make a connection to the BluetoothSocket
            try {
                // This is a blocking call and will only return on a
                // successful connection or an exception
                mmSocket.connect();
            } catch (IOException e) {
                connectionFailed();
                // Close the socket
                try {
                    mmSocket.close();
                } catch (IOException e2) {
                    Log.e(TAG, "unable to close() socket during connection failure", e2);
                }
                // Start the service over to restart listening mode
                //BluetoothSerialService.this.start();
                return;
            }

            // Reset the ConnectThread because we're done
            synchronized (MainActivity.this) 
            {
                mConnectThread = null;
            }

            // Start the connected thread
            connected(mmSocket, mmDevice);
        }

        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "close() of connect socket failed", e);
            }
        }
    }
    
    /**
     * This thread runs during a connection with a remote device.
     * It handles all incoming and outgoing transmissions.
     */
    private class ConnectedThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;
       

        public ConnectedThread(BluetoothSocket socket) {
            Log.d(TAG, "create ConnectedThread");
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the BluetoothSocket input and output streams
            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
                Log.e(TAG, "temp sockets not created", e);
            }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run() 
        {
            Log.i(TAG, "BEGIN mConnectedThread");
            final byte[] buffer = new byte[1024];

            // Keep listening to the InputStream while connected
            while (true) {
                try {
                    // Read from the InputStream
                    @SuppressWarnings("unused")
					final int bytes = mmInStream.read (buffer);
                    
/*
                    runOnUiThread(new Runnable(){
                        @Override
                        public void run()
                        {
                        	//textBTReceived.setText(new String(buffer, 0, bytes));
                        }
                    });
  */                  
                    // Send the obtained bytes to the UI Activity
                    //mHandler.obtainMessage(BlueTerm.MESSAGE_READ, bytes, -1, buffer).sendToTarget();
                    try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
                } catch (IOException e) {
                    Log.e(TAG, "disconnected", e);
                    connectionLost();
                    break;
                }
            }
        }

        /**
         * Write to the connected OutStream.
         * @param buffer  The bytes to write
         */
        public void write(byte[] buffer) {
            try {
                mmOutStream.write(buffer);

                /*
                // Share the sent message back to the UI Activity
                mHandler.obtainMessage(BlueTerm.MESSAGE_WRITE, buffer.length, -1, buffer)
                        .sendToTarget();
                        */
            } catch (IOException e) {
                Log.e(TAG, "Exception during write", e);
            }
        }

        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "close() of connect socket failed", e);
            }
        }
    }    
    
    /**
     * Set the current state of the chat connection
     * @param state  An integer defining the current connection state
     */
    private synchronized void setState(final int state) {
        //Log.d(TAG, "setState() " + stateToString(mState) + " -> " + stateToString(state));
        mState = state;

        /*
        // Give the new state to the Handler so the UI Activity can update
        mHandler.obtainMessage(BlueTerm.MESSAGE_STATE_CHANGE, state, -1).sendToTarget();
        */
        runOnUiThread(new Runnable() {
            public void run() {
            	Toast.makeText(getBaseContext(), stateToString(state), Toast.LENGTH_SHORT).show();
            }
        });
    }

    private String stateToString(int state) 
    {
    	switch (state)
    	{
    		case STATE_NONE: return "Sate: none";
    		case STATE_LISTEN: return "State: listening";
    		case STATE_CONNECTING: return "State: connecting";
    		case STATE_CONNECTED: return "State: connected";
    	}

		return "Unknown";
	}

	/**
     * Return the current connection state. */
    public synchronized int getState() {
        return mState;
    }
    
    private class SensorsThread 
    	extends Thread
    	implements SensorEventListener
    {
	    public void registerListeners() 
	    {
	        m_sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
	        
	        m_sensorManager.registerListener(this, m_sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD), SensorManager.SENSOR_DELAY_GAME);
	        m_sensorManager.registerListener(this, m_sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_GAME);
	    }
	    
	    private void unregisterListeners() {
	        m_sensorManager.unregisterListener(this);
	    }
    	
        public void run() 
        {
        	while (true)
        	{
        		if (mState == STATE_CONNECTED)
        		{
	        		runOnUiThread(new Runnable() 
	        		{
						@Override
						public void run() 
						{
							if (checkBoxSensors.isChecked() && bodyShift.differ(previouslySentBodyShift))
							{
								previouslySentBodyShift.copyFrom(bodyShift);

								byte[] cmd = new byte[8];
								
								cmd[0] = 'b';
								
								cmd[1] = bodyShift.x;
								cmd[2] = bodyShift.y;
								cmd[3] = bodyShift.z;
								
								cmd[4] = bodyShift.pitch;
								cmd[5] = bodyShift.roll;
								cmd[6] = bodyShift.yaw;

								cmd[7] = 'B';
								
								write(cmd);
								//movementHint.setText("yaw: " + bodyShift.yaw + " pitch: " + bodyShift.pitch + " roll: " + bodyShift.roll);
							}
							
							//if (bodyMovement.differ(previousSentBodyMovement))
							{
								previousSentBodyMovement.copyFrom(bodyMovement);
								
								byte[] cmd = new byte[6];
								
								cmd[0] = 'm';
								cmd[1] = bodyMovement.x;
								cmd[2] = bodyMovement.y;
								cmd[3] = bodyMovement.turn;
								cmd[4] = bodyMovement.speed;
								cmd[5] = 'M';
								
								write(cmd);
							}
						}
					});
        		}

        		
        		try 
        		{
					sleep(100);
				} catch (InterruptedException e) 
				{
					e.printStackTrace();
				}

        	}
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }
     
        @Override
        public void onSensorChanged(SensorEvent event) {
            if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
                accel(event);
            }
            if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
                mag(event);
            }
        }
     
        private void accel(SensorEvent event) {
            if (m_lastAccels == null) {
                m_lastAccels = new float[3];
            }
     
            System.arraycopy(event.values, 0, m_lastAccels, 0, 3);
     
            /*if (m_lastMagFields != null) {
                computeOrientation();
            }*/
        }
     
        private void mag(SensorEvent event) {
            if (m_lastMagFields == null) {
                m_lastMagFields = new float[3];
            }
     
            System.arraycopy(event.values, 0, m_lastMagFields, 0, 3);
     
            if (m_lastAccels != null) {
                computeOrientation();
            }
        }        
        
        private void computeOrientation() 
        {
        	if (SensorManager.getRotationMatrix(
        			m_rotationMatrix, 
        			null,
                    m_lastAccels, 
                    m_lastMagFields)) 
        	{
        		SensorManager.getOrientation(m_rotationMatrix, m_orientation);

                /* 1 radian = 57.2957795 degrees */
                /* [0] : yaw, rotation around z axis
                 * [1] : pitch, rotation around x axis
                 * [2] : roll, rotation around y axis */
                float yaw = m_orientation[0] * 57.2957795f;
                float pitch = m_orientation[1] * 57.2957795f;
                float roll = m_orientation[2] * 57.2957795f;

                /* append returns an average of the last 10 values */
/*
                m_lastYaw = m_filters[0].append(yaw);
                m_lastPitch = m_filters[1].append(pitch);
                m_lastRoll = m_filters[2].append(roll);
*/
                m_lastYaw = yaw;
                m_lastPitch = pitch;
                m_lastRoll = roll;
                
                bodyShift.pitch = (byte)((limitTo90(m_lastRoll) / 90) * 127);
                bodyShift.roll = (byte)((limitTo90(-m_lastPitch) / 90) * 127);
            }
        } 	
        
        private float limitTo90(float angle)
        {
        	if (angle < -90)
        		return -90;
        	if (angle > 90)
        		return 90;
        	
        	return angle;
        }
        /*
        private class Filter {
            static final int AVERAGE_BUFFER = 10;
            float []m_arr = new float[AVERAGE_BUFFER];
            int m_idx = 0;
     
            public float append(float val) {
                m_arr[m_idx] = val;
                m_idx++;
                if (m_idx == AVERAGE_BUFFER)
                    m_idx = 0;
                return avg();
            }
            public float avg() {
                float sum = 0;
                for (float x: m_arr)
                    sum += x;
                return sum / AVERAGE_BUFFER;
            }
     
        }*/
        
        SensorManager m_sensorManager;
		final BodyShift previouslySentBodyShift = new BodyShift();
		final BodyMovement previousSentBodyMovement = new BodyMovement();
        
//        private Filter [] m_filters = { new Filter(), new Filter(), new Filter() };

        private float[] m_lastMagFields;
        private float[] m_lastAccels;
        private float[] m_rotationMatrix = new float[16];
  //      private float[] m_remappedR = new float[16];
        private float[] m_orientation = new float[4];
        
        /* fix random noise by averaging tilt values */
//        final static int AVERAGE_BUFFER = 30;
//        float []m_prevPitch = new float[AVERAGE_BUFFER];
        float m_lastPitch = 0.f;
        @SuppressWarnings("unused")
		float m_lastYaw = 0.f;
        /* current index int m_prevEasts */
//        int m_pitchIndex = 0;
     
//        float []m_prevRoll = new float[AVERAGE_BUFFER];
        float m_lastRoll = 0.f;
        /* current index into m_prevTilts */
  //      int m_rollIndex = 0;
     
        /* center of the rotation */
 //       private float m_tiltCentreX = 0.f;
 //       private float m_tiltCentreY = 0.f;
 //       private float m_tiltCentreZ = 0.f;        
    };
    
    private ImageView movementController;
    private ImageView bodyShiftArea; 
    private TextView movementHint;
    private Button buttonBT;

    private Button buttonEngine;
    
    private Button buttonGait1;
    private Button buttonGait2;
    private CheckBox checkBoxSensors;
    private CheckBox checkBoxLinear;
    private CheckBox checkBoxHorizShift;
    

    private BluetoothAdapter mAdapter;
    private int mState;
//    private final Handler mHandler;
    private ConnectThread mConnectThread;
    private ConnectedThread mConnectedThread;
    
    private SensorsThread mSensorsThread;
    
    private BodyShift bodyShift;
    private BodyMovement bodyMovement;
    
    
    private static final UUID SerialPortServiceClass_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private static final String TAG = "BluetoothReadService";
    
}
