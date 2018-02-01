//
// Created by isaac on 10/13/17.
//

#include "pod.h"

Pod::Pod(float max_speed) {

    // Save the max speed of the pod
    _max_speed = max_speed;

}

float Pod::minCurveRadius() const { return pow (_max_speed, 2.0) / g; }

float Pod::calcCentripetalAccel(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) const {

    float accel = 0.0;

    accel = pow(_max_speed, 2) / Bezier::radiusOfCurvature(p0,p1,p2);

    return accel;
}

float Pod::timeForCurve(const std::vector<glm::vec3>& points) const {

    float time = 0.0;

    std::vector<glm::vec3> interestPoints;

    // Add points whose centripetal acceleration is greater than g to a vec3
    for(int i = 1; i < points.size() - 2; i++) {
        if(calcCentripetalAccel(points[i-1], points[i], points[i+1]) > g)
            interestPoints[i] = points[i];
    }

    std::vector<float> accel_at_point;
    std::vector<float> vel_at_point;

    // The distance between the first interest point and the beginning of the route.
    int backtrack_init = 0;

    for(int round = 0; round < interestPoints.size(); round++){
        if(interestPoints[round] != glm::vec3(0.0)) {
            backtrack_init = round;
            break;
        }
    }

    // The amount to backtrack from an interest point to start slowing down
    int backtrack = backtrack_init / 2;

    // Calculate the acceleration at each point
    for(int i = 0; i < points.size() - 1; i++) {

        // Calculate the acceleration at each point and add it to a vector
        if(i == 0) {
            accel_at_point[0] = 0.0;
            vel_at_point[0] = 0.0;
        }
        else if(calcCentripetalAccel(points[i-1], points[i], points[i+1]) < g) {
            // Sets the acceleration at a point to the centripetal acceleration at a point if it does not have a
            // centripetal acceleration greater than or equal to g.
            accel_at_point[i] = calcCentripetalAccel(points[i - 1], points[i], points[i + 1]);

            // Sets the velocity at all points where the centripetal accleration is less than g to the max speed
            vel_at_point[i] = _max_speed;

        }
        else if(calcCentripetalAccel(points[i-1], points[i], points[i+1]) >= g){
            accel_at_point[i] = .9 * g;

            // Calculate the new velocity at point i if point i caused an acceleration that was greater than g.
            vel_at_point[i] = sqrt(pow(_max_speed, 2) + 2 * (-.2 * g) * (Bezier::avgDistanceBetweenPoints(points) * backtrack));

            // Calculates the velocities before an interest point
            for(int j = floor((i/2) + 1); j < i; j++)
                vel_at_point[j] = vel_at_point[i/2] - j * (.2 * g);

            // Calculates the velocities after an interest point
            for(int k = i + 1; k < 2 * i || k < (points.size() - 1); k++)
                vel_at_point[k] = vel_at_point[i] + k * (.2 * g);
        }
    }

    // Accumulates the time by multiplying the velocity at a point by the average distance between the points
    for(int t = 0; t < points.size() - 1; t++)
        time += vel_at_point[t] * Bezier::avgDistanceBetweenPoints(points);

    return time;
}
