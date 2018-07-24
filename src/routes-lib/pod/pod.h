//
// Created by isaac on 10/13/17.
//

#ifndef ROUTES_POD_H
#define ROUTES_POD_H

#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <math.h>
#include <vector>
#include "../bezier/bezier.h"
#include <unordered_map>

/** Lowercase g in physics (the gravitational acceleration constant) in meters / second^2. */
#define g 9.81

/** The maximum slope grade that should be allowed for the track. */
#define MAX_SLOPE_GRADE 0.06f

/** The maximum depth that a hyperloop track can go underground where basic excavation with suffice. */
#define EXCAVATION_DEPTH 10.0f

/** The default max speed of a hyperloop pod in meters / second. */
#define DEFAULT_POD_MAX_SPEED 339.75f

/** The acceleration of the pod when it needs to slow down */
#define DE_ACCEL 0.5 * g

/** THe acceleration of the pod when it is starting out*/
#define BEGIN_ACCEL g

/** THe acceleration of the pod when it is nearing the end of its route*/
#define END_ACCEL g


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
    float calcCentripetalAccel(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) const;

    /**
     * Gets the velocities at each index along the route
     *
     * @return
     * vector of velocities at each point in m/s
     */
    std::vector<float> getVelocities(const std::vector<glm::vec3>& points);

    /**
     * Calculates the time to travel the route in seconds
     *
     * @param points
     * The points of the bezier curve
     *
     * @return
     * The time required to travel on a given hyperloop route in seconds
     */
    float timeForCurve(const std::vector<glm::vec3>& points);


private:

    /**
     * Calculates the initial velocities along the route without accounting for accelerations
     *
     * @param lengthMap
     * An unordered map of index in the points array to length so far along the route
     *
     * @param points
     * The points of the bezier curve for the route
     *
     * @return
     * The vector of velocities that have been calculated
     */
    std::vector<float> calcInitVelocities(std::unordered_map<int, float>& lengthMap, const std::vector<glm::vec3>& points);

    /**
     * Calculate initial accelerations, which are just the centripetal accelerations so far
     *
     * @param lengthMap
     * An unordered map of index in the points array to length so far along the route
     *
     * @param points
     * The points of the bezier curve for the route
     *
     * @param begin_accel_distance
     * The distance that the pod ramps up for
     *
     * @param end_accel_distance
     * The distance that the pod starts winding down at
     *
     * @return
     * THe vector of the initial accelerations along the route
     */
    std::vector<float> calcInitAccels(std::unordered_map<int, float>& lengthMap, const std::vector<glm::vec3>& points, float begin_accel_distance, float end_accel_distance);

    /**
     * Calculate the indices where the centripetal acceleration is greater than g
     *
     * @param points
     * The points of the bezier curve for the route
     *
     * @return
     * A vector of indices where the centrip accel is greater than g along the route
     *
     */
    std::vector<int> calcAccelsTooHigh(const std::vector<glm::vec3>& points);

    /**
     * Fix velocities based off the new accelerations
     *
     * @param vels
     * The initial velocities along the route
     *
     * @param lengthMap
     * An unordered map of index in the points array to length so far along the route
     *
     * @param accels_too_high
     * A vector of indices where the centrip accel is greater than g along the route
     *
     * @return
     * The final velocities at each point
     *
     */
    std::vector<float> fixVelocities(std::vector<float> vels, std::unordered_map<int, float>& lengthMap, std::vector<int> accels_too_high, const std::vector<glm::vec3>& points);

    /** The max speed in meters / second of this pod. */
    float _max_speed;

    // The index where winding up ends
    int _count1;

    // The index where winding down starts
    int _count2;
};


#endif //ROUTES_POD_H
