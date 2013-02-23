#include <Servo.h>

#include "Leg.h"
#include "Gait2.h"
#include "Mark1Config.h"
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
    Serial1.println("ksjfhksjd skdhf kjsdhfueyeyrueryeuu  iw ksjh skfd ");
  
    Serial.println("setup()");
    
    LegConfiguration::apply(legs, N);
    moveSimple.rememberDefault();
   
    for (Leg* leg = legs; leg < legs + N; leg++)
        leg->reachRelativeToDefault(zero);

/*
  moveSimple.walk(4, Point(0, 80, 50));
  moveSimple.walk(4, Point(-70, 0, 50));
  moveSimple.walk(4, Point(0, -80, 50));
  moveSimple.walk(4, Point(70, 0, 50));

  moveSimple.walk(2, Point(60, 60, 50));
  moveSimple.walk(2, Point(-60, 60, 50));
  moveSimple.walk(2, Point(-60, -60, 50));
  moveSimple.walk(2, Point(60, -60, 50));

  moveSimple.smoothTo(zero);

*/

  
/*  for (int i = 0; i < N; i++)
    legs[i].detach();
*/

    if (digitalRead(9) == HIGH)
        runSequence1();
        
    gait.setGait3x3();
    //gait.setGaitTest();
    //gait.
} 
 
void runSequence1()
{
    attachAll();
    moveSimple.shiftReset();

    moveSimple.walk(4, Point(0, 80, 50));
    moveSimple.smoothTo(zero);

    const int dt = 2;
    const float da = PI / 300;
    const float dh = 1.0;
    const int iterations = 30;

    for (int i = 0; i < iterations; ++i)
    {
        moveSimple.shiftRoll(- da);
        delay(dt);
    }
    
    for (int i = 0; i < iterations; ++i)
    {
        moveSimple.shiftPitch(- da);
        delay(dt);
    }
    
    for (int i = 0; i < iterations * 2; ++i)
    {
        moveSimple.shiftRoll(da);
        delay(dt);
    }
        
    for (int i = 0; i < iterations * 2; ++i)
    {
        moveSimple.shiftPitch(da);
        delay(dt);
    }
    
    for (int i = 0; i < iterations; ++i)
    {
        moveSimple.shiftPitch(- da);
        moveSimple.shiftRoll(- da);
        delay(dt);
    }
    
    moveSimple.shiftReset();
 
    moveSimple.walk(8, Point(0, 80, 50));
    
    moveSimple.smoothTo(zero);
    moveSimple.rotate(11, 1.0);

    moveSimple.smoothTo(zero);
    
    for (int i = 0; i < iterations * 4; ++i)
    {
        moveSimple.shift(Point(0,0, dh));
        delay(dt);
    }
    for (int i = 0; i < iterations * 5; ++i)
    {
        moveSimple.shift(Point(0,0, -dh));
        delay(dt);
    }
    
    moveSimple.walk(8, Point(0, 80, 50));
    moveSimple.shiftReset();
    

    moveSimple.smoothTo(zero);
    moveSimple.walk(2, Point(70, 0, 50));
    moveSimple.smoothTo(zero);
    moveSimple.walk(4, Point(-70, 0, 50));
    moveSimple.smoothTo(zero);
    moveSimple.walk(2, Point(70, 0, 50));
    
    moveSimple.smoothTo(zero);
    moveSimple.walk(4, Point(0, 80, 50));
    
    moveSimple.smoothTo(zero);
    moveSimple.rotate(11, 1.0);
    
    detachAll();
      
}
 
char command = 0;
unsigned long lastCommandTime = 0;
static float progress = 0; 

SmoothFloat fpitch(0,0);
SmoothFloat froll(0,0);
SmoothFloat fyaw(0,0);
Point bodyShift;

