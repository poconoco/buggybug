#ifndef GAIT_H__
#define GAIT_H__

#include <Arduino.h>

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

    LegGroup(Leg** legs, int count)
        : _legs(legs)
        , _count(count)
        , _state(DOWN)
    {
        _invCount = 1.0 / (float) _count;
    }

    void setState(State state)
    {
        _state = state;
    }

    State state() const
    {
        return _state;
    }

    Leg** legs() const
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
    Leg** _legs;
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
        , _stepSpeed(1.0) // In steps per second // TODO: configurable
        , _gaitHeight(10) // TODO: configurable
        , _raiseFactorThreshold(sqr(20.0)) // TODO: configurable
        , _putDownDistanceThreshold(5.0) // TODO: configurable
    {
        _lastTickTime = millis();
    }

    void updateMovementForce(Point& force)
    {
        // TODO
    }

    void updateSpeedLimit(float speed)
    {
        // TODO
    }

    // TODO make this protected and use only updateMovementForce() instead
    // x and y are speed in millimeters per second
    void updateMovementDirect(Point& movement)
    {
        _movement = movement;
        _movement.z = 0;
    }

    void tick()
    {
        const unsigned long now = millis();
        const unsigned long deltaT = now - _lastTickTime;

        int downCount = 0;

        // 1: Move body with legs that are down
        for (int gi = 0; gi < _groupCount; ++gi)
        {
            if (_groups[gi].state() == LegGroup::DOWN)
            {
                ++downCount;

                for (int li = 0; li < _groups[gi].count(); ++li)
                {
                    // TODO: radial movement
                    Point nextRel;
                    nextRel.x = (- _movement.x / 1000.0) * _stepSpeed * deltaT;
                    nextRel.y = (- _movement.y / 1000.0) * _stepSpeed * deltaT;
                    nextRel.z = 0;
                    _groups[gi].legs()[li]->reachRelativeToCurrent(nextRel);
                }
            }
        }

        // 2: Decide what leg groups should be raised (one group at a tick)
        // TODO: implement minimum raise interval
        if (downCount > _minGroupDown)
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

            if (bestFactor > _raiseFactorThreshold)
            {
                _groups[bestGi].setState(LegGroup::UP);
            }
        }

        // 3: Move leg groups that are raised, put down if needed
        for (int gi = 0; gi < _groupCount; ++gi)
        {
            if (_groups[gi].state() != LegGroup::DOWN)
            {
                int downCount = 0;

                for (int li = 0; li < _groups[gi].count(); ++li)
                {
                    Leg* leg = _groups[gi].legs()[li];
                    Point& def = leg->getDefaultPos();
                    Point currRel = leg->getCurrentRelative();

                    float defToNextStep = sqrt(sqr(_movement.x) + sqr(_movement.y));
                    float currToNextStep = distance2D(currRel.x, currRel.y, _movement.x, _movement.y);

                    if (currToNextStep < _putDownDistanceThreshold)
                    {
                        _groups[gi].legs()[li]->reachRelativeToDefault(_movement);
                        ++downCount;
                        break;
                    }

                    float progress = defToNextStep / currToNextStep;

                    Point upMovement;
                    upMovement.x = _movement.x - currRel.x;
                    upMovement.y = _movement.y - currRel.y;
                    upMovement.z = (0.5 - abs(0.5 - progress)) * 2 * _gaitHeight - currRel.z;

                    // Normalize upMovement to have defToNextStep length
                    normalize(upMovement, defToNextStep / currToNextStep);
                    
                    Point nextRel;
                    nextRel.z = (upMovement.z / 1000.0) * _stepSpeed * deltaT;
                    nextRel.x = (upMovement.x / 1000.0) * _stepSpeed * deltaT;
                    nextRel.y = (upMovement.y / 1000.0) * _stepSpeed * deltaT;

                    _groups[gi].legs()[li]->reachRelativeToCurrent(nextRel);
                }

                if (downCount == _groups[gi].count())
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
        float sumSqrDistance = 0.0;
        int i = 0;
        for (Leg* leg = *group.legs(); i < group.count(); i++, leg++)
        {
            Point currLegPos = leg->getCurrentRelative();
            sumSqrDistance += distanceSqr2D(currLegPos.x, currLegPos.y, _movement.x, _movement.y);
        }

        // Use invCount to spare divide operation in favor of multiply
        return sumSqrDistance * group.invCount();
    }

private:
    LegGroup _groups;
    const int _groupCount;
    const int _minGroupDown;

    Point _movement;
    float _stepSpeed;
    float _gaitHeight;
    float const _raiseFactorThreshold;
    float const _putDownDistanceThreshold;

    unsigned long _lastTickTime;
};

#endif
