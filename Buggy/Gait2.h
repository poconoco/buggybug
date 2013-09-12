#ifndef GAIT_2_H__
#define GAIT_2_H__

#include <Arduino.h>
#include <HardwareSerial.h>

#include "Leg.h"
#include "Geometry.h"
#include "SmoothFloat.h"

class LegCycle
{
public:

    LegCycle()
        : _leg(NULL)
        , _cycleShift(0)
        , _stepHeight(0)
        , _turn(false)
        , _turnAngle(0)
        , _turnCX(0)
        , _turnCY(0)
    {
        setCycleMiddle(127);
    }

    void setLeg(Leg *leg, bool rightLeg)
    {
        _leg = leg;
        _rightLeg = rightLeg;
    }

    void setCycleShift(byte shift)
    {
        _cycleShift = shift;
    }

    void setCycleMiddle(byte middle)
    {
        _cycleMiddle = middle;
        _invCycleMiddle = 1.0 / static_cast<float>(_cycleMiddle);

        _cycleAfterMiddle = 255 - _cycleMiddle;
        _invCycleAfterMiddle = 1.0 / static_cast<float>(_cycleAfterMiddle);
    }

    void setStep(Point start, 
                 Point stop, 
                 float height, 
                 bool turn, 
                 float turnAngle,
                 float turnCX,
                 float turnCY)
    {
        _start = start;
        _stop = stop;
        _stepHeight = height;
        _turn = turn;
        _turnAngle = turnAngle;
        _turnCX = turnCX;
        _turnCY = turnCY;
    }

    void setCyclePos(byte pos)
    {
        float norm1pos;
        float height;
        cycle(shiftCyclePos(pos), norm1pos, height);

        Point point;

        if (! _turn)
        {
            point.x = _start.x + (_stop.x - _start.x) * norm1pos;
            point.y = _start.y + (_stop.y - _start.y) * norm1pos;
            point.z = _start.z + (_stop.z - _start.z) * norm1pos + height;

            _leg->reachRelativeToDefault(point);
        }
        else
        {
            point = _start;
            point.z += height;
            _leg->reachRelativeToDefaultAndRotate(point, _turnAngle * norm1pos, _turnCX, _turnCY);
        }
    }

private:

    inline byte shiftCyclePos(byte pos)
    {
        // Hope overflow will work as expected here :)
        return pos + _cycleShift;
    }

    void cycle(byte cyclePos, float& norm1pos, float& height)
    {
        if (cyclePos <= _cycleMiddle)
        {
            norm1pos = static_cast<float>(cyclePos) * _invCycleMiddle;
            height = 0.0;
        }
        else
        {
            norm1pos = static_cast<float>(_cycleAfterMiddle - (cyclePos - _cycleMiddle)) * _invCycleAfterMiddle;
            height = _stepHeight * (1 - fabs(0.5 - norm1pos) * 2);
        }
    }

private:

    Leg* _leg;
    byte _cycleShift;
    byte _cycleMiddle;
    float _invCycleMiddle;
    byte _cycleAfterMiddle;
    float _invCycleAfterMiddle;

    Point _start;
    Point _stop;
    float _stepHeight;
    bool  _turn;
    float _turnAngle;
    float _turnCX;
    float _turnCY;
    bool _rightLeg;
};

class Gait2
{
public:

    Gait2(Leg* legs)
        : _legs(legs)
        , _stepHeight(50)
        , _stepPerSecond(0, 0)
        , _lastTickTime(0)
        , _currentCyclePos(0)
        , _turn(0, 0)
        , _directionX(0, 0)
        , _directionY(0, 0)
        , _directionZ(0, 0)
        , _doTurn(false)
    {
        for (byte i = 0; i < 6; ++i)
            _legCycles[i].setLeg(&_legs[i], i < 3);

        _rightLegCycles = _legCycles;
        _leftLegCycles = _legCycles + 3;
    }

