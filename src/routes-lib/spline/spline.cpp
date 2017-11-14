//
//  spline.cpp
//  Routes
//
//  Created by Logan on 11/14/17.
//

#include "spline.h"

glm::vec3 Spline::evaluateSpline(const std::vector<glm::vec3>& controls, const std::vector<float>& knots, float t) {
    
    // Determine n and the order of the curve
    int n = controls.size() - 1;
    int m = knots.size() - 1;
    int k = m - n - 1;
    
    // Perform DeBoor's
    return deBoors(k, n, k, controls, knots, t);
    
}

glm::vec3 Spline::deBoors(int k, int n, int i, const std::vector<glm::vec3>& controls, const std::vector<float>& knots, float t) {
    
    // d(i,0) is control point at i
    if (!k)
        return controls[i];
    
    // Perform the unoptimzied De Boor's algorithm becasue on the GPU we dont want to modify the control points
    float alpha = (t - knots[i]) / (knots[i + n + 1 - k] - knots[i]);
    return deBoors(k - 1, n, i - 1, controls, knots, t) * (1.0f - alpha) + deBoors(k - 1, n, i, controls, knots, t) * alpha;
    
}

