#ifndef MARK1_CONFIG_H__
#define MARK1_CONFIG_H__

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

        rightLegs[0].configureServoDirections(-1, -1,  1, false);
        leftLegs [0].configureServoDirections(-1,  1, -1, true);

        rightLegs[1].configureServoDirections(-1, -1,  1, false);
        leftLegs [1].configureServoDirections(-1,  1, -1, true);

        rightLegs[2].configureServoDirections(-1, -1,  1, false);
        leftLegs [2].configureServoDirections(-1,  1, -1, true);

        for (int i = 0; i < 3; i++)
        {
            rightLegs[i].configureFemur(-26, 12, 46.5, deg2rad(10));
            leftLegs [i].configureFemur(-26, 12, 46.5, deg2rad(10));

            rightLegs[i].configureTibia(58, deg2rad(-70));
            leftLegs [i].configureTibia(58, deg2rad(-70));
        }

        rightLegs[0].configureCoxa( 34,  65, deg2rad(          20),  10);
        leftLegs [0].configureCoxa(-34,  65, deg2rad(- (180 + 20)), -10);

        rightLegs[1].configureCoxa( 52,   0, deg2rad(-         20),  10);
        leftLegs [1].configureCoxa(-52,   0, deg2rad(- (180 - 20)), -10);

        rightLegs[2].configureCoxa( 34, -65, deg2rad(-         63),  10);
        leftLegs [2].configureCoxa(-34, -65, deg2rad(- (180 - 63)), -10);

        rightLegs[0].configureDefault(Point( 60, 130, -70), true);
        leftLegs [0].configureDefault(Point(-60, 130, -70), true);

        rightLegs[1].configureDefault(Point( 110, -10, -65), true);
        leftLegs [1].configureDefault(Point(-110, -10, -70), true);

        rightLegs[2].configureDefault(Point( 70, -120, -70), true);
        leftLegs [2].configureDefault(Point(-70, -120, -70), true);

        // Fine tuning
        rightLegs[0].tuneRestAngles(PI / 2,
                                    PI / 2 + deg2rad(10),
                                    PI / 2);

        leftLegs[0].tuneRestAngles(PI / 2,
                                   PI / 2 - deg2rad(5),
                                   PI / 2);

        leftLegs[1].tuneRestAngles(PI / 2,
                                   PI / 2,
                                   PI / 2 - deg2rad(10));

        rightLegs[1].tuneRestAngles(PI / 2,
                                    PI / 2,
                                    PI / 2 + deg2rad(20));


        leftLegs[2].tuneRestAngles(PI / 2,
                                   PI / 2 - deg2rad(10),
                                   PI / 2);

    }

};

#endif
