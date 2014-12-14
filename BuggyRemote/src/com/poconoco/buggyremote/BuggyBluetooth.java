package com.poconoco.buggyremote;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.util.Log;

public class BuggyBluetooth
{
    enum State
    {
        STATE_NONE,
        STATE_LISTEN,
        STATE_CONNECTING,
        STATE_CONNECTED
    }

    public interface Listener
    {
        void onStateChanged(State state, String stateStr);
    }

    BuggyBluetooth(final Listener listener, final Activity activity)
    {
        _listener = listener;
        _context = activity;
    }

    void setUUID(final String uuid)
    {
        _uuid = uuid;
    }

    public void toggle()
    {
        _adapter = BluetoothAdapter.getDefaultAdapter();
        if (_adapter == null)
        {
            _listener.onStateChanged(_state, "No bluetooth on device");
            return;
        }

        if (! _adapter.isEnabled())
        {
            final Intent enableBtIntent = new Intent(
                    BluetoothAdapter.ACTION_REQUEST_ENABLE);
            _context.startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            return;
        }

        // FIXME: hardcoded MAC
        if (_state == State.STATE_NONE)
            connect(_adapter.getRemoteDevice("00:12:10:12:10:04"));

        if (_state == State.STATE_CONNECTED)
            stop();
    }

    public synchronized void connect(final BluetoothDevice device)
    {
        Log.d(LOG_TAG, "connect to: " + device);

        // Cancel any thread attempting to make a connection
        if (_state == State.STATE_CONNECTING)
        {
            if (_connectThread != null)
            {
                _connectThread.cancel();
                _connectThread = null;
            }
        }

        // Cancel any thread currently running a connection
        if (_connectedThread != null)
        {
            _connectedThread.cancel();
            _connectedThread = null;
        }

        // Start the thread to connect with the given device
        _connectThread = new ConnectThread(device);
        _connectThread.start();
        setState(State.STATE_CONNECTING);
    }

    public synchronized void connected(final BluetoothSocket socket,
            final BluetoothDevice device)
    {
        // Cancel the thread that completed the connection
        if (_connectThread != null)
        {
            _connectThread.cancel();
            _connectThread = null;
        }

        // Cancel any thread currently running a connection
        if (_connectedThread != null)
        {
            _connectedThread.cancel();
            _connectedThread = null;
        }

        // Start the thread to manage the connection and perform transmissions
        _connectedThread = new ConnectedThread(socket);
        _connectedThread.start();

        /*
         * // Send the name of the connected device back to the UI Activity
         * Message msg = mHandler.obtainMessage(BlueTerm.MESSAGE_DEVICE_NAME);
         * Bundle bundle = new Bundle(); bundle.putString(BlueTerm.DEVICE_NAME,
         * device.getName()); msg.setData(bundle); mHandler.sendMessage(msg);
         */

        setState(State.STATE_CONNECTED);
    }

    public synchronized void start()
    {
        // Cancel any thread attempting to make a connection
        if (_connectThread != null)
        {
            _connectThread.cancel();
            _connectThread = null;
        }

        // Cancel any thread currently running a connection
        if (_connectedThread != null)
        {
            _connectedThread.cancel();
            _connectedThread = null;
        }

        setState(State.STATE_NONE);
    }

    /**
     * Stop all threads
     */
    public synchronized void stop()
    {
        Log.d(LOG_TAG, "stop");

        if (_connectThread != null)
        {
            _connectThread.cancel();
            _connectThread = null;
        }

        if (_connectedThread != null)
        {
            _connectedThread.cancel();
            _connectedThread = null;
        }

        setState(State.STATE_NONE);
    }

    public synchronized State getState()
    {
        return _state;
    }

    public String stateToString(final State state)
    {
        switch (state)
        {
        case STATE_NONE:
            return "Sate: none";
        case STATE_LISTEN:
            return "State: listening";
        case STATE_CONNECTING:
            return "State: connecting";
        case STATE_CONNECTED:
            return "State: connected";
        }

        return "Unknown";
    }

    private synchronized void setState(final State state)
    {
        _state = state;

        _listener.onStateChanged(state, stateToString(state));
    }

    public void write(final byte[] out)
    {
        // Create temporary object
        ConnectedThread r;
        // Synchronize a copy of the ConnectedThread
        synchronized (this)
        {
            if (_state != State.STATE_CONNECTED)
                return;
            r = _connectedThread;
            r.write(out);
        }
    }

    public void write(final String str)
    {
        write(str.getBytes());
    }

