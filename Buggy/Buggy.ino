#include <Servo.h>
#include "Leg.h"

// TODO: Remove this temporary goo
void log(const char* x) { Serial.println(x); }
void log(float x) { Serial.println(x); }
void log(int x) { Serial.println(x); }

static Leg legs[6];
static int N = 6;//sizeof(legs) / sizeof(Leg);

static Leg* rightLegs = legs;
static Leg* leftLegs = legs + N / 2;

static Point zero(0,0,0);

static Point stateLinearMovement;

void processState()
{
    for (int i = 0; i < N; ++i)
        legs[i].reachRelativeToCurrent(stateLinearMovement);
}

void walk(Leg* legs, int iterations, Point direction)
{
    for (int i = 0; i < iterations; i++)
    {
        for(float p = 0; p <= 2; p += 0.02)
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
                    
            for (int li = 0; li < 6; li+=2)
            {
                    
              
                legs[li].reachRelativeToDefault(group1);                  
                legs[li + 1].reachRelativeToDefault(group2);                  
            }

            delay(1);
        }        
    }
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
  
    for(float p = 0; p <= 1; p += 0.02)
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

void configureLegs()
{
    rightLegs[0].attach(46, 47, 39);
    rightLegs[1].attach(40, 41, 42);
    rightLegs[2].attach(43, 44, 45);
    
    leftLegs[0].attach(29, 28, 36);
    leftLegs[1].attach(35, 34, 33);
    leftLegs[2].attach(32, 31, 30);


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

    rightLegs[0].configureDefault(Point( 84, 130, -70), true);
    leftLegs [0].configureDefault(Point(-84, 130, -70), true);
  
    rightLegs[1].configureDefault(Point( 120, 30, -70), true);
    leftLegs [1].configureDefault(Point(-120, 30, -70), true);

    rightLegs[2].configureDefault(Point( 94, -80, -70), true);
    leftLegs [2].configureDefault(Point(-94, -80, -70), true);
    
    // Fine tuning
/*    rightLegs[0].tuneRestAngles(PI / 2,
                                PI / 2 + deg2rad(10),
                                PI / 2);
    leftLegs[0].tuneRestAngles(PI / 2,
                                PI / 2 + deg2rad(10),
                                PI / 2);
*/
    leftLegs[1].tuneRestAngles(PI / 2,
                                PI / 2,
                                PI / 2 - deg2rad(10));

    rightLegs[1].tuneRestAngles(PI / 2,
                                PI / 2,
                                PI / 2 + deg2rad(10));
    
    for (Leg* leg = legs; leg < legs + 6; leg++)
        leg->reachRelativeToDefault(zero);
      //leg->reset();
}

void setup() 
{
    Serial.begin(9600);
  
    Serial.println("Starting");
    
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
  

  walk(legs, 2, Point(50, 50, 60));
  walk(legs, 2, Point(-50, 50, 60));
  walk(legs, 2, Point(-50, -50, 60));
  walk(legs, 2, Point(50, -50, 60));
  smoothTo(zero);

  
  for (int i = 0; i < 6; i++)
    legs[i].detach();

} 
 
 
void loop() 
{ 

} 
