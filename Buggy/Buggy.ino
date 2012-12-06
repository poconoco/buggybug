#include <Servo.h>

#include "Leg.h"

static const int N = 6;
static Leg legs[N];

static Leg* rightLegs = legs;
static Leg* leftLegs = legs + N / 2;

static Point zero(0,0,0);

float walk(int steps, Point direction, float startProgress, bool (*pContinue)() = NULL)
{
    float p = startProgress;
    for (int i = 0; i < steps; i++)
    {
        for(; p <= 2; /*p += 0.025*/)
        {
            float progress; 
            float height1;
            float height2;
            if (p < 1) 
            {
                progress = p;
                height1 = 0;
                height2 = direction.z * (0.5 - fabs(0.5 - p));
            }
            else 
            {
                progress = 1 - (p - 1);
                height1 = direction.z * (0.5 - fabs(1.5 - p));
                height2 = 0;
            }
          
            Point group1(- direction.x / 2 + (direction.x - progress * direction.x),
                         - direction.y / 2 + (direction.y - progress * direction.y),
                         height1);
            Point group2(- direction.x / 2 + progress * direction.x,
                         - direction.y / 2 + progress * direction.y,
                         height2);

            if (legs[0].getCurrentRelative().maxDistance(group1) > 10)
                smoothTo(group1, 0);
            if (legs[1].getCurrentRelative().maxDistance(group2) > 10)
                smoothTo(group2, 1);
                    
            for (int li = 0; li < N; li+=2)
            {
                legs[li].reachRelativeToDefault(group1);                  
                legs[li + 1].reachRelativeToDefault(group2);                  
            }

            delay(1);
            if (pContinue != NULL && ! pContinue())
                return p;
                
            p += 0.01 + 0.04 * (0.5 - fabs(0.5 - progress));
        }

        if (p > 2)
            p = 0;        
    }
    
    return p;
}

float rotate(int steps, float clockwise, float startProgress, bool (*pContinue)() = NULL)
{
    float p = startProgress;
    for (int i = 0; i < steps; i++)
    {
        for(; p <= 2; /*p += 0.025*/)
        {
            float progress; 
            float h[2]; // height
            float s[2]; // sine
            float c[2]; // cosine
            const float angle = clockwise * (PI / 500);

            if (p < 1) 
            {
                progress = p;
                h[0] = 0;
                h[1] = 50 * (0.5 - fabs(0.5 - p));
                s[0] = sin(angle);
                c[0] = cos(angle);
                s[1] = sin(- angle);
                c[1] = cos(- angle);
            }
            else 
            {
                progress = 1 - (p - 1);
                h[0] = 50 * (0.5 - fabs(1.5 - p));
                h[1] = 0;
                s[0] = sin(- angle);
                c[0] = cos(- angle);
                s[1] = sin(angle);
                c[1] = cos(angle);
            }

            for (int li = 0; li < N; ++li)
            {
                // li - leg index, gi - group index
                int gi = li % 2;
                Point pNew;
                Point pCurr = legs[li].getCurrentPos();
              
                pNew.x = pCurr.x * c[gi] - pCurr.y * s[gi];
                pNew.y = pCurr.x * s[gi] + pCurr.y * c[gi];
              
                // Calc default pos
                pNew.x -= legs[li].getDefaultPos().x;
                pNew.y -= legs[li].getDefaultPos().y;
                pNew.z = h[gi];

//                if (legs[li].getCurrentRelative().maxDistance(pNew) > 10)
  //                  smoothTo(pNew, gi);
                legs[li].reachRelativeToDefault(pNew);
            }

            delay(1);
            if (pContinue != NULL && ! pContinue())
                return p;
                
            p += 0.025 /*+ 0.00 * (0.5 - fabs(0.5 - progress))*/;
        }

        if (p > 2)
            p = 0;
    }
    
    return p;
}


void smoothTo(Point& to)
{
    smoothTo(to, 0);
    smoothTo(to, 1);
}

// Relative to default!
void smoothTo(Point& to, int legGroup)
{
    Point relative[N];
    Point currentPositions[N];
    for (int i = legGroup; i < N; i += 2)
    {
        currentPositions[i] = legs[i].getCurrentPos();
        relative[i].assign((legs[i].getDefaultPos() + to) - legs[i].getCurrentPos());
    }
  
    for(float p = 0; p <= 1; p += 0.03)
    {
        for (int li = legGroup; li < N; li += 2)
        {
            Point currSubStep = relative[li] * p;
            Point currStep = currentPositions[li] + currSubStep;
            currStep.z = legs[li].getDefaultPos().z + to.z + 50 * (0.5 - fabs(0.5 - p));
            legs[li].reachAbsolute(currStep);
        }
        
        delay(5);
    }
}

static bool attached;

void attachAll()
{
    if (attached)
        return;
      
    attached = true;  
    tone(9, 4000, 200);

    for (int i = 0; i < 6; i++)
        legs[i].attach();
}

