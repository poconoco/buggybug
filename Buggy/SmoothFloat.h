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

    float getCurrent()
    {
        return _current;
    }

    float getCurrent(float stepDeltaOrig)
    {
        float stepDelta;
        const float dist = abs(_current - _target);
        if (dist < stepDeltaOrig * 5)
            stepDelta = stepDeltaOrig * 0.2;
        else if (dist < stepDeltaOrig * 2)
            stepDelta = stepDeltaOrig * 0.1;
        else stepDelta = stepDeltaOrig;
      
        if (_current >= _target)
            _current = max(_target, _current - stepDelta);
        else
            _current = min(_target, _current + stepDelta);

        return _current;
    }
    
private:

    float _target;
    float _current;
};

#endif
