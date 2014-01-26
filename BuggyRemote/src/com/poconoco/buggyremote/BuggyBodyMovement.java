package com.poconoco.buggyremote;


public class BuggyBodyMovement
{
    public byte x;
    public byte y;
    public byte turn;
    public byte speed;

    // joystick* are -1..1
    public void encodeLinear(final double joystickX, final double joystickY)
    {
        final double length = Math.sqrt(joystickX * joystickX + joystickY
                * joystickY);

        if (length < 0.0001)
        {
            speed = 0;
            x = 0;
            y = 0;
            turn = 0;
            return;
        }

        speed = (byte) (length * 127);

        x = (byte) (joystickX * (1.0 / length) * 127.0);
        y = (byte) (joystickY * (1.0 / length) * 127.0);

        final double slowTreshold = 0.1;
        if (length < slowTreshold)
        {
            final double breaking = 1 - (slowTreshold - length)
                    / slowTreshold;

            x = (byte) (x * breaking);
            y = (byte) (y * breaking);
        }

        turn = 0;
    }

    // joystick* are -1..1
    public void encodeWithTurn(final double joystickX_, final double joystickY_)
    {
        final double joystickX = 0;
        final double joystickY = joystickY_;

        final double length = Math.sqrt(joystickX * joystickX + joystickY
                * joystickY);

        if (length < 0.0001)
        {
            speed = 0;
            x = 0;
            y = 0;
            turn = 0;
            return;
        }

        speed = (byte) (length * 127);

        x = (byte) (joystickX * (1.0 / length) * 127.0);
        y = (byte) (joystickY * (1.0 / length) * 127.0);

        final double slowTreshold = 0.1;
        if (length < slowTreshold)
        {
            final double breaking = 1 - (slowTreshold - length)
                    / slowTreshold;

            x = (byte) (x * breaking);
            y = (byte) (y * breaking);
            return;
        }

        turn = (byte) (joystickX_ * 127);
    }

    public void copyFrom(final BuggyBodyMovement src)
    {
        x = src.x;
        y = src.y;
        turn = src.turn;
        speed = src.speed;
    }

    public boolean differ(final BuggyBodyMovement arg)
    {
        if (arg == null)
            return true;

        return x != arg.x || y != arg.y || turn != arg.turn
                || speed != arg.speed;

    }
}
