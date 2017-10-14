//
// Created by isaac on 10/13/17.
//

#include "pod.h"

Pod::Pod(float max_speed) {

    // Save the max speed of the pod
    _max_speed = max_speed;

}

float Pod::minCurveRadius() { return pow (_max_speed, 2.0) / 2.0 * g; }