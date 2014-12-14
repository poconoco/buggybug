#include <Servo.h>

#include "Leg.h"
#include "Gait2.h"
#include "Mark2Config.h"
#include "SimpleMovements.h"

static Point zero(0,0,0);

static const int N = 6;
static Leg legs[N];
static Leg mandibles[2];

static SimpleMovements moveSimple(legs, N, mandibles, 2);
static Gait2 gait(legs);

static bool attached = false;
static bool mandiblesAttached = false;

char command = 0;
unsigned long lastCommandTime = 0;
static float progress = 0;

SmoothFloat fpitch(0,0);
SmoothFloat froll(0,0);
SmoothFloat fyaw(0,0);
SmoothFloat fbodyX(0,0);
SmoothFloat fbodyY(0,0);
SmoothFloat fbodyZ(0,0);

SmoothFloat fRMandibleX(0,0);
SmoothFloat fRMandibleY(0,0);
SmoothFloat fRMandibleZ(0,0);
SmoothFloat fLMandibleX(0,0);
SmoothFloat fLMandibleY(0,0);
SmoothFloat fLMandibleZ(0,0);

void attachAllLegs()
{
    if (attached)
        return;

    attached = true;

    for (int i = 0; i < N; i++)
        legs[i].attach();
}

void detachAllLegs()
{
    if (! attached)
        return;

    attached = false;

    for (int i = 0; i < N; i++)
        legs[i].detach();
}

void attachMandibles()
{
    if (mandiblesAttached)
        return;

    mandiblesAttached = true;
    mandibles[0].attach();
    mandibles[1].attach();
}

void detachMandibles()
{
    if (! mandiblesAttached)
        return;

    mandiblesAttached = false;
    mandibles[0].detach();
    mandibles[1].detach();
}

bool toggleMandibles()
{
    mandiblesAttached ? detachMandibles()
                      : attachMandibles();

    return mandiblesAttached;
}

bool toggleLegs()
{
    attached ? detachAllLegs()
             : attachAllLegs();

    return attached;
}

bool tryMultibyte(char cmd)
{
    static unsigned long lastMoveCommandTime = 0;
    const unsigned long now = millis();

    if (cmd == 'b')
    {
        while (Serial1.available() < 7)
            tickMovements();

        char x, y, z, pitch, roll, yaw;
        x = Serial1.read();
        y = Serial1.read();
        z = Serial1.read();
        pitch = Serial1.read();
        roll = Serial1.read();
        yaw = Serial1.read();

        // Confirm footer byte
        if (Serial1.read() != 'B')
            return false;

        fbodyX.setTarget(normalizeByte(x, 40));
        fbodyY.setTarget(normalizeByte(y, 40));
        fbodyZ.setTarget(normalizeByte(z, 40));
        fpitch.setTarget(normalizeByte(pitch, PI / 6));
        froll.setTarget(normalizeByte(roll, PI / 6));
        fyaw.setTarget(normalizeByte(yaw, PI / 6));

        return true;
    }

    if (cmd == 'm')
    {
        while (Serial1.available() < 5)
            tickMovements();

        char x,y,turn, speed;

        x = Serial1.read();
        y = Serial1.read();
        turn = Serial1.read();
        speed = Serial1.read();

        // Confirm footer byte
        if (Serial1.read() != 'M')
            return false;

        gait.setStep(Point(normalizeByte(x, 40),
                           normalizeByte(y, 40),
                           0),
                     turn != 0,
                     normalizeByte(turn, 1.0));

        if (speed < 0)
            speed = 0;

        gait.setSpeed(normalizeByte(speed, 2.0));

        return true;
    }

    if (cmd == 'g')
    {
        while (Serial1.available() < 2)
            tickMovements();

        char gaitId = Serial1.read();

        // Confirm footer byte
        if (Serial1.read() != 'G')
            return false;

        switch (gaitId)
        {
            case 1:
                gait.setGait2x3();
                break;
            case 2:
                gait.setGait6x1();
                break;
            case 3:
                gait.setGait3x2();
                break;
        }

        lastMoveCommandTime = now;
        return true;
    }

    if (cmd == 'p')
    {
        while (Serial1.available() < 7)
            tickMovements();

        fRMandibleX.setTarget(normalizeByte(Serial1.read(), 50));
        fRMandibleY.setTarget(normalizeByte(Serial1.read(), 50));
        fRMandibleZ.setTarget(normalizeByte(Serial1.read(), 50));

        fLMandibleX.setTarget(normalizeByte(Serial1.read(), 50));
        fLMandibleY.setTarget(normalizeByte(Serial1.read(), 50));
        fLMandibleZ.setTarget(normalizeByte(Serial1.read(), 50));

        // Confirm footer byte
        if (Serial1.read() != 'P')
            return false;

        return true;
    }

    if (now - lastMoveCommandTime > 1000)
    {
        gait.setSpeed(0);
        gait.setStep(zero, false, 0);

        lastMoveCommandTime = now + 1000000;
    }

    return false;
}

void tickMovements()
{
    gait.tick();

#ifdef SMOOTH_ANGLES
    for (int i = 0; i < N; i++)
        legs[i].tick();
#endif

    static unsigned long _lastTickTime = 0;
    const unsigned long now = millis();
    const unsigned long deltaT = now - _lastTickTime;
    const float angleStepDelta = ((PI * 0.25) * 0.001) * (float) deltaT;
    const float shiftStepDelta = (100.0 * 0.001) * (float) deltaT;
    const float shiftMandibleDelta = (200.0 * 0.001) * (float) deltaT;

    Point bodyShift(fbodyX.getCurrent(shiftStepDelta),
                    fbodyY.getCurrent(shiftStepDelta),
                    fbodyZ.getCurrent(shiftStepDelta));

    moveSimple.shiftAbsolute(
        bodyShift,
        fpitch.getCurrent(angleStepDelta),
        froll.getCurrent(angleStepDelta),
        fyaw.getCurrent(angleStepDelta));

    moveSimple.mandiblesReach(Point(fRMandibleX.getCurrent(shiftMandibleDelta),
                                    fRMandibleY.getCurrent(shiftMandibleDelta),
                                    fRMandibleZ.getCurrent(shiftMandibleDelta)),
                              Point(fLMandibleX.getCurrent(shiftMandibleDelta),
                                    fLMandibleY.getCurrent(shiftMandibleDelta),
                                    fLMandibleZ.getCurrent(shiftMandibleDelta)));

    _lastTickTime = now;
}

void setup()
{
    Serial1.begin(9600);
//    Serial1.println("AT+BAUD8");
//    delay(500);
//    Serial1.begin(115200);

    LegConfiguration::apply(legs, N);
    MandibleConfiguration::apply(mandibles, 2);

    moveSimple.rememberDefault();

    for (Leg* leg = legs; leg < legs + N; leg++)
        leg->reachRelativeToDefault(zero);

    for (Leg* mandible = mandibles; mandible < mandibles + 2; mandible++)
        mandible->reachRelativeToDefault(zero);

    gait.setGait6x1();
}

void loop()
{
    tickMovements();
    char incoming;

    if (Serial1.available() > 0)
        incoming = Serial1.read();
    else
        return;

    if (tryMultibyte(incoming))
        return;

    // One shot actions
    if (incoming == ' ')
        toggleLegs();
    else if (incoming == '_')
        toggleMandibles();
}
