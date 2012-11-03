#include <Servo.h>
#include "Leg.h"

static Leg legs[6];
static int N = 6;//sizeof(legs) / sizeof(Leg);

static Leg* rightLegs = legs;
static Leg* leftLegs = legs + N / 2;

static Point zero(0,0,0);

static Point stateLinearMovement;

void processState()
{
    // TODO:
    for (int i = 0; i < N; ++i)
        legs[i].reachRelativeToCurrent(stateLinearMovement);
}

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
            if (legs[1].getCurrentRelative().maxDistance(group2) > 5)
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

void smoothTo(Point& to)
{
    smoothTo(to, 0);
    smoothTo(to, 1);
}

// Relative to default!
void smoothTo(Point& to, int legGroup)
{
    Point relative[N];
    for (int i = legGroup; i < N; i += 2)
        relative[i].assign((legs[i].getDefaultPos() + to) - legs[i].getCurrentPos());
  
    for(float p = 0; p <= 1; p += 0.03)
    {
        for (int li = legGroup; li < N; li += 2)
        {
            Point currSubStep = relative[li] * p;
            Point currStep = legs[li].getCurrentPos() + currSubStep;
            currStep.z = legs[li].getDefaultPos().z + to.z + 50 * (0.5 - fabs(0.5 - p));
            legs[li].reach(currStep);
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

 /* stateLinearMovement.assign(0, -1, 0);
  for (int i = 0; i < 50; ++i)
  {
      processState();
      delay(20);
  }

  stateLinearMovement.assign(0, 1, 0);
  for (int i = 0; i < 50; ++i)
  {
      processState();
      delay(10);
  }*/
  
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
 
 
char direction = 0;
unsigned long directionTime = 0;
static float progress = 0; 

bool checkDirection()
{
    int incoming = -1;
    while (Serial.available() > 0)
        incoming = Serial.read();

    if (incoming == ' ')
    {
        attachChange();
        return false;
    }

    if (incoming == direction)
    {
        directionTime = millis();
        return true;
    }

    if (incoming <= 0 &&
            millis() - directionTime < 300)
        return true;

    direction = incoming;
    directionTime = millis();
        
    Serial.println("changing direction");
    tone (9, 8000, 100);
    return false;
}
 
void loop() 
{ 
    checkDirection();

  
    switch(direction)
    {
        case 'w':
            progress = walk(1, Point(0, 80, 50), progress, checkDirection);
            break;
        case 's':
            progress = walk(1, Point(0, -80, 50), progress, checkDirection);
            break;
        case 'a':
            progress = walk(1, Point(-70, 0, 50), progress, checkDirection);
            break;
        case 'd':
            progress = walk(1, Point(70, 0, 50), progress, checkDirection);
            break;
    }
} 
