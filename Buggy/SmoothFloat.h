#ifndef SMOOTH_FLOAT_H__
#define SMOOTH_FLOAT_H__

#include "Geometry.h"

class SmoothFloat
{
public:

    SmoothFloat(float target, float current)
        : _target(target)
        , _current(current)
    {}
    
    void setTarget(float target)
    {
        _target = target;
    }

    float getCurrent(float stepDelta)
    {
        if (_current >= _target)
        {
            if (_current - _target <= stepDelta)
                _current = _target;
            else
                _current -= stepDelta;
        }
        else
        {
            if (_target - _current <= stepDelta)
                _current = _target;
            else
                _current += stepDelta;
        }

        return _current;
    }
    
private:

    float _target;
    float _current;
};

#endif
