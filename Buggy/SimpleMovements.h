#ifndef SIMPLE_MOVEMENTS_H__
#define SIMPLE_MOVEMENTS_H__

class SimpleMovements
{
public:
    SimpleMovements(Leg* legs, int legCount, Leg* mandibles, int mandibleCount)
        : _legs(legs)
        , _legCount(legCount)
        , _mandibles(mandibles)
        , _mandibleCount(mandibleCount)
    {
        //_defaultPositions = new Point[_legCount];
        //_defaultMandiblePositions = new Point[_mandibleCount];

        _speedFactor = 0.025;
    }

    ~SimpleMovements()
    {
        ..delete _defaultPositions;
        //delete _defaultMandiblePositions;
    }

    void rememberDefault()
    {
        for (byte i = 0; i < _legCount; ++i)
            _defaultPositions[i] = _legs[i].getDefaultPos();

        for (byte i = 0; i < _mandibleCount; ++i)
            _defaultMandiblePositions[i] = _mandibles[i].getDefaultPos();

        _linearShift.assignZero();
    }

    void shiftAbsolute(Point& absolute, float pitch, float roll, float yaw)
    {
        for (byte i = 0; i < _legCount; ++i)
            _legs[i].delayReach();

        for (byte i = 0; i < _legCount; ++i)
            _legs[i].shiftDefault(_defaultPositions[i] + absolute);

        shiftPitch(pitch);
        shiftRoll(roll);
        shiftYaw(yaw);

        for (byte i = 0; i < _legCount; ++i)
            _legs[i].commitDelayedReach();
    }

    void mandiblesReach(Point right, Point left)
    {
        _mandibles[0].shiftDefault(_defaultMandiblePositions[0] + right);
        _mandibles[1].shiftDefault(_defaultMandiblePositions[1] + left);
    }

    void shift(Point& delta)
    {
        for (byte i = 0; i < _legCount; ++i)
            _legs[i].shiftDefaultRelative(delta);

        // TODO: Do we need to store shift in _linearShift?
    }

    void shiftPitch(float angleDelta)
    {
        float sinval = sin(angleDelta);
        float cosval = cos(angleDelta);

        for (byte i = 0; i < _legCount; ++i)
        {
            Point currDef = _legs[i].getDefaultPos();
            Point newDef;
            newDef.x = currDef.x;
            newDef.y = currDef.y * cosval - currDef.z * sinval;
            newDef.z = currDef.y * sinval + currDef.z * cosval;

            _legs[i].shiftDefault(newDef);
        }
    }

    void shiftRoll(float angleDelta)
    {
        float sinval = sin(angleDelta);
        float cosval = cos(angleDelta);

        for (byte i = 0; i < _legCount; ++i)
        {
            Point currDef = _legs[i].getDefaultPos();
            Point newDef;
            newDef.x = currDef.x * cosval - currDef.z * sinval;
            newDef.y = currDef.y;
            newDef.z = currDef.x * sinval + currDef.z * cosval;

            _legs[i].shiftDefault(newDef);
        }
    }

    void shiftYaw(float angleDelta)
    {
        float sinval = sin(angleDelta);
        float cosval = cos(angleDelta);

        for (byte i = 0; i < _legCount; ++i)
        {
            Point currDef = _legs[i].getDefaultPos();
            Point newDef;
            newDef.x = currDef.x * cosval - currDef.y * sinval;
            newDef.y = currDef.x * sinval + currDef.y * cosval;
            newDef.z = currDef.z;

            _legs[i].shiftDefault(newDef);
        }
    }

    void shiftReset()
    {
        const int steps = 20;
        Point currDefs[_legCount];

        for (byte i = 0; i < _legCount; ++i)
            currDefs[i] = _legs[i].getDefaultPos();

        for (byte si = 0; si < steps; ++si)
        {
            for (byte i = 0; i < _legCount; ++i)
            {
                Point step = (_defaultPositions[i] - currDefs[i]) / steps;
                _legs[i].shiftDefaultRelative(step);
            }

            delay(2);
        }

        // Prevent default migration
        for (byte i = 0; i < _legCount; ++i)
            _legs[i].shiftDefault(_defaultPositions[i]);
    }

    float rotate(int steps, float clockwise, float startProgress = 0, bool (*pContinue)() = NULL)
    {
        float p = startProgress;
        for (int i = 0; i < steps; i++)
        {
            for(; p <= 2; /*p += 0.025*/)
            {
                float progress;
                float h[2]; // height
                float s[2]; // sine
                float c[2]; // cosine
                const float angle = clockwise * (PI / 500);

                if (p < 1)
                {
                    progress = p;
                    h[0] = 0;
                    h[1] = 50 * (0.5 - fabs(0.5 - p));
                    s[0] = sin(angle);
                    c[0] = cos(angle);
                    s[1] = sin(- angle);
                    c[1] = cos(- angle);
                }
                else
                {
                    progress = 1 - (p - 1);
                    h[0] = 50 * (0.5 - fabs(1.5 - p));
                    h[1] = 0;
                    s[0] = sin(- angle);
                    c[0] = cos(- angle);
                    s[1] = sin(angle);
                    c[1] = cos(angle);
                }

                for (int li = 0; li < _legCount; ++li)
                {
                    // li - leg index, gi - group index
                    int gi = li % 2;
                    Point pNew;
                    Point pCurr = _legs[li].getCurrentPos();

                    pNew.x = pCurr.x * c[gi] - pCurr.y * s[gi];
                    pNew.y = pCurr.x * s[gi] + pCurr.y * c[gi];

                    // Calc default pos
                    pNew.x -= _legs[li].getDefaultPos().x;
                    pNew.y -= _legs[li].getDefaultPos().y;
                    pNew.z = h[gi];

                    _legs[li].reachRelativeToDefault(pNew);
                }

                delay(1);
                if (pContinue != NULL && ! pContinue())
                    return p;

                p += _speedFactor /*+ 0.00 * (0.5 - fabs(0.5 - progress))*/;
            }

            if (p > 2)
                p = 0;
        }

        return p;
    }

    void smoothTo(Point& to)
    {
        smoothTo(to, 0);
        smoothTo(to, 1);
    }

    // Relative to default!
    void smoothTo(Point& to, int legGroup)
    {
        Point relative[_legCount];
        Point currentPositions[_legCount];
        for (int i = legGroup; i < _legCount; i += 2)
        {
            currentPositions[i] = _legs[i].getCurrentPos();
            relative[i].assign((_legs[i].getDefaultPos() + to) - _legs[i].getCurrentPos());
        }

        for(float p = 0; p <= 1; p += 0.03)
        {
            for (int li = legGroup; li < _legCount; li += 2)
            {
                Point currSubStep = relative[li] * p;
                Point currStep = currentPositions[li] + currSubStep;
                currStep.z = _legs[li].getDefaultPos().z + to.z + 50 * (0.5 - fabs(0.5 - p));
                _legs[li].reachAbsolute(currStep);
            }

            delay(5);
        }
    }

private:

    Leg* _legs;
    const int _legCount;
    Leg* _mandibles;
    const int _mandibleCount;

    // FIXME
    Point _defaultPositions[6];
    Point _defaultMandiblePositions[2];

    Point _linearShift;

    float _speedFactor;
};

#endif



