//#define IOSTREAM_DEBUG

#ifdef IOSTREAM_DEBUG  
#include <iostream>
#else
#include <Servo.h> 
#endif
#include <math.h>

#define sqr(x) ((x) * (x))

#ifndef IOSTREAM_DEBUG  
Servo hips[8];  // Бёдра
Servo shins[8]; // Голени

int hipsTune[8];
int shinsTune[8];
#endif

const float hipLength  = 32;
const float shinLength = 40;
const float hipStartAngle = - M_PI / 2.0;
const float shinStartAngle = - 3.0 * M_PI / 4.0;

void hipsWrite(int idx, int val)
{
  hips[idx].write(val + hipsTune[idx]);
}

void shinsWrite(int idx, int val)
{
  shins[idx].write(val + shinsTune[idx]);
}

void attachLegs()
{
#ifndef IOSTREAM_DEBUG  
  hips [0].attach(30); hips [1].attach(45);
  shins[0].attach(31); shins[1].attach(44);
  hips [2].attach(32); hips [3].attach(43);
  shins[2].attach(33); shins[3].attach(42);
  hips [4].attach(34); hips [5].attach(41);
  shins[4].attach(35); shins[5].attach(40);
  hips [6].attach(29); hips [7].attach(46);
  shins[6].attach(28); shins[7].attach(47);
  
  hipsTune[0] = 4; hipsTune[1] = 12;
  hipsTune[2] = 10; hipsTune[3] = 0;
  hipsTune[4] = 17; hipsTune[5] = 0;
  hipsTune[6] = 4; hipsTune[7] = 0;

  shinsTune[0] = 0; shinsTune[1] = -10;
  shinsTune[2] = 20; shinsTune[3] = 0;
  shinsTune[4] = 5; shinsTune[5] = 5;
  shinsTune[6] = 0; shinsTune[7] = 0;
#endif
}

void detachLegs()
{
#ifndef IOSTREAM_DEBUG  
  for (int i = 0; i < 8; ++i)
  {
    hips[i].detach();
    shins[i].detach();
  }
#endif
}

void resetLegs()
{
  for (int i = 0; i < 8; i++)
  {
#ifndef IOSTREAM_DEBUG
    hipsWrite(i, 90);
    shinsWrite(i, 90);
#endif
  }
}

float polarAngle(float x, float y)
{
  if (x > 0)
    return atan(y / x);

  if (x < 0 && y >= 0)
    return atan(y / x) + M_PI;

  if (x < 0 && y < 0)
    return atan(y / x) - M_PI;

  // x = 0
  if (y > 0)
    return M_PI * 0.5;

  if (y < 0)
    return M_PI * -0.5;

  // y = 0
  return 0;
}

void legWrite(float hipAngleRad, float shinAngleRad, int legIndex)
{
#ifdef IOSTREAM_DEBUG  
  std::cout << std::endl;
  std::cout << "hip rad: " << hipAngleRad << std::endl;
  std::cout << "shin rad: " << shinAngleRad << std::endl;
#endif

  float hipAngle = hipAngleRad * 180.0 / M_PI + 90;
  float shinAngle = shinAngleRad * 180.0 / M_PI + 90;

#ifdef IOSTREAM_DEBUG  
  std::cout << std::endl;
  std::cout << "hip deg: " << hipAngle << std::endl;
  std::cout << "shin deg: " << shinAngle << std::endl;
#endif
  
#ifndef IOSTREAM_DEBUG
  hipsWrite(legIndex, ! (legIndex % 2) ? hipAngle  : (180 - hipAngle));
  shinsWrite(legIndex, ! (legIndex % 2) ? (180 - shinAngle) : shinAngle);
#else
  std::cout << std::endl;
  std::cout << ((legIndex % 2) ? hipAngle : (180 - hipAngle)) << std::endl;
  std::cout << ((legIndex % 2) ? (180 - shinAngle) : shinAngle) << std::endl;
#endif
}

void legsReachTo(float x, float y, int legGroup)
{
  // Решаем задачу инверсной кинематики на плоскости, в которой надо найти 2 угла
  // Мы знаем координаты точки крепления бедра (0;0), и координаты точки, куда хотим 
  // дотянуться - (x;y). Задача сводится к задаче поиска точки пересечения двух окружностей
  float A = -2 * x;
  float B = -2 * y;
  float C = sqr(x) + sqr(y) + sqr(hipLength) - sqr(shinLength);
  float X0 = -A * C / (sqr(A) + sqr(B));
  float Y0 = -B * C / (sqr(A) + sqr(B));
  float D = sqrt( sqr(hipLength) - (sqr(C) / (sqr(A) + sqr(B))) );
  float mult = sqrt( sqr(D) / (sqr(A) + sqr(B)) );
  float ax, ay, bx, by;
  ax = X0 + B * mult;
  bx = X0 - B * mult;
  ay = Y0 - A * mult;
  by = Y0 + A * mult;

  // Select solution on top as joint
  float jointLocalX = ax;
  float jointLocalY = ay;

  float hipPrimaryAngle  = polarAngle(jointLocalX, jointLocalY);
  float hipAngle = hipPrimaryAngle - hipStartAngle;
  
  float shinPrimaryAngle = polarAngle(x - jointLocalX, y - jointLocalY);
  float shinAngle = (shinPrimaryAngle - hipAngle) - shinStartAngle;

  if (legGroup % 2 == 0)
  {
    legWrite(hipAngle, shinAngle, 0);
    legWrite(hipAngle, shinAngle, 3);
    legWrite(hipAngle, shinAngle, 4);
    legWrite(hipAngle, shinAngle, 7);
  }
  else
  {
    legWrite(hipAngle, shinAngle, 1);
    legWrite(hipAngle, shinAngle, 2);
    legWrite(hipAngle, shinAngle, 5);
    legWrite(hipAngle, shinAngle, 6);
  }
}

void stepForward(float height, float deltaHeight)
{
  for (float i = 0; i < 200; i += 1.5)
  {
    float dx1 = + 10.0 - (float) i / 10;
    float dx2 = - 10.0 + (float) i / 10;
    
    float dhNormal = abs(dx1) / 10.0;
    float dh = deltaHeight * dhNormal;
    
    legsReachTo(dx1, - height, 0);
    legsReachTo(dx2, - (height - deltaHeight + dh), 1);
    
    delay(1);
  }
  
  for (float i = 0; i < 200; i += 1.5)
  {
    float dx1 = + 10.0 - (float) i / 10;
    float dx2 = - 10.0 + (float) i / 10;
    
    float dhNormal = abs(dx1) / 10.0;
    float dh = deltaHeight * dhNormal;
    
    legsReachTo(dx1, - height, 1);
    legsReachTo(dx2, - (height - deltaHeight + dh), 0);
    
    delay(1);
  }
}

void setup() 
{ 
  attachLegs();
  
  //resetLegs();
  
//  legsReachTo(-0, -60, 0);
//  legsReachTo(-0, -60, 1);
#ifndef IOSTREAM_DEBUG
 // delay(300);
#endif

  for (int i = 0; i < 6; ++i)
    stepForward(70, 16);

/*  for (int i = 0; i < 200; ++i)
  {
    
#ifndef IOSTREAM_DEBUG
    delay(1);
#endif
  }*/
  
  delay(200);
  detachLegs();
} 

void loop() {}

#ifdef IOSTREAM_DEBUG
int main()
{
  setup();
  return 0;
}
#endif

