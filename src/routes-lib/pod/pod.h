//
// Created by isaac on 10/13/17.
//

#ifndef ROUTES_POD_H
#define ROUTES_POD_H

#include <glm/vec3.hpp>
#include <math.h>
#include <vector>
#include "../bezier/bezier.h"

/** Lowercase g in physics (the gravitational acceleration constant) in meters / second^2. */
#define g 9.81

/** The maximum slope grade that should be allowed for the track. */
#define MAX_SLOPE_GRADE 0.06f

/** The maximum depth that a hyperloop track can go underground where basic excavation with suffice. */
#define EXCAVATION_DEPTH 10.0f

/** The default max speed of a hyperloop pod in meters / second. */
#define DEFAULT_POD_MAX_SPEED 339.75f

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
        * and a max lateral acceleration of g
        *
        * @return
        * The minimum radius of curvature that should be allowed for human comfort on the track.
        */
        float minCurveRadius() const;

        /**
         * Calculates the centripetal acceleration at a point in m/s^2
         *
         * @param p0
         *
         * The point before the point of interest
         * @param p1
         *
         * The point of interest
         * @param p2
         * The point after the point of interest
         *
         * @return
         * The centripetal acceleration at a point in m/s^2
         *
         */
        float calcCentripetalAccel(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2);

        /**
         * Calculates the time to travel the route in seconds
         *
         * @param points
         * The points of the bezier curve
         *
         * @return
         * The time required to travel on a given hyperloop route
         */
        float timeForCurve(const std::vector<glm::vec3>& points);

    private:

        /** The max speed in meters / second of this pod. */
        float _max_speed;
};


#endif //ROUTES_POD_H
