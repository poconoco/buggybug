#ifndef GAIT_2_H__
#define GAIT_2_H__

#include <Arduino.h>
#include <HardwareSerial.h>

#include "Leg.h"
#include "Geometry.h"
#include "SmoothFloat.h"

#define CYCLE_MIDDLE 127
#define CYCLE_END 255


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
    {}

    void setLeg(Leg *leg, bool rightLeg)
    {
        _leg = leg;
        _rightLeg = rightLeg;
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

    void setCycleShift(byte shift)
    {
        _cycleShift = shift;
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
        if (cyclePos <= CYCLE_MIDDLE)
        {
            norm1pos = static_cast<float>(cyclePos * 2) / 255;
            height = 0.0;
        }
        else
        {
            norm1pos = static_cast<float>(CYCLE_END - ((cyclePos - CYCLE_MIDDLE) * 2)) / 255;
            height = _stepHeight * (1 - fabs(0.5 - norm1pos) * 2);
        }
    }

private:

    Leg* _leg;
    byte _cycleShift;

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
        , _currentCyclePos(CYCLE_MIDDLE)
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

    void setGait3x3()
    {
        _legCycles[0].setCycleShift(0);
        _legCycles[1].setCycleShift(CYCLE_MIDDLE);
        _legCycles[2].setCycleShift(0);
        _legCycles[3].setCycleShift(CYCLE_MIDDLE);
        _legCycles[4].setCycleShift(0);
        _legCycles[5].setCycleShift(CYCLE_MIDDLE);
    }

    void setGait6()
    {
        _legCycles[0].setCycleShift(0);
        _legCycles[5].setCycleShift((CYCLE_END / 6) * 1);
        _legCycles[1].setCycleShift((CYCLE_END / 6) * 2);
        _legCycles[4].setCycleShift((CYCLE_END / 6) * 3);
        _legCycles[2].setCycleShift((CYCLE_END / 6) * 4);
        _legCycles[3].setCycleShift((CYCLE_END / 6) * 5);
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
            angle = _turn.getCurrent() / 1.5;

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

        const float speedStepDelta = 0.01 * deltaT;
        const float directionStepDelta = 0.1 * deltaT;
        const float turnStepDelta = 0.01 * deltaT;
        _directionX.getCurrent(directionStepDelta);
        _directionY.getCurrent(directionStepDelta);
        _directionZ.getCurrent(directionStepDelta);
        _turn.getCurrent(turnStepDelta);

        tickStep();

        const float cycleDelta = deltaT * ((float)CYCLE_END / 1000) * _stepPerSecond.getCurrent(speedStepDelta);
        _currentCyclePos += cycleDelta;

        while (_currentCyclePos >= CYCLE_END)
            _currentCyclePos -= CYCLE_END;

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
