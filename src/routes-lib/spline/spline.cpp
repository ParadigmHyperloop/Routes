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
    int p = m - n - 1;
    
    // Perform DeBoor's
    return deBoors(determineK(knots, t), p, p, controls, knots, t);
    
}

glm::vec3 Spline::deBoors(int i, int r, int p, const std::vector<glm::vec3>& controls, const std::vector<float>& knots, float t) {
    
    // d(i,0) is control point at i
    if (!r)
        return controls[i];
    
    // Perform the unoptimzied De Boor's algorithm becasue on the GPU we dont want to modify the control points
    float alpha = (t - knots[i]) / (knots[i + p + 1 - r] - knots[i]);
    return deBoors(i - 1, r - 1, p, controls, knots, t) * (1.0f - alpha) + deBoors(i, r - 1, p, controls, knots, t) * alpha;
    
}

int Spline::determineK(const std::vector<float>& knots, float t) {
    
    // Find k, the range that t falls into. Since we are using closed curves we use s to count how many times we have
    // found a value equal to t. At the end of the curve we need to subtract s so that the final point is the last control
    // point.
    int k = 0;
    int s = 0;
    
    // Loop to find the range, k is incremented by the loop
    for (; k < knots.size(); k++) {
        
        if (t == knots[k])
            s++;
        else if (t < knots[k])
            break;
        
    }
    
    // Eliminate branching with a little trickery
    return k - 1 - (int)(k != s) * s;
    
}

