#include "Leg.h"

Leg legs[6];

Leg* rightLegs = legs;
Leg* leftLegs = legs + 3;

void log(const char* x) { Serial.println(x); }
void log(float x) { Serial.println(x); }
void log(int x) { Serial.println(x); }

void squareInConstXPlane(Leg& leg)
{
    for (int i = 0; i <= 60; i++)
    {
      leg.reach(Point(70, -30 + i, 10));
      delay(5);
    }
  
    for (int i = 0; i <= 60; i++)
    {
      leg.reach(Point(70, 30, 10 - i));
      delay(5);
    }
  
    for (int i = 0; i <= 60; i++)
    {
      leg.reach(Point(70, 30 - i, -50));
      delay(5);
    }
  
    for (int i = 0; i <= 60; i++)
    {
      leg.reach(Point(70, -30, -50 + i));
      delay(5);
    }
}



void squareInConstZPlane(Leg& leg)
{
    int z = -50;
    int pause = 0;
    int speedd = 5;
  
    for (int i = 0; i <= 60; i++)
    {
      leg.reach(Point(30, -50 + i, z));
      delay(speedd);
    }
    delay(pause);
  
    for (int i = 0; i <= 40; i++)
    {
      leg.reach(Point(30 + i, 10, z));
      delay(speedd);
    }
    delay(pause);
  
    for (int i = 0; i <= 60; i++)
    {
      leg.reach(Point(70, 10 - i, z));
      delay(speedd);
    }
    delay(pause);
  
    for (int i = 0; i <= 40; i++)
    {
      leg.reach(Point(70 - i, -50, z));
      delay(speedd);
    }
    delay(pause);
}

/*
void circleInConstZPlane()
{
    float z = -50;
    
    float r = 30;
    float cx = 50;
    float cy = -20;
    
    for (float pol = 0; pol < PI * 2; pol+=0.1)
    {
        test.reach({ r * cos(pol) + cx, r * sin(pol) + cy, z });
        delay(5);
    }
}
*/

void forwardBackward(Leg* legs, int legNum, int legStep, Point start, Point relativeEnd, int iterations)
{
    for (int i = 0; i < iterations; i++)
    {
        for(float p = 0; p <= 2; p += 0.02)
        {
            float progress; 
            if (p < 1) progress = p;
            else progress = 1 - (p - 1); 
          
            Point current(start.x + progress * relativeEnd.x,
                              start.y + progress * relativeEnd.y,
                              start.z + progress * relativeEnd.z );

            for (int li = 0; li < legNum * legStep; li += legStep)
                legs[li].reachRelative(current);                  

            delay(10);
        }        
    }
}

void walk(Leg* legs, int iterations)
{
    // Point(0, -60, 0)
    // Point(0, 60, 0)
    float stepLength = 80;
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
                height2 = 60 * (0.5 - fabs(0.5 - p));
            }
            else 
            {
                progress = 1 - (p - 1);
                height1 = 60 * (0.5 - fabs(1.5 - p));
                height2 = 0;
            }
          
            Point group1(0,
                         -60 + (stepLength - progress * stepLength),
                         height1);
            Point group2(0,
                         -60 + progress * stepLength,
                         height2);

            for (int li = 0; li < 3; li++)
            {
                legs[li * 2].reachRelative(group1);                  
                legs[li * 2 + 1].reachRelative(group2);                  
            }

            delay(2);
        }        
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

    rightLegs[0].configureDefaultReach(Point( 54, 140, -80));
    leftLegs [0].configureDefaultReach(Point(-54, 140, -80));
  
    rightLegs[1].configureDefaultReach(Point( 100, 30, -80));
    leftLegs [1].configureDefaultReach(Point(-100, 30, -80));

    rightLegs[2].configureDefaultReach(Point( 74, -80, -80));
    leftLegs [2].configureDefaultReach(Point(-74, -80, -80));
    
    //for (Leg* leg = legs; leg < legs + 6; leg++)
      //  leg->reachRelative(Point(0,0,0));
      //leg->reset();
}

void setup() 
{
    Serial.begin(9600);
  
    Serial.println("Starting");
    
    //for (Leg* leg = legs; leg < legs + 6; leg++)
     //   leg->debug(true);
    

    configureLegs();
    //rightLegs[0].attach(46, 47, 39);
    //rightLegs[0].reset();  
  
//  rightLegs[0].rechRelative(Point(0, 0, 0));
//  leftLegs[0].rechRelative(Point(0, 0, 0));
 
 /* 
  forwardBackward(legs, 
                  3,
                  2, 
                  Point(0, -60, 0), 
                  Point(0, 60, 0), 
                  5);
                  */

  walk(legs, 10);

  
  for (int n = 0; n < 4; n++)
  {
    //circleInConstZPlane();
    //squareInConstZPlane(rightLegs[0]);
    //squareInConstXPlane(rightLegs[0]);
  }
  
  //test.reset();

} 
 
 
void loop() 
{ 

} 
