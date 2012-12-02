#include <Leg.h>

class LegGroup
{
public:

    enum State
    {
        DOWN,
        RAISING,
        LOWERING
    };

    LegGroup(Leg** legs, int count)
        : _legs(legs)
        , _count(count)
        , _state(DOWN)
    {}

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

private:
    Leg** _legs;
    int _count;

    State _state;
};

class Gait
{
public:

    Gait(LegGroup* groups, int groupCount, int minGroupDown)
        : _groups(groups)
        , _groupCount(groupCount)
        , _minGroupDown(minGroupDown)
        , _speed(1.0) // TODO: configurable
        , _gaitHeight(10) // TODO: configurable
        , _raiseFactorThreshold(sqr(15.0)) // TODO: configurable
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
    }

    void tick()
    {
        const unsigned long now = millis();
        const unsigned long deltaT = now - _lastTickTime;

        int downCount = 0;

        // 1: Move body with legs that are down
        for (int gi = 0; gi < _groupCount; ++gi)
        {
            if (_groups[i].state() == LegGroup::DOWN)
            {
                ++downCount;

                for (int li = 0; li < _groups[i].count(); ++li)
                {
                    // TODO: radial movement
                    Point relMove;
                    relMove.x = (- _movement.x / 1000.0) * deltaT;
                    relMove.y = (- _movement.y / 1000.0) * deltaT;
                    relMove.z = 0;
                    _groups[i].legs()[li]->reachRelativeToCurrent(relMove);
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
                if (_groups[i].state() == LegGroup::DOWN)
                {
                    float factor = calcRaiseFactorForGroup(_groups[i]);
                    if (factor > bestFactor)
                    {
                        bestFactor = factor;
                        bestGi = gi;
                    }
                }
            }

            if (bestFactor > _raiseFactorThreshold)
            {
                _groups[bestGi].setState(LegGroup.RAISING);
            }
        }

        // 3: Move leg groups that are raised, put down if needed
        // TODO:

        _lastTickTime = now;
    }

private:

    // The bigger returned number, the more likely this group should be raised to move
    // In fact, it's average of square of distance from next step point of each leg to the current pos
    float calcRaiseFactorForGroup(LegGroup& group)
    {
        // TODO: radial movement
        float sumDistance = 0.0;
        for (Leg* leg = group.legs(), int i = 0; i < group.count(); i++, leg++)
        {
            Point currLegPos = leg->getCurrentRelative();
            sumDistance +=
                sqr(currLegPos.x - _movement.x) +
                sqr(currLegPos.y - _movement.y);
        }

        return sumDistance / group.count();
    }

private:
    const LegGroup* _groups;
    const int _groupCount;
    const int _minGroupDown;

    Point _movement;
    float _speed;
    float _gaitHeight;
    float _raiseFactorThreshold;

    unsigned long _lastTickTime;
};
