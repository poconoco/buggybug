package com.poconoco.buggyremote;

public class BuggyProtocol
{
    public static byte[] getToggleServoCmd()
    {
        return new byte[] {' '};
    }

    public static byte[] getToggleMandibleCmd()
    {
        return new byte[] {'_'};
    }

    public static byte[] getGaitCmd(final int gait)
    {
        return new byte[] { 'g', (byte) gait, 'G'};
    }

    public static byte[] getBodyShiftCmd(final BuggyBodyShift bodyShift)
    {
        final byte[] cmd = new byte[8];

        cmd[0] = 'b';

        cmd[1] = bodyShift.x;
        cmd[2] = bodyShift.y;
        cmd[3] = bodyShift.z;

        cmd[4] = bodyShift.pitch;
        cmd[5] = bodyShift.roll;
        cmd[6] = bodyShift.yaw;

        cmd[7] = 'B';

        return cmd;
    }

    public static byte[] getBodyMovementCmd(final BuggyBodyMovement bodyMovement)
    {
        final byte[] cmd = new byte[6];

        cmd[0] = 'm';

        cmd[1] = bodyMovement.x;
        cmd[2] = bodyMovement.y;
        cmd[3] = bodyMovement.turn;
        cmd[4] = bodyMovement.speed;

        cmd[5] = 'M';

        return cmd;
    }

    public static byte[] getMandiblesCmd(final BuggyMandibles mandibles)
    {
        final byte[] cmd = new byte[8];

        cmd[0] = 'p';

        cmd[1] = mandibles.rx;
        cmd[2] = mandibles.ry;
        cmd[3] = mandibles.rz;

        cmd[4] = mandibles.lx;
        cmd[5] = mandibles.ly;
        cmd[6] = mandibles.lz;

        cmd[7] = 'P';

        return cmd;
    }

}
