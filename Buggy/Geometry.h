#ifndef GEOMETRY_H__
#define GEOMETRY_H__

#include <math.h>

#define sqr(x) ((x)*(x))
#define PI 3.141592653589793238
#define rad2deg(x) ( (180.0 / PI) * (x) )
#define deg2rad(x) ( (PI / 180.0) * (x) )
#define fabs(x) ((x) >= 0 ? (x) : - (x))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define distanceSqr2D(x1, y1, x2, y2) (sqr((x1) - (x2)) + sqr((y1) - (y2)))
#define distance2D(x1, y1, x2, y2) (sqrt(distanceSqr2D(x1, y1, x2, y2)))

// TODO: Rename third quarter fix to something more meaningful
float polarAngle(float x, float y, bool thirdQuarterFix)
{
    if (x > 0)
    {
        if (thirdQuarterFix && y > 0)
            return atan(y / x) - 2 * PI;
        return atan(y / x);
    }

    if (x < 0 && y >= 0)
    {
        if (thirdQuarterFix)
            return atan(y / x) - PI;
        return atan(y / x) + PI;
    }

    if (x < 0 && y < 0)
        return atan(y / x) - PI;

    // x = 0
    if (y > 0)
        return PI / 2;

    if (y < 0)
        return PI / -2;

    // y = 0
    return 0;
}

struct Point
{
    Point()
    {}

    Point(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    float x, y, z;

    Point operator+(Point& that)
    {
        return Point(x + that.x,
                     y + that.y,
                     z + that.z);
    }

    Point operator-(Point& that)
    {
        return Point(x - that.x,
                     y - that.y,
                     z - that.z);
    }

    Point operator*(float m)
    {
        return Point(x * m,
                     y * m,
                     z * m);
    }

    void operator=(Point& that)
    {
        x = that.x;
        y = that.y;
        z = that.z;
    }

    // Workaround for an gccavr issue with reference args
    void assign(Point that)
    {
        x = that.x;
        y = that.y;
        z = that.z;
    }

    void assign(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    float maxDistance(Point& that)
    {
        float dx = fabs(x - that.x);
        float dy = fabs(y - that.y);
        float dz = fabs(z - that.z);

        return max(dz, max(dx, dy));
    }
    
    float distance(Point& that)
    {
        return sqrt(sqr(x - that.x) + sqr(y - that.y) + sqr(z - that.z));
    }
    
};

void normalize2D(Point& vector, float normalizer)
{
    vector.x = vector.x * normalizer;
    vector.y = vector.y * normalizer;
//    vector.z = vector.z * normalizer;
}

bool floatEqual(float a, float b)
{
    return fabs(a - b) < 0.00001;
}

#endif