void detachAll()
{
    if (! attached)
        return;
    
    attached = false;
    tone(9, 2000, 200);

    for (int i = 0; i < 6; i++)
        legs[i].detach();
}

bool attachChange()
{
    attached ? detachAll()
             : attachAll();
             
    return attached;
}

void configureLegs()
{
    rightLegs[0].attach(46, 47, 39);
    rightLegs[1].attach(40, 41, 42);
    rightLegs[2].attach(43, 44, 45);
    
    leftLegs[0].attach(29, 28, 36);
    leftLegs[1].attach(35, 34, 33);
    leftLegs[2].attach(32, 31, 30);
    
    attached = true;

    rightLegs[0].configureServoDirections(-1, -1,  1, false);
    leftLegs [0].configureServoDirections(-1,  1, -1, false);
    
    rightLegs[1].configureServoDirections(-1, -1,  1, false);
    leftLegs [1].configureServoDirections(-1,  1, -1, true);
    
    rightLegs[2].configureServoDirections(-1, -1,  1, false);
    leftLegs [2].configureServoDirections(-1,  1, -1, true);

    for (int i = 0; i < 3; i++)
    {
        rightLegs[i].configureFemur(-26, 12, 46.5, deg2rad(10));
        leftLegs [i].configureFemur(-26, 12, 46.5, deg2rad(10));

        rightLegs[i].configureTibia(58, deg2rad(-70));
        leftLegs [i].configureTibia(58, deg2rad(-70));
    }
    
    rightLegs[0].configureCoxa( 34,  65, deg2rad(      20),  10);                        
    leftLegs [0].configureCoxa(-34,  65, deg2rad(180 - 20), -10);

    rightLegs[1].configureCoxa( 52,   0, deg2rad(-         20),  10);                        
    leftLegs [1].configureCoxa(-52,   0, deg2rad(- (180 - 20)), -10);

    rightLegs[2].configureCoxa( 34, -65, deg2rad(-         63),  10);                        
    leftLegs [2].configureCoxa(-34, -65, deg2rad(- (180 - 63)), -10);

    rightLegs[0].configureDefault(Point( 94, 120, -70), true);
    leftLegs [0].configureDefault(Point(-94, 120, -70), true);
  
    rightLegs[1].configureDefault(Point( 120, 00, -70), true);
    leftLegs [1].configureDefault(Point(-120, 00, -70), true);

    rightLegs[2].configureDefault(Point( 94, -110, -70), true);
    leftLegs [2].configureDefault(Point(-94, -110, -70), true);
    
    // Fine tuning
    rightLegs[0].tuneRestAngles(PI / 2,
                                PI / 2 + deg2rad(10),
                                PI / 2);
    leftLegs[0].tuneRestAngles(PI / 2,
                                PI / 2 - deg2rad(5),
                                PI / 2);

    leftLegs[1].tuneRestAngles(PI / 2,
                                PI / 2,
                                PI / 2 - deg2rad(10));

    rightLegs[1].tuneRestAngles(PI / 2,
                                PI / 2,
                                PI / 2 + deg2rad(20));


    leftLegs[2].tuneRestAngles(PI / 2,
                                PI / 2 - deg2rad(10),
                                PI / 2);
    
    for (Leg* leg = legs; leg < legs + 6; leg++)
        leg->reachRelativeToDefault(zero);
      //leg->reset();
}

void setup() 
{
    Serial.begin(9600);
  
    Serial.println("setup()");
    
    //for (Leg* leg = legs; leg < legs + 6; leg++)
     //   leg->debug(true);
    

    configureLegs();

/*
  walk(4, Point(0, 80, 50));
  walk(4, Point(-70, 0, 50));
  walk(4, Point(0, -80, 50));
  walk(4, Point(70, 0, 50));

  walk(2, Point(60, 60, 50));
  walk(2, Point(-60, 60, 50));
  walk(2, Point(-60, -60, 50));
  walk(2, Point(60, -60, 50));

  smoothTo(zero);

*/

  
/*  for (int i = 0; i < 6; i++)
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
        smoothTo(zero);
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
    if (incoming <= 0 && millis() - lastCommandTime < 200)
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
    bool moved = true;

    switch(command)
    {
        case 'w':
            progress = walk(1, Point(0, 80, 50), progress, processCommands);
            break;
        case 's':
            progress = walk(1, Point(0, -80, 50), progress, processCommands);
            break;
        case 'a':
            progress = walk(1, Point(-70, 0, 50), progress, processCommands);
            break;
        case 'd':
            progress = walk(1, Point(70, 0, 50), progress, processCommands);
            break;
        case 'e':
            if (lastMoveCommand != command)
                smoothTo(zero);
            progress = rotate(1, 1.0, progress, processCommands);
            break;
        case 'q':
            if (lastMoveCommand != command)
                smoothTo(zero);
            progress = rotate(1, -1.0, progress, processCommands);
            break;
        default:
            moved = 0;
            break;
    }

    if (moved)
        lastMoveCommand = command;
} 
