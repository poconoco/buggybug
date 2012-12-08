#ifndef SIMPLE_MOVEMENTS_H__
#define SIMPLE_MOVEMENTS_H__


class SimpleMovements
{
public:
    SimpleMovements(Leg* legs, int N)
        : _legs(legs)
        , _N(N)
    {}
  
    float walk(int steps, Point direction, float startProgress, bool (*pContinue)() = NULL)
    {
        float p = startProgress;
        for (int i = 0; i < steps; i++)
        {
            for(; p <= 2; /*p += 0.025*/)
            {
                float progress; 
                float height1;
                float height2;
                if (p < 1) 
                {
                    progress = p;
                    height1 = 0;
                    height2 = direction.z * (0.5 - fabs(0.5 - p));
                }
                else 
                {
                    progress = 1 - (p - 1);
                    height1 = direction.z * (0.5 - fabs(1.5 - p));
                    height2 = 0;
                }
              
                Point group1(- direction.x / 2 + (direction.x - progress * direction.x),
                             - direction.y / 2 + (direction.y - progress * direction.y),
                             height1);
                Point group2(- direction.x / 2 + progress * direction.x,
                             - direction.y / 2 + progress * direction.y,
                             height2);
    
                if (_legs[0].getCurrentRelative().maxDistance(group1) > 10)
                    smoothTo(group1, 0);
                if (_legs[1].getCurrentRelative().maxDistance(group2) > 10)
                    smoothTo(group2, 1);
                        
                for (int li = 0; li < _N; li+=2)
                {
                    _legs[li].reachRelativeToDefault(group1);                  
                    _legs[li + 1].reachRelativeToDefault(group2);                  
                }
    
                delay(1);
                if (pContinue != NULL && ! pContinue())
                    return p;
                    
                p += 0.025 /*+ 0.04 * (0.5 - fabs(0.5 - progress))*/;
            }
    
            if (p > 2)
                p = 0;        
        }
        
        return p;
    }
    
    float rotate(int steps, float clockwise, float startProgress, bool (*pContinue)() = NULL)
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
    
                for (int li = 0; li < _N; ++li)
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
                    
                p += 0.025 /*+ 0.00 * (0.5 - fabs(0.5 - progress))*/;
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
        Point relative[_N];
        Point currentPositions[_N];
        for (int i = legGroup; i < _N; i += 2)
        {
            currentPositions[i] = _legs[i].getCurrentPos();
            relative[i].assign((_legs[i].getDefaultPos() + to) - _legs[i].getCurrentPos());
        }
      
        for(float p = 0; p <= 1; p += 0.03)
        {
            for (int li = legGroup; li < _N; li += 2)
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
    const int _N;
};

#endif



