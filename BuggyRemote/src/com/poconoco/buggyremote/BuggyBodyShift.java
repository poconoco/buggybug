package com.poconoco.buggyremote;

public class BuggyBodyShift
{
    public byte x = 0;
    public byte y = 0;
    public byte z = 0;
    public byte pitch = 0;
    public byte roll = 0;
    public byte yaw = 0;

    public void copyFrom(final BuggyBodyShift src)
    {
        x = src.x;
        y = src.y;
        z = src.z;
        pitch = src.pitch;
        roll = src.roll;
        yaw = src.yaw;
    }

    public boolean differ(final BuggyBodyShift arg)
    {
        if (arg == null)
            return true;

        final byte angleMargin = 0;
        final byte linearMargin = 0;

        return !(withinMargin(x, arg.x, linearMargin)
            && withinMargin(y, arg.y, linearMargin)
            && withinMargin(z, arg.z, linearMargin)
            && withinMargin(pitch, arg.pitch, angleMargin)
            && withinMargin(roll, arg.roll, angleMargin)
            && withinMargin(yaw, arg.yaw, angleMargin));
    }

    private boolean withinMargin(final byte a1, final byte a2, final byte margin)
    {
        return Math.abs(a1 - a2) <= margin;
    }
}
