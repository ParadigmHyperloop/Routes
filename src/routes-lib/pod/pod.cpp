//
// Created by isaac on 10/13/17.
//


#include "pod.h"

Pod::Pod(float max_speed) {

    // Save the max speed of the pod
    _max_speed = max_speed;

}

float Pod::minCurveRadius() const { return pow (_max_speed, 2.0) / g_CONST; }

float Pod::calcCentripetalAccel(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) const {

    float accel = 0.0;

    accel = pow(_max_speed, 2) / Bezier::radiusOfCurvature(p0,p1,p2);

    return accel;
}

std::vector<float> Pod::getVelocities(const std::vector<glm::vec3>& points) {

    //map from index to length of track at that index
    std::unordered_map<int, float> lengthMap = Bezier::bezierLengthMap(points);
    /*
     * This will also initialize the accelerations along the track. At the beginning and end of the track, the acceleration is
     * determined by the accel needed to get the pod up to speed. Between these points, the acceleration is the
     * centripetal acceleration at the point. In the future, centripetal accel should be accounted for in the beginning
     * and end of the route.
     */


    std::vector<float> vels(calcInitVelocities(lengthMap, points));

    //convert the indices for winding up and ramping down to a point in meters on the curve
    float begin_accel_distance = lengthMap[_count1];
    float end_accel_distance = lengthMap[_count2];

    std::vector<float> accels(calcInitAccels(lengthMap, points, begin_accel_distance, end_accel_distance));

    // This is a vector of the indices in the points and accels array where the acceleration is larger than g_CONST.
    std::vector<int> accels_too_high(calcAccelsTooHigh(points));

    std::vector<float> finalVels(fixVelocities(vels, lengthMap, accels_too_high, points));

    return finalVels;
}

//TODO: Account for going vertical movement
float Pod::timeForCurve(const std::vector<glm::vec3>& points) {

    std::vector<float> vels = getVelocities(points);
    std::unordered_map<int, float> lengthMap = Bezier::bezierLengthMap(points);

    //We now calculate the time for the pod to traverse the route
    float time = 0.0;
    float dist = 0.0;

    //Calculate times.
    for (int i = 1; i < vels.size() - 1 ; i++) {
        dist = lengthMap[i+1] - lengthMap[i];
        time += (dist / vels[i]);
    }

    return time;

}

std::vector<float> Pod::calcInitVelocities(std::unordered_map<int, float>& lengthMap, const std::vector<glm::vec3>& points) {

    //initialize velocity vector, the first value should be 0
    std::vector<float> vels;
    vels.push_back(0);

    //used for getting the last index that the pod is ramping up at
    float newVel1 = 0.0;

    //Calculate velocities for ramping up, and fill the rest of the route with DEFAULT_POD_MAX_SPEED
    for (int i = 1; i < points.size() - 1; i++) {
        if (vels[i - 1] < DEFAULT_POD_MAX_SPEED) {
            newVel1 = glm::sqrt(glm::pow(vels[i - 1], 2) + (2 * BEGIN_ACCEL * (lengthMap[i] - lengthMap[i - 1])));
            vels.push_back(newVel1);
        } else {
            vels.push_back(DEFAULT_POD_MAX_SPEED);
        }
    }

    //Get the index where you stop ramping up
    int count1 = 0;
    for (int i = 0; i < points.size(); i++) {
        if (vels[i] < DEFAULT_POD_MAX_SPEED) {
            count1 += 1;
        } else {
            count1 += 0;
        }
    }

    count1 += -1;

    int accel_upto_index = count1;

    //used for getting the first index that the pod is winding down at
    float newVel2 = 0.0;

    //set all values after ramping up to 0 for velocity so that we can work backwards in next loop
    for (int i = accel_upto_index; i < points.size(); i++) {
        vels[i] = 0.0;
    }

    //calculate velocities during the winding down phase. Set all values in between
    //to max vel
    for (int i = points.size() - 2; i > accel_upto_index - 1; i--) {
        if (vels[i + 1] < DEFAULT_POD_MAX_SPEED) {
            newVel2 = glm::sqrt(glm::pow(vels[i+1], 2) + (2 * END_ACCEL * (lengthMap[i+1] - lengthMap[i])));
            vels[i] = newVel2;
        } else {
            vels[i] = DEFAULT_POD_MAX_SPEED;
        }
    }

    //get the index where you start winding down
    int count2 = 0;
    for (int i = points.size() - 1; i > accel_upto_index - 1; i--) {
        if (vels[i] < DEFAULT_POD_MAX_SPEED) {
            count2 += 1;
        } else {
            count1 += 0;
        }
    }

    count2 += -1;


    count2 = points.size() - count2;

    _count1 = count1-1;
    _count2 = count2+1;

    return vels;

}

