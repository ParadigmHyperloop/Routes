//
// Created by isaac on 10/13/17.
//

#include "pod.h"

Pod::Pod(float max_speed) {

    // Save the max speed of the pod
    _max_speed = max_speed;

}

float Pod::minCurveRadius() const { return pow (_max_speed, 2.0) / g; }

float  Pod::timeForCurve (float length){

    float t = 0.0;

    // t is in seconds
    t = _max_speed / length;

    return t;
}