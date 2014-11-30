#include <Servo.h>

#include "Leg.h"
#include "Gait2.h"
#include "Mark2Config.h"
#include "SimpleMovements.h"

static Point zero(0,0,0);

static const int N = 6;
static Leg legs[N];

//static PLeg group1Legs[3] = {&legs[0], &legs[2], &legs[4]};
//static PLeg group2Legs[3] = {&legs[1], &legs[3], &legs[5]};
//static LegGroup legGroups[2] = { LegGroup(group1Legs, 3), LegGroup(group2Legs, 3) };

//static Gait moveGait(legGroups, 2, 1);
static SimpleMovements moveSimple(legs, N);
static Gait2 gait(legs);

static bool attached = false;

void attachAll()
{
    if (attached)
        return;

    attached = true;  
    tone(9, 4000, 200);

    for (int i = 0; i < N; i++)
        legs[i].attach();
}

void detachAll()
{
    if (! attached)
        return;

    attached = false;
    tone(9, 2000, 200);

    for (int i = 0; i < N; i++)
        legs[i].detach();
}

bool attachChange()
{
    attached ? detachAll()
             : attachAll();
             
    return attached;
}

void setup() 
{
    Serial.begin(9600);
    Serial1.begin(9600);
    
    LegConfiguration::apply(legs, N);
    moveSimple.rememberDefault();
   
    for (Leg* leg = legs; leg < legs + N; leg++)
        leg->reachRelativeToDefault(zero);
        
    gait.setGait6x1();
} 
  
char command = 0;
unsigned long lastCommandTime = 0;
static float progress = 0; 

SmoothFloat fpitch(0,0);
SmoothFloat froll(0,0);
SmoothFloat fyaw(0,0);
SmoothFloat fbodyX(0,0);
SmoothFloat fbodyY(0,0);
SmoothFloat fbodyZ(0,0);

void tick()
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

bool tryMultibyte(char cmd)
{
    static unsigned long lastMoveCommandTime = 0;
    const unsigned long now = millis();

    if (cmd == 'b')
    {
        while (Serial1.available() < 7)
            tick();

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
            tick();

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
            tick();

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
    if (now - lastMoveCommandTime > 1000)
    {
        gait.setSpeed(0);
        gait.setStep(zero, false, 0);

        lastMoveCommandTime = now + 1000000;
    }

    return false;
}

bool processCommands()
{
    tick();

    char incoming;
    if (Serial1.available() > 0)
        incoming = Serial1.read();
    else return true;

    if (tryMultibyte(incoming))
        return true;

    // One shot actions
    if (incoming == ' ')
    {
        attachChange();

        command = incoming;
        lastCommandTime = millis();

        return false;
    }

    static char lastMoveCommand = 0;
    static char lastCommand = 0;
    bool moved = true;

    // Command is the same, continuing
    if (incoming == command)
    {
        lastCommandTime = millis();
        return true;
    }

    lastCommand = command;
    if (moved)
        lastMoveCommand = command;

    // Check if continuous command timed out
    if (incoming <= 0 && millis() - lastCommandTime < 500)
        return true;

    command = incoming;
    lastCommandTime = millis();

    tone(9, 8000, 100);
    return false;
}
 
void loop() 
{ 
    processCommands();
}
