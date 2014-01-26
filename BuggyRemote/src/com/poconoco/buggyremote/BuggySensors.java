package com.poconoco.buggyremote;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

public class BuggySensors
    implements SensorEventListener
{
    public interface Listener
    {
        void onPitchRollChanged(float pitch, float roll);
    }

    BuggySensors(final Context context, final Listener listener)
    {
        _listener = listener;
        _context = context;

        register();
    }

    public void unregister()
    {
        _sensorManager.unregisterListener(this);
    }

    public void register()
    {
        _sensorManager = (SensorManager) _context.getSystemService(Context.SENSOR_SERVICE);

        _sensorManager.registerListener(this, _sensorManager
                .getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD),
                SensorManager.SENSOR_DELAY_GAME);
        _sensorManager
        .registerListener(this, _sensorManager
                .getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                SensorManager.SENSOR_DELAY_GAME);
    }

    @Override
    public void onAccuracyChanged(final Sensor sensor, final int accuracy)
    {
    }

    @Override
    public void onSensorChanged(final SensorEvent event)
    {
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER)
        {
            accel(event);
        }
        if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD)
        {
            mag(event);
        }
    }

    private void accel(final SensorEvent event)
    {
        if (_lastAccels == null)
        {
            _lastAccels = new float[3];
        }

        System.arraycopy(event.values, 0, _lastAccels, 0, 3);

        /*
         * if (m_lastMagFields != null) { computeOrientation(); }
         */
    }

    private void mag(final SensorEvent event)
    {
        if (_lastMagFields == null)
        {
            _lastMagFields = new float[3];
        }

        System.arraycopy(event.values, 0, _lastMagFields, 0, 3);

        if (_lastAccels != null)
        {
            computeOrientation();
        }
    }

    private void computeOrientation()
    {
        if (SensorManager.getRotationMatrix(_rotationMatrix, null,
                _lastAccels, _lastMagFields))
        {
            SensorManager.getOrientation(_rotationMatrix, _orientation);

            /* 1 radian = 57.2957795 degrees */
            /*
             * [0] : yaw, rotation around z axis [1] : pitch, rotation
             * around x axis [2] : roll, rotation around y axis
             */
            final float yaw = _orientation[0] * 57.2957795f;
            final float pitch = _orientation[1] * 57.2957795f;
            final float roll = _orientation[2] * 57.2957795f;

            /* append returns an average of the last 10 values */
            /*
             * m_lastYaw = m_filters[0].append(yaw); m_lastPitch =
             * m_filters[1].append(pitch); m_lastRoll =
             * m_filters[2].append(roll);
             */
            _lastYaw = yaw;
            _lastPitch = pitch;
            _lastRoll = roll;

            _listener.onPitchRollChanged(limitTo90(_lastRoll), limitTo90(-_lastPitch));
        }
    }

    private float limitTo90(final float angle)
    {
        if (angle < -90)
            return -90;
        if (angle > 90)
            return 90;

        return angle;
    }

    private final Listener _listener;
    private final Context _context;

    SensorManager _sensorManager;

    private float[] _lastMagFields;
    private float[] _lastAccels;
    private final float[] _rotationMatrix = new float[16];
    private final float[] _orientation = new float[4];

    float _lastPitch = 0.f;
    float _lastYaw = 0.f;
    float _lastRoll = 0.f;
}
