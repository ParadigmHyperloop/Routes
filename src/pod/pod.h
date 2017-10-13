//
// Created by isaac on 10/13/17.
//

#ifndef ROUTES_POD_H
#define ROUTES_POD_H

#include <math.h>

class Pod {
    public:

        /**
         * @return
         * Returns the acceleration due to gravity (g) at the Earth's surface in m/s^2
         */
        static float g()
        {return 9.81;}

        /**
         * @return
         * Returns the maximum slope grade (6%) that the track will have
         */
        float maxSlopeGrade()
        {return 0.06;}

        /**
         * @return
         * Returns the maximum depth that a hyperloop track can go underground where
         * basic excavation with suffice.
         */
        static float excavationDepth()
        {return 10.0;}

        /**
        * Void -> Double
        * Returns the minimum radius of curvature that the track may have in meters
        * This is based on the maximum speed of the hyperloop pod in m/s
        * and a max lateral acceleration of 1/2 g
        * @return
        */
        float minCurveRadius()
        {return pow (maxSpeed(), 2) / 2*g(); }

        Pod(float max_speed);

    private:
        /**
         * @return
         * Returns the maximum speed of the pod in  m/s
         */
        float maxSpeed()
        {return 339.0;}
};


#endif //ROUTES_POD_H