    void setGait2x3()
    {
        _legCycles[0].setCycleShift(0);
        _legCycles[1].setCycleShift(127);
        _legCycles[2].setCycleShift(0);
        _legCycles[3].setCycleShift(127);
        _legCycles[4].setCycleShift(0);
        _legCycles[5].setCycleShift(127);
        
        for (byte i = 0; i < 6; ++i)
            _legCycles[i].setCycleMiddle(127);
    }

    void setGait3x2()
    {
        _legCycles[0].setCycleShift(0);
        _legCycles[5].setCycleShift(0);
        
        _legCycles[1].setCycleShift((255 / 3) * 1);
        _legCycles[4].setCycleShift((255 / 3) * 1);
        
        _legCycles[2].setCycleShift((255 / 3) * 2);
        _legCycles[3].setCycleShift((255 / 3) * 2);
        
        for (byte i = 0; i < 6; ++i)
            _legCycles[i].setCycleMiddle(255 - (255 / 3));
    }

    void setGait6x1()
    {
        _legCycles[0].setCycleShift(0);
        _legCycles[1].setCycleShift((255 / 6) * 1);
        _legCycles[2].setCycleShift((255 / 6) * 2);
        _legCycles[3].setCycleShift((255 / 6) * 3);
        _legCycles[4].setCycleShift((255 / 6) * 4);
        _legCycles[5].setCycleShift((255 / 6) * 5);
        
        for (byte i = 0; i < 6; ++i)
            _legCycles[i].setCycleMiddle(255 - (255 / 6));
    }

    void setStep(Point dir, bool turn, float turnValue)
    {
        _directionX.setTarget(dir.x);
        _directionY.setTarget(dir.y);
        _directionZ.setTarget(dir.z);
        _turn.setTarget(turnValue);
        _doTurn = turn;
    }

    void tickStep()
    {
        Point direction(_directionX.getCurrent(),
                        _directionY.getCurrent(),
                        _directionZ.getCurrent());

        float cx, cy; // rotate around these points
        float angle;
        if (_doTurn)
        {
            angle = _turn.getCurrent() * 0.7;

            cx = distanceToHorde(fabs(direction.y), angle);
            cy = 0;

            if (direction.y < 0)
                angle *= -1;
        }

        Point stepStart = direction * 0.5;
        Point stepStop = stepStart * (-1);

        for (byte i = 0; i < 6; ++i)
            _legCycles[i].setStep(stepStart,
                                  stepStop,
                                  _stepHeight,
                                  _doTurn,
                                  angle,
                                  cx,
                                  cy);
    }

    void setSpeed(float stepsPerSecond)
    {
        _stepPerSecond.setTarget(stepsPerSecond);
    }

    void tick()
    {
        const unsigned long now = millis();
        const unsigned long deltaT = now - _lastTickTime;
        _lastTickTime = now;

        const float speedStepDelta = 0.01 * (float) deltaT;
        const float directionStepDelta = 0.1 * (float) deltaT;
        const float turnStepDelta = 0.01 * (float) deltaT;
        _directionX.getCurrent(directionStepDelta);
        _directionY.getCurrent(directionStepDelta);
        _directionZ.getCurrent(directionStepDelta);
        _turn.getCurrent(turnStepDelta);

        tickStep();

        const float cycleDelta = deltaT * (255.0 * 0.001) * _stepPerSecond.getCurrent(speedStepDelta);
        _currentCyclePos += cycleDelta;

        while (_currentCyclePos >= 255)
            _currentCyclePos -= 255;

        byte currCyclePosByte = (byte) _currentCyclePos;

        for (byte i = 0; i < 6; ++i)
            _legCycles[i].setCyclePos(currCyclePosByte);
    }

private:

    LegCycle _legCycles[6];
    LegCycle* _rightLegCycles;
    LegCycle* _leftLegCycles;
    Leg* _legs;
    float _stepHeight;
    float _currentCyclePos; // 0 .. 255 number to be rounded to byte

    bool _doTurn;
    SmoothFloat _stepPerSecond;
    SmoothFloat _turn;
    SmoothFloat _directionX;
    SmoothFloat _directionY;
    SmoothFloat _directionZ;

    unsigned long _lastTickTime;
};

#endif
