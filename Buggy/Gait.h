#include <Leg.h>

class LegGroup
{
public:
    LegGroup(Leg** legs, int count)
    {
        _legs = legs;
        _count = count;
    }
    
    double reachedDistance()
    {
        // TODO
    }
    
private:
    Leg** _legs;
    int _count;
};

class Gait
{
public:
    Gait(LegGroup* group1, LegGroup* group2)
    {}
    
    void updateMovementForce(Point& force)
    {
        //TODO
    }
    
    void updateSpeedLimit(double speed)
    {
        //TODO
    }
    
    void tick()
    {
        // TODO
    }
    
private:
    LegGroup* group1;
    LegGroup* group2;
    
};
