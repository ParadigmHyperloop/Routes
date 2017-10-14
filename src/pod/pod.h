//
// Created by isaac on 10/13/17.
//

#ifndef ROUTES_POD_H
#define ROUTES_POD_H

#include <math.h>

/** Lowercase g in physics (the gravitational acceleration constant) in meters / second^2. */
#define g 9.81

/** The maximum slope grade that should be allowed for the track. */
#define MAX_SLOPE_GRADE 0.06

/** The maximum depth that a hyperloop track can go underground where basic excavation with suffice. */
#define EXCAVATION_DEPTH 10.0

/** The default max speed of a hyperloop pod in meters / second. */
#define DEFAULT_POD_MAX_SPEED 339.0

class Pod {

    public:

        /**
         * Creates a new Pod object with the given max speed.
         *
         * @param max_speed
         * The max speed of the hyperloop pod in meters / second.
         */
        Pod(float max_speed = DEFAULT_POD_MAX_SPEED);

        /**
        * Calculates the minimum radius of curvature that the track may have in meters.
        * This is based on the maximum speed of the hyperloop pod in meters / second
        * and a max lateral acceleration of 1/2 g
        *
        * @return
        * The minimum radius of curvature that should be allowed for human comfort on the track.
        */
        float minCurveRadius();

    private:

        /** The max speed in meters / second of this pod. */
        float _max_speed;
};


#endif //ROUTES_POD_H
