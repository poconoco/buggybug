package com.poconoco.buggyremote;

public class BuggyMandibles
{
    public byte rx = 0;
    public byte ry = 0;
    public byte rz = 0;

    public byte lx = 0;
    public byte ly = 0;
    public byte lz = 0;

    public void copyFrom(final BuggyMandibles src)
    {
        rx = src.rx;
        ry = src.ry;
        rz = src.rz;
        lx = src.lx;
        ly = src.ly;
        lz = src.lz;
    }

    // all args are -1..1
    public void encodeRight(final double joystickRX,
                            final double joystickRY)
    {
        rx = (byte) (joystickRX * 127.0);
        rz = (byte) (joystickRY * 127.0);
    }

    // all args are -1..1
    public void encodeLeft(final double joystickLX,
                           final double joystickLY)
    {
        lx = (byte) (joystickLX * 127.0);
        lz = (byte) (joystickLY * 127.0);
    }

    // all args are -1..1
    public void encodeHeight(final double height)
    {
        ry = (byte) (height * 127.0);
        ly = (byte) (height * 127.0);
    }

    public boolean differ(final BuggyMandibles arg)
    {
        if (arg == null)
            return true;

        final byte linearMargin = 0;

        return !(withinMargin(rx, arg.rx, linearMargin)
            && withinMargin(ry, arg.ry, linearMargin)
            && withinMargin(rz, arg.rz, linearMargin)
            && withinMargin(lx, arg.lx, linearMargin)
            && withinMargin(ly, arg.ly, linearMargin)
            && withinMargin(lz, arg.lz, linearMargin));
    }

    private boolean withinMargin(final byte a1, final byte a2, final byte margin)
    {
        return Math.abs(a1 - a2) <= margin;
    }
}
