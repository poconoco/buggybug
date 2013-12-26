#include <Servo.h> 
#include <math.h>

#define sqr(x) ((x) * (x))

Servo hips[8];  // Бёдра
Servo shins[8]; // Голени

int hipsTune[8];
int shinsTune[8];

const float hipLength  = 32;
const float shinLength = 40;
const float hipStartAngle = - M_PI / 2.0;
const float shinStartAngle = - 3.0 * M_PI / 4.0;

enum State
{
  STOP,
  FORWARD,
  BACKWARD,
  FORWARD_RIGHT,
  FORWARD_LEFT
};

State state = STOP;

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

  shinsTune[0] = 0; shinsTune[1] = -3;
  shinsTune[2] = 20; shinsTune[3] = 7;
  shinsTune[4] = 5; shinsTune[5] = 12;
  shinsTune[6] = 0; shinsTune[7] = 7;
}

void detachLegs()
{
  for (int i = 0; i < 8; ++i)
  {
    hips[i].detach();
    shins[i].detach();
  }
}

void resetLegs()
{
  int d = 20;
  
  for (int i = 0; i < 8; i++)
  {
    hipsWrite(i, 90);
    shinsWrite(i, 90 + ((i % 2) ? d : -d));
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
  if (state == FORWARD_RIGHT && (legIndex % 2) == 1)
      return;
  
  if (state == FORWARD_LEFT && (legIndex % 2) != 1)
      return;
  
  float hipAngle = hipAngleRad * 180.0 / M_PI + 90;
  float shinAngle = shinAngleRad * 180.0 / M_PI + 90;

  hipsWrite(legIndex, ! (legIndex % 2) ? hipAngle  : (180 - hipAngle));
  shinsWrite(legIndex, ! (legIndex % 2) ? (180 - shinAngle) : shinAngle);
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

void stepForward(float height, float deltaHeight, float xamp, float xshift)
{
  for (float i = 0; i < 200; i += 2.5)
  {
    float dx1 = + 10.0 - (float) i / 10;
    float dx2 = - 10.0 + (float) i / 10;
    
    float dhNormal = abs(dx1) / 10.0;
    float dh = deltaHeight * dhNormal;
    
    legsReachTo(dx1 * xamp + xshift, - height, 0);
    legsReachTo(dx2 * xamp + xshift, - (height - deltaHeight + dh), 1);
    
    delay(1);
  }
  
  for (float i = 0; i < 200; i += 2.5)
  {
    float dx1 = + 10.0 - (float) i / 10;
    float dx2 = - 10.0 + (float) i / 10;
    
    float dhNormal = abs(dx1) / 10.0;
    float dh = deltaHeight * dhNormal;
    
    legsReachTo(dx1 * xamp + xshift, - height, 1);
    legsReachTo(dx2 * xamp + xshift, - (height - deltaHeight + dh), 0);
    
    delay(1);
  }
}

void setup() 
{ 
  Serial1.begin(9600);

  attachLegs();
  resetLegs();
  delay(300);
  detachLegs();
} 

void loop() 
{
  float xamp = 1.5;
  float xshift = 5;
  
  float h = 66;
  float dh = 16;

  switch (state)
  {
    case FORWARD:    
    case FORWARD_RIGHT:    
    case FORWARD_LEFT:    
      attachLegs();
      stepForward(h, dh, xamp, xshift);
      detachLegs();
      break;
    case BACKWARD:
      attachLegs();
      stepForward(h, dh, - xamp, xshift);
      detachLegs();
      break;
  }
  
  char command;
  
  while (Serial1.available())
    command = Serial1.read();
    
  switch (command)
  {
    case 'w':
      state = FORWARD;
      break;
    case 's':
      state = BACKWARD;
      break;
    case 'd':
      state = FORWARD_RIGHT;
      break;
    case 'a':
      state = FORWARD_LEFT;
      break;
    default:
      state = STOP;
  }
}