    /**
     * This thread runs during a connection with a remote device. It handles all
     * incoming and outgoing transmissions.
     */
    private class ConnectedThread extends Thread
    {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(final BluetoothSocket socket)
        {
            Log.d(LOG_TAG, "create ConnectedThread");
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the BluetoothSocket input and output streams
            try
            {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (final IOException e)
            {
                Log.e(LOG_TAG, "temp sockets not created", e);
            }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        @Override
        public void run()
        {
            Log.i(LOG_TAG, "BEGIN mConnectedThread");
            final byte[] buffer = new byte[1024];

            // Keep listening to the InputStream while connected
            while (true)
            {
                try
                {
                    // Read from the InputStream
                    @SuppressWarnings("unused")
                    final int bytes = mmInStream.read(buffer);

                    /*
                     * runOnUiThread(new Runnable(){
                     *
                     * @Override public void run() {
                     * //textBTReceived.setText(new String(buffer, 0, bytes)); }
                     * });
                     */
                    // Send the obtained bytes to the UI Activity
                    // mHandler.obtainMessage(BlueTerm.MESSAGE_READ, bytes, -1,
                    // buffer).sendToTarget();
                    try
                    {
                        Thread.sleep(500);
                    } catch (final InterruptedException e)
                    {
                        e.printStackTrace();
                    }
                } catch (final IOException e)
                {
                    Log.e(LOG_TAG, "disconnected", e);
                    connectionLost();
                    break;
                }
            }
        }

        /**
         * Write to the connected OutStream.
         *
         * @param buffer
         *            The bytes to write
         */
        public void write(final byte[] buffer)
        {
            try
            {
                mmOutStream.write(buffer);

                /*
                 * // Share the sent message back to the UI Activity
                 * mHandler.obtainMessage(BlueTerm.MESSAGE_WRITE, buffer.length,
                 * -1, buffer) .sendToTarget();
                 */
            } catch (final IOException e)
            {
                Log.e(LOG_TAG, "Exception during write", e);
            }
        }

        public void cancel()
        {
            try
            {
                mmSocket.close();
            } catch (final IOException e)
            {
                Log.e(LOG_TAG, "close() of connect socket failed", e);
            }
        }
    }

    /**
     * This thread runs while attempting to make an outgoing connection with a
     * device. It runs straight through; the connection either succeeds or
     * fails.
     */
    private class ConnectThread extends Thread
    {
        private final BluetoothSocket mmSocket;
        private final BluetoothDevice mmDevice;

        public ConnectThread(final BluetoothDevice device)
        {
            mmDevice = device;
            BluetoothSocket tmp = null;

            // Get a BluetoothSocket for a connection with the
            // given BluetoothDevice
            try
            {
                tmp = device.createRfcommSocketToServiceRecord(UUID.fromString(_uuid));
            } catch (final IOException e)
            {
                Log.e(LOG_TAG, "create() failed", e);
            }
            mmSocket = tmp;
        }

        @Override
        public void run()
        {
            Log.i(LOG_TAG, "BEGIN mConnectThread");
            setName("ConnectThread");

            // Always cancel discovery because it will slow down a connection
            _adapter.cancelDiscovery();

            // Make a connection to the BluetoothSocket
            try
            {
                // This is a blocking call and will only return on a
                // successful connection or an exception
                mmSocket.connect();
            } catch (final IOException e)
            {
                connectionFailed();
                // Close the socket
                try
                {
                    mmSocket.close();
                } catch (final IOException e2)
                {
                    Log.e(LOG_TAG,
                            "unable to close() socket during connection failure",
                            e2);
                }
                // Start the service over to restart listening mode
                // BluetoothSerialService.this.start();
                return;
            }

            // Reset the ConnectThread because we're done
            synchronized (_context)
            {
                _connectThread = null;
            }

            // Start the connected thread
            connected(mmSocket, mmDevice);
        }

        public void cancel()
        {
            try
            {
                mmSocket.close();
            } catch (final IOException e)
            {
                Log.e(LOG_TAG, "close() of connect socket failed", e);
            }
        }
    }

    private void connectionLost()
    {
        setState(State.STATE_NONE);
    }

    private void connectionFailed()
    {
        setState(State.STATE_NONE);
    }

    private final Listener _listener;
    private final Activity _context;

    private String _uuid;
    private State _state = State.STATE_NONE;

    private ConnectThread _connectThread;
    private ConnectedThread _connectedThread;
    private BluetoothAdapter _adapter;

    protected static final int REQUEST_ENABLE_BT = 12; // Magic number :)
    private static final String LOG_TAG = "BluetoothReadService";
}
