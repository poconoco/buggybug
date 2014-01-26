package com.poconoco.buggyremote;

public class BuggyProtocol
{
    public static byte[] getToggleServoCmd()
    {
        return new byte[] {' '};
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

}
