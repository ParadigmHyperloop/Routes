//
// Created by isaac on 10/13/17.
//

#ifndef ROUTES_POD_H
#define ROUTES_POD_H

#include <math.h>

class pod {
    public:

        /**
         * Void -> Double
         * Returns the acceleration due to gravity (g) at the Earth's surface in m/s^2
         * @return
         */
        static double g()
        {return 9.81;}

        /**
         * Void -> Double
         * Returns the maximum slope grade (6%) that the track will have
         * @return
         */
        double maxSlopeGrade()
        {return 0.06;}

        /**
         * Void -> Double
         * Returns the maximum speed of the pod in  m/s
         * @return
         */
        static double maxSpeed()
        {return 339.0;}

        /**
         * Void -> Double
         * Returns the minimum radius of curvature that the track may have in meters
         * This is based on the maximum speed of the hyperloop pod in m/s
         * and a max lateral acceleration of 1/2 g
         * @return
         */
        static double minCurveRadius()
        {return pow (maxSpeed(), 2) / 2*g(); }

        /**
         * Void -> Double
         * Returns the maximum depth that a hyperloop track can go underground where
         * basic excavation with suffice.
         * @return
         */
        static double excavationDepth()
        {return 10.0;}

};


#endif //ROUTES_POD_H