std::vector<float> Pod::calcInitAccels(std::unordered_map<int, float>& lengthMap, const std::vector<glm::vec3>& points, float begin_accel_distance, float end_accel_distance) {

    std::vector<float> accels;

    //fill in the accelerations of the curve, use the centrip accel when in middle of curve
    for (int i = 0; i < points.size() - 1; i++) {

        if (lengthMap[i] <= begin_accel_distance) {
            accels.push_back(BEGIN_ACCEL);
        } else if (lengthMap[i] >= end_accel_distance) {
            accels.push_back(END_ACCEL * -1);
        } else if (lengthMap[i] > begin_accel_distance && lengthMap[i] < end_accel_distance) {
            accels.push_back(calcCentripetalAccel(points[i - 1], points[i], points[i + 1]));
        }
    }

    return accels;

}

std::vector<int> Pod::calcAccelsTooHigh(const std::vector<glm::vec3>& points) {
    // This is a vector of the indices in the points and accels array where the acceleration is larger than g_CONST.
    std::vector<int> accels_too_high;

    // Now we need to find all the points where the acceleration is larger than "g_CONST". For now we will ignore the
    // beginning and the end of the route.
    // ignore first and last points
    for (int i = _count1 + 1; i < _count2; i++) {
        float centrip = calcCentripetalAccel(points[i - 1], points[i], points[i + 1]);
        if (centrip > g_CONST) {
            accels_too_high.push_back(i);
        }
    }

    return accels_too_high;
}

std::vector<float> Pod::fixVelocities(std::vector<float> vels, std::unordered_map<int, float>& lengthMap, std::vector<int> accels_too_high, const std::vector<glm::vec3>& points) {
    // Now we must go to all these points, using the velocity, and bringing it down to the velocity that it must be
    // in order to make the centripetal acceleration at a good level.
    for (int i = _count1 + 1; i < accels_too_high.size(); i++) {
        float radiusAtPoint = Bezier::radiusOfCurvature(points[accels_too_high[i] - 1],
                                                        points[accels_too_high[i]],
                                                        points[accels_too_high[i] + 1]);
        float targetVel = glm::sqrt(g_CONST * radiusAtPoint);
        float distanceToAccel = (_max_speed - targetVel) / (2 * DE_ACCEL);
        float distanceFromStart = lengthMap[accels_too_high[i]];
        float distanceToStartAccel = distanceFromStart - distanceToAccel;
        float startIndex = 0.0;

        //get index where slow down needs to start
        for (int a = 0; a < accels_too_high[i]; a++) {
            startIndex = lengthMap[a];
            if (startIndex > distanceToStartAccel) {
                a = accels_too_high[i];
            }
        }

        // Correct the velocities. After this the accelerations and velocities are done.
        for (int j = startIndex; j < accels_too_high[i]; j++) {
            if (lengthMap[j] > (distanceFromStart - distanceToAccel)) {
                vels[j] = glm::sqrt(glm::pow(vels[j-1], 2) + (2 * -1 * DE_ACCEL * (lengthMap[i] - lengthMap[i-1])));
            }
        }
    }

    return vels;
}