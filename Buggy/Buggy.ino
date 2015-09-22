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

bool trySerialMultibyte(char cmd)
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
            
        char buffer[6];
        for (int i = 0; i < sizeof(buffer); ++i)
          buffer[i] = Serial1.read();

        // Confirm footer byte
        if (Serial1.read() != 'P')
            return false;
            
        fRMandibleX.setTarget(normalizeByte(buffer[0], 50));
        fRMandibleY.setTarget(normalizeByte(buffer[1], 50));
        fRMandibleZ.setTarget(normalizeByte(buffer[2], 50));

        fLMandibleX.setTarget(normalizeByte(buffer[3], 50));
        fLMandibleY.setTarget(normalizeByte(buffer[4], 50));
        fLMandibleZ.setTarget(normalizeByte(buffer[5], 50));

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

    static unsigned long _lastTickTime = 0;
    const unsigned long now = millis();
    const unsigned long deltaT = now - _lastTickTime;
    const float angleStepDelta = ((PI * 0.50) * 0.005) * (float) deltaT;
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

    _lastTickTime = now;
}

const int rcPins[6] = { 2, 3, 4, 5, 6, 7 };
const int rcMin = 1300;
const int rcMax = 2600;
const int rcMid = (rcMin + rcMax) / 2;

const int rcMinThreshold = rcMin + (rcMax - rcMin) / 3;
const int rcMaxThreshold = rcMax - (rcMax - rcMin) / 3;

const int rcMidLowThreshold = rcMid - 50;
const int rcMidHighThreshold = rcMid + 50;

const float rcOneDivRangeHalf = 2.0 / (rcMax - rcMin);

const int tumblerAPin = 8;
const int tumblerBPin = 9;

void configureRCReceiver()
{
    for (int i = 0; i < 6; ++i)
        pinMode(rcPins[i], INPUT);
}

void configureTumbler()
{
    pinMode(tumblerAPin, INPUT);
    pinMode(tumblerBPin, INPUT);
}

void setup()
{
    Serial1.begin(9600);
    Serial.begin(9600);
    Serial.println("ready");
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

    configureRCReceiver();
    configureTumbler();

    gait.setGait2x3();
}

void trySerialInput()
{
    char incoming;
    
    if (Serial1.available() > 0)
        incoming = Serial1.read();
    else
        return;

    if (trySerialMultibyte(incoming))
        return;

    // One shot actions
    if (incoming == ' ')
        toggleLegs();
    else if (incoming == '_')
        toggleMandibles();
}

void readTumblerInput()
{
    int tumblerA = digitalRead(tumblerAPin);
    int tumblerB = digitalRead(tumblerBPin);

    Gait newGait = gait2x3;
    static Gait previousGait = newGait;
  
    if (tumblerA == LOW && tumblerB == HIGH)
        newGait = gait3x2;
    else if (tumblerA == HIGH && tumblerB == LOW)
        newGait = gait6x1;
        
    if (previousGait != newGait)
        gait.setGait(newGait);
}

bool isRcInputMiddle(int input)
{
    return input > rcMidLowThreshold && input < rcMidHighThreshold;
}

int alignIfMiddle(int input)
{
    return isRcInputMiddle(input) ? rcMid : input;
}

void readRCReceiverInput()
{
    int rcInput[6];
    bool engaged = false;
    for (int i = 0; i < 6; ++i)
    {
        rcInput[i] = pulseIn(rcPins[i], HIGH, 10000);
        if (rcInput[i] != 0)
            engaged = true;
    }
    
    if (! engaged)
    {
        detachAllLegs();
        return;
    }
    
    attachAllLegs();
   
    int rawTurn = alignIfMiddle(rcInput[0]);
    int rawY = alignIfMiddle(rcInput[1]);

    static unsigned long firstZeroYTime = 0;
    
    if (rawY == rcMid)
    {
        if (firstZeroYTime == 0)
            firstZeroYTime = millis();
        else
            if (millis() - firstZeroYTime > 1000)
                gait.stop();
    }
    else
    {
        firstZeroYTime = 0;
    }
    
    float x = 0; // turn, no linear
    float y = normalize(rawY, rcMin, rcOneDivRangeHalf, 40.0);
    float turn = normalize(rawTurn, rcMin, rcOneDivRangeHalf, 1.0);   
       
    gait.setStep(Point(x, y, 0),
                       ! isRcInputMiddle(rawTurn),
                       turn);

    gait.setSpeed(fabs(normalize(rawY, rcMin, rcOneDivRangeHalf, 2.0)));
    
    int rawPitch = alignIfMiddle(rcInput[2]);
    int rawRoll = alignIfMiddle(rcInput[3]);
    int rawBodyZ = alignIfMiddle(rcInput[5]);
    
    fpitch.setTarget(normalize(rawPitch, rcMin, rcOneDivRangeHalf, PI / 6));
    froll.setTarget(normalize(rawRoll, rcMin, rcOneDivRangeHalf, PI / 6));
    fbodyZ.setTarget(normalize(rawBodyZ, rcMin, rcOneDivRangeHalf, 40));

    int rawGait = rcInput[4];
    if (rawGait < rcMinThreshold)
        gait.setGait(gait6x1);
    else if (rawGait >= rcMinThreshold && rawGait < rcMaxThreshold)
        gait.setGait(gait2x3);
    else 
        gait.setGait(gait3x2);
        
    //Serial.println(fpitch.getCurrent());
}

void loop()
{
    tickMovements();
    //readTumblerInput();
    readRCReceiverInput();
}
