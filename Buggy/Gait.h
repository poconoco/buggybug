#ifndef GAIT_H__
#define GAIT_H__

#include <Arduino.h>
#include <HardwareSerial.h>

#include "Leg.h"
#include "Geometry.h"

class LegGroup
{
public:

    enum State
    {
        DOWN,
        UP,
    };

    LegGroup(PLeg* legs, int count)
        : _legs(legs)
        , _count(count)
        , _state(DOWN)
    {
        _invCount = 1.0 / (float) _count;
    }
    
    void setState(State state)
    {
        _state = state;

        if (state == UP)
            for (PLeg* leg = legs(); leg < legs() + count(); ++leg)
                (*leg)->rememberRaisePoint();
    }

    State state() const
    {
        return _state;
    }

    PLeg* legs() const
    {
        return _legs;
    }

    int count() const
    {
        return _count;
    }

    float invCount() const
    {
        return _invCount;
    }

private:
    PLeg* _legs;
    
    int _count;
    float _invCount;

    State _state;
};

class Gait
{
public:

    Gait(LegGroup* groups, int groupCount, int minGroupDown)
        : _groups(groups)
        , _groupCount(groupCount)
        , _minGroupDown(minGroupDown)
        , _gaitHeight(25) // TODO: configurable
        , _putDownDistanceThreshold(0.5) // TODO: configurable
        , _upAllowed(true)
    {
        _lastTickTime = millis();
        setSpeed(3.0);
    }

    void updateMovementForce(Point& force)
    {
        // TODO
    }

    void updateSpeedLimit(float speed)
    {
        // TODO
    }

    void setSpeed(float speed)
    {
        _stepSpeedUp = speed;
        _stepSpeedDown = _stepSpeedUp * (1 + 0.2);
        
    }

    // TODO make this protected and use only updateMovementForce() instead
    // x and y are speed in millimeters per second
    void updateMovementDirect(Point movement)
    {
        _movement = movement;
        _movement.z = 0;
        
        _raiseFactorThreshold = distance2D(0, 0, _movement.x, _movement.y) * (1 + 0.2);
        _maxFactorThreshold = _raiseFactorThreshold * 2.25;
    }
    
    void stop()
    {
        _movement.x = 0;
        _movement.y = 0;
        _movement.z = 0;
    }

    void tick()
    {
        const unsigned long now = millis();
        const unsigned long deltaT = now - _lastTickTime;

        int downGroupCount = 0;

        // 1: Move body with legs that are down
        for (int gi = 0; gi < _groupCount; ++gi)
        {
            if (_groups[gi].state() == LegGroup::DOWN)
            {
                ++downGroupCount;

                // TODO avoid double factor calculation
                if (calcRaiseFactorForGroup(_groups[gi]) > _maxFactorThreshold)
                    continue;

                for (int li = 0; li < _groups[gi].count(); ++li)
                {
                    // TODO: radial movement
                    Point nextRel;
                    nextRel.x = (- _movement.x) * 0.001 * _stepSpeedDown * deltaT;
                    nextRel.y = (- _movement.y) * 0.001 * _stepSpeedDown * deltaT;
                    nextRel.z = 0;
                    _groups[gi].legs()[li]->reachRelativeToCurrent(nextRel);
                }
            }
        }

        // 2: Decide what leg groups should be raised (one group at a tick)
        // TODO: implement minimum raise interval
        if (downGroupCount > _minGroupDown)
        {
            // Find most recommended leg group for raising
            int bestGi = -1;
            float bestFactor = 0;
            for (int gi = 0; gi < _groupCount; ++gi)
            {
                if (_groups[gi].state() == LegGroup::DOWN)
                {
                    float factor = calcRaiseFactorForGroup(_groups[gi]);
                    if (factor > bestFactor)
                    {
                        bestFactor = factor;
                        bestGi = gi;
                    }
                }
            }

            if (bestFactor > _raiseFactorThreshold &&
                _upAllowed)
                _groups[bestGi].setState(LegGroup::UP);
        }

        // 3: Move leg groups that are raised, put down if needed
        for (int gi = 0; gi < _groupCount; ++gi)
        {
            if (floatEqual(_movement.x, 0.0) && floatEqual(_movement.y, 0.0))
                break;
          
            if (_groups[gi].state() != LegGroup::DOWN)
            {
                int downLegCount = 0;

                for (int li = 0; li < _groups[gi].count(); ++li)
                {
                    Leg* leg = _groups[gi].legs()[li];
                    Point currRel = leg->getCurrentRelative();
                    Point& raisedAt = leg->getRaisePoint();

                    float fullToNextStep = distance2D(raisedAt.x, raisedAt.y, _movement.x, _movement.y);
                    float currToNextStep = distance2D(currRel.x, currRel.y, _movement.x, _movement.y);

//                    float fullToNextStep = _movement.distance(raisedAt);
//                    float currToNextStep = _movement.distance(currRel);

                    if (currToNextStep < _putDownDistanceThreshold)
                    {
                        _groups[gi].legs()[li]->reachRelativeToDefault(_movement);
                        ++downLegCount;
                        continue;
                    }

                    float progress = fullToNextStep == 0 ? 1.0
                                                         : currToNextStep / fullToNextStep;
                    if (progress > 1.0)
                        progress = 1.0;
                        
                    Point upMovement;
                    upMovement.x = _movement.x - currRel.x;
                    upMovement.y = _movement.y - currRel.y;
                    float currHeight = (0.5 - fabs(0.5 - progress)) * 2 * _gaitHeight;

                    upMovement.z = currHeight - currRel.z;
                    
                    // Normalize upMovement to have fullToNextStep length
                    normalize2D(upMovement, fullToNextStep / currToNextStep);
                    
                    Point nextRel;
                    nextRel.x = upMovement.x * 0.001 * _stepSpeedUp * deltaT;
                    nextRel.y = upMovement.y * 0.001 * _stepSpeedUp * deltaT;
                    nextRel.z = upMovement.z/* * 0.001 * _stepSpeedUp * deltaT*/;

                    _groups[gi].legs()[li]->reachRelativeToCurrent(nextRel);
                }

                if (downLegCount == _groups[gi].count())
                    _groups[gi].setState(LegGroup::DOWN);
            }
        }

        _lastTickTime = now;
    }

private:

    // The bigger returned number, the more likely this group should be raised to move
    // In fact, it's average of square of distance from next step point of each leg to the current pos
    float calcRaiseFactorForGroup(const LegGroup& group) const
    {
        // TODO: radial movement

        // Using square distance to spare square root calculation
        float sumDistance = 0.0;
        for (PLeg* leg = group.legs(); leg < group.legs() + group.count(); ++leg)
        {
            Point currLegPos = (*leg)->getCurrentRelative();
            sumDistance += distance2D(currLegPos.x, currLegPos.y, _movement.x, _movement.y);
        }

        // Use invCount to spare divide operation in favor of multiply
        return sumDistance * group.invCount();
    }

private:
    LegGroup* _groups;
    const int _groupCount;
    const int _minGroupDown;

    Point _movement;
    float _stepSpeedDown;
    float _stepSpeedUp;
    float _gaitHeight;
    float _raiseFactorThreshold;
    float _maxFactorThreshold;
    float const _putDownDistanceThreshold;
    bool _upAllowed;

    unsigned long _lastTickTime;
};

#endif
