#include <Servo.h>

#include "Leg.h"
#include "Gait.h"
#include "Mark1Config.h"
#include "SimpleMovements.h"

static Point zero(0,0,0);

static const int N = 6;
static Leg legs[N];

static PLeg group1Legs[3] = {&legs[0], &legs[2], &legs[4]};
static PLeg group2Legs[3] = {&legs[1], &legs[3], &legs[5]};
static LegGroup legGroups[2] = { LegGroup(group1Legs, 3), LegGroup(group2Legs, 3) };

static Gait moveGait(legGroups, 2, 1);
static SimpleMovements moveSimple(legs, N);

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
  
    Serial.println("setup()");
    
    LegConfiguration::apply(legs, N);
   
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

} 
 
 
char command = 0;
unsigned long lastCommandTime = 0;
static float progress = 0; 

const char* commandToStr(char cmd)
{
    switch (cmd)
    {
        case 'w':
            return "forward";
        case 's':
            return "backward";
        case 'a':
            return "step left";
        case 'd':
            return "step right";
        case 'q':
            return "rotate left";
        case 'e':
            return "rotate right";
        case ' ':
            return "engine start/stop";
        case 'z':
            return "default legs pos";
        case 'r':
            return "body up";
        case 'f':
            return "body down";
    }
    
    return "n/a";
}

bool processCommands()
{
    int incoming = -1;
    while (Serial.available() > 0)
        incoming = Serial.read();

    // One shot actions
    if (incoming == ' ')
    {
        attachChange();
        
        command = incoming;
        lastCommandTime = millis();
        Serial.println(commandToStr(command));
        
        return false;
    }
    else if (incoming == 'z')
    {
        moveSimple.smoothTo(zero);
        tone(9, 2000, 100);
        
        command = incoming;
        lastCommandTime = millis();
        Serial.println(commandToStr(command));
        
        progress = 0;
        return false;
    }
    else if (incoming == 'r' || incoming == 'f')
    {
        command = incoming;
        lastCommandTime = millis();
        Serial.println(commandToStr(command));
        
        for (int i = 0; i < N; ++i)
        {
            Point def = legs[i].getDefaultPos(); 
            def.z = def.z + (incoming == 'r' ? -2 : 2);
            legs[i].shiftDefault(def);
        }
    }

    // Command is the same, continuing
    if (incoming == command)
    {
        lastCommandTime = millis();
        return true;
    }

    // Check if continuous command timed out
    if (incoming <= 0 && millis() - lastCommandTime < 500)
        return true;

    command = incoming;
    lastCommandTime = millis();

    Serial.println(commandToStr(command));

    tone(9, 8000, 100);
    return false;
}
 
void loop() 
{ 
    processCommands();

    static char lastMoveCommand = 0;
    static char lastCommand = 0;
    bool moved = true;
    const char currCommand = command;
    
    switch(currCommand)
    {
        case 'w':
            progress = moveSimple.walk(1, Point(0, 80, 50), progress, processCommands);
            //if (lastCommand != command)
            //    moveGait.updateMovementDirect(Point(0, 30, 0));
            break;
        case 's':
            progress = moveSimple.walk(1, Point(0, -80, 50), progress, processCommands);
            //if (lastCommand != command)
            //    moveGait.updateMovementDirect(Point(0, -30, 0));
            break;
        case 'a':
            progress = moveSimple.walk(1, Point(-70, 0, 50), progress, processCommands);
            //if (lastCommand != command)
            //    moveGait.updateMovementDirect(Point(-20, 0, 0));
            break;
        case 'd':
            progress = moveSimple.walk(1, Point(70, 0, 50), progress, processCommands);
            //if (lastCommand != command)
            //    moveGait.updateMovementDirect(Point(20, 0, 0));
            break;
        case 'e':
            if (lastMoveCommand != command)
                moveSimple.smoothTo(zero);
            progress = moveSimple.rotate(1, 1.0, progress, processCommands);
            break;
        case 'q':
            if (lastMoveCommand != command)
                moveSimple.smoothTo(zero);
            progress = moveSimple.rotate(1, -1.0, progress, processCommands);
            break;
        default:
            moved = false;
            moveGait.stop();
    }

    lastCommand = currCommand;
    if (moved)
        lastMoveCommand = currCommand;
        
    delay(2);
    moveGait.tick();
} 
