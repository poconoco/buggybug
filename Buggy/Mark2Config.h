#ifndef MARK1_CONFIG_H__
#define MARK1_CONFIG_H__

class MandibleConfiguration
{
public:

    static void apply(Leg* mandibles, int N)
    {
        if (N != 2) // Only 2 mandibles supported for now
            return;

        Leg* rightMandible = mandibles;
        Leg* leftMandible  = mandibles + 1;

        rightMandible->setPins(50, 49, 48);
        leftMandible ->setPins(25, 26, 27);

        rightMandible->configureServoDirections(-1, -1,  1, false);
        leftMandible ->configureServoDirections(-1,  1, -1, true);

        /* configureFemur(float fStartZOffset,
         *                float fStartFarOffset,
         *                float fLength,
         *                float fStartAngle)
         */
        rightMandible->configureFemur(-26, 12, 46.5, deg2rad(10));
        leftMandible ->configureFemur(-26, 12, 46.5, deg2rad(10));

        /* configureTibia(float tLenght,
         *                float tStartAngle)
         */
        rightMandible->configureTibia(58, deg2rad(-70));
        leftMandible ->configureTibia(58, deg2rad(-70));

        /*
         * configureCoxa(float cStartX,
         *               float cStartY,
         *               float cStartAngle,
         *               float cFemurOffset)
         */
        rightMandible->configureCoxa( 30,  -20, deg2rad(         70),   10);
        leftMandible ->configureCoxa(-30,  -20, deg2rad(- (180 + 70)), -10);

        rightMandible->configureDefault(Point( 30, 0, -70), true);
        leftMandible ->configureDefault(Point(-30, 0, -70), true);
    }
};

class LegConfiguration
{
public:

    // NOTE: legs are attached after calling this method but are not moved
    static void apply(Leg* legs, int N)
    {
        Leg* rightLegs = legs;
        Leg* leftLegs = legs + N / 2;

        rightLegs[0].setPins(46, 47, 39);
        rightLegs[1].setPins(40, 41, 42);
        rightLegs[2].setPins(43, 44, 45);

        leftLegs[0].setPins(29, 28, 36);
        leftLegs[1].setPins(35, 34, 33);
        leftLegs[2].setPins(32, 31, 30);

        /*
         * configureServoDirections(float cServoDirection,
         *                          float fServoDirection,
         *                          float tServoDirection,
         *                          bool thirdQuarterFix)
         */

        rightLegs[0].configureServoDirections(1, -1,  1, false);
        leftLegs [0].configureServoDirections(1,  1, -1, true);

        rightLegs[1].configureServoDirections(1, -1,  1, false);
        leftLegs [1].configureServoDirections(1,  1, -1, true);

        rightLegs[2].configureServoDirections(1, -1,  1, false);
        leftLegs [2].configureServoDirections(1,  1, -1, true);

        for (int i = 0; i < 3; i++)
        {
            /* configureFemur(float fStartZOffset,
             *                float fStartFarOffset,
             *                float fLength,
             *                float fStartAngle)
             */
            rightLegs[i].configureFemur(-26, 3, 32, deg2rad(0));
            leftLegs [i].configureFemur(-26, 3, 32, deg2rad(0));

            /* configureTibia(float tLenght,
             *                float tStartAngle)
             */
            rightLegs[i].configureTibia(61, deg2rad(-90));
            leftLegs [i].configureTibia(61, deg2rad(-90));
        }

        /*
         * configureCoxa(float cStartX,
         *               float cStartY,
         *               float cStartAngle,
         *               float cFemurOffset)
         */
        rightLegs[0].configureCoxa( 23,  55, deg2rad(         54),   5);
        leftLegs [0].configureCoxa(-23,  55, deg2rad(- (180 + 54)), -5);

        rightLegs[1].configureCoxa( 28,   2, deg2rad(-        0),   5);
        leftLegs [1].configureCoxa(-28,   3, deg2rad(- (180 - 0)), -5);

        rightLegs[2].configureCoxa( 24, -38, deg2rad(-        34),   5);
        leftLegs [2].configureCoxa(-24, -38, deg2rad(- (180 - 34)), -5);

        /*
         * configureDefault(Point def, bool move)
         */
        rightLegs[0].configureDefault(Point( 45, 90, -90), true);
        leftLegs [0].configureDefault(Point(-45, 90, -90), true);

        rightLegs[1].configureDefault(Point( 70, 10, -90), true);
        leftLegs [1].configureDefault(Point(-70, 10, -90), true);

        rightLegs[2].configureDefault(Point( 60, -60, -90), true);
        leftLegs [2].configureDefault(Point(-60, -60, -90), true);

        /*
         * tuneRestAngles(float cServoRestAngle,
         *                float fServoRestAngle,
         *                float tServoRestAngle)
         * +/- deg2rad(10)
         */
        rightLegs[0].tuneRestAngles(PI / 2,
                                    PI / 2 - deg2rad(5),
                                    PI / 2);

        leftLegs[0].tuneRestAngles(PI / 2,
                                   PI / 2 + deg2rad(15),
                                   PI / 2);

        rightLegs[1].tuneRestAngles(PI / 2,
                                    PI / 2 + deg2rad(10),
                                    PI / 2);

        leftLegs[1].tuneRestAngles(PI / 2,
                                   PI / 2 + deg2rad(10),
                                   PI / 2 + deg2rad(5));

        rightLegs[2].tuneRestAngles(PI / 2,
                                    PI / 2 + deg2rad(20),
                                    PI / 2);

        leftLegs[2].tuneRestAngles(PI / 2,
                                   PI / 2 + deg2rad(15),
                                   PI / 2);

    }

};

#endif