void tick()
{
    gait.tick();
    for (int i = 0; i < N; i++)
        legs[i].tick();
        
    static unsigned long _lastTickTime = 0;
    const unsigned long now = millis();
    const unsigned long deltaT = now - _lastTickTime;
    _lastTickTime = now;
    const float stepDelta = ((PI / 4) / 1000) * deltaT;
        
    moveSimple.shiftAbsolute(
        bodyShift,
        fpitch.getCurrent(stepDelta),
        froll.getCurrent(stepDelta),
        fyaw.getCurrent(stepDelta));
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
    
        bodyShift.x = normalizeByte(x, 20);  
        bodyShift.y = normalizeByte(y, 20);
        bodyShift.z = normalizeByte(z, 20);
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
        
        gait.setStep(Point(normalizeByte(x, 80),
                           normalizeByte(y, 80),
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
                gait.setGait3x3();
                break;
            case 2:
                gait.setGait6();
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
    else if (incoming == 'z' || incoming == '0')
    {
        moveSimple.smoothTo(zero);
        tone(9, 2000, 100);
        if (incoming == '0')
            runSequence1();
        
        command = incoming;
        lastCommandTime = millis();
        
        progress = 0;
        return false;
    }
    else if (incoming == 'r' ||
             incoming == 'f' ||
             incoming == 't' ||
             incoming == 'g' ||
             incoming == 'y' ||
             incoming == 'h' ||
             incoming == 'u' ||
             incoming == 'j' ||
             incoming == 'i' ||
             incoming == 'k' ||
             incoming == 'x')
    {
        command = incoming;
        lastCommandTime = millis();

        switch (incoming)
        {
            case 'r': moveSimple.shift(Point( 0,  0, -2)); break;
            case 'f': moveSimple.shift(Point( 0,  0,  2)); break;

            case 'y': moveSimple.shift(Point( 0, -2,  0)); break;
            case 'h': moveSimple.shift(Point( 0,  2,  0)); break;
            case 'g': moveSimple.shift(Point( 2,  0,  0)); break;
            case 'j': moveSimple.shift(Point(-2,  0,  0)); break;

            case 't': moveSimple.shiftRoll(- (PI / 100)); break;
            case 'u': moveSimple.shiftRoll(+ (PI / 100)); break;
            case 'i': moveSimple.shiftPitch(+ (PI / 100)); break;
            case 'k': moveSimple.shiftPitch(- (PI / 100)); break;
            
            case 'x': moveSimple.shiftReset(); break;
        }

        return false;
    }


    static char lastMoveCommand = 0;
    static char lastCommand = 0;
    bool moved = true;
    
    switch(command)
    {
        case 'w':
            progress = moveSimple.walk(1, Point(0, 80, 70), progress, NULL);
            //if (lastCommand != command)
            //    moveGait.updateMovementDirect(Point(0, 30, 0));
            break;
        case 's':
            progress = moveSimple.walk(1, Point(0, -80, 70), progress, NULL);
            //if (lastCommand != command)
            //    moveGait.updateMovementDirect(Point(0, -30, 0));
            break;
        case 'a':
            progress = moveSimple.walk(1, Point(-70, 0, 70), progress, NULL);
            //if (lastCommand != command)
            //    moveGait.updateMovementDirect(Point(-20, 0, 0));
            break;
        case 'd':
            progress = moveSimple.walk(1, Point(70, 0, 70), progress, NULL);
            //if (lastCommand != command)
            //    moveGait.updateMovementDirect(Point(20, 0, 0));
            break;
        case 'e':
            if (lastMoveCommand != command)
                moveSimple.smoothTo(zero);
            progress = moveSimple.rotate(1, 1.0, progress, NULL);
            break;
        case 'q':
            if (lastMoveCommand != command)
                moveSimple.smoothTo(zero);
            progress = moveSimple.rotate(1, -1.0, progress, NULL);
            break;
        default:
            moved = false;
//            moveGait.stop();
    }

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

//    const char currCommand = command;
    

        
//    delay(2);
  //  moveGait.tick();
} 
