//
//  spline.h
//  Routes
//
//  Created by Logan on 11/14/17.
//

#ifndef spline_h
#define spline_h

#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <vector>

/** This class provides the utilities to evaluate a B-spline curve on the CPU */
class Spline {
    
    public:
    
        /**
         * Evaluates the B-Spline curve with the given control points and
         * knots.
         *
         * @param controls
         * The control points that make up the B-Spline
         *
         * @param knots
         * The knot vector
         *
         * @param t
         * The parametric parameter of the B-spline curve
         */
        static glm::vec3 evaluateSpline(const std::vector<glm::vec3>& controls, const std::vector<float>& knots, float t);
    
    private:
    
        /**
         * Performs De Boor's algorithm recursively to evaluate a B-Spline.
         * Algorithm can be found at https://en.wikipedia.org/wiki/De_Boor%27s_algorithm
         *
         * @param k
         * The degree of the B-spline curve.
         *
         * @param n
         * The number of control points - 1
         *
         * @param i
         * i in DeBoor's algorithm.
         *
         * @param controls
         * The control points that make up the B-Spline
         *
         * @param knots
         * The knot vector
         *
         * @param t
         * The parametric parameter of the B-spline curve
         */
        static glm::vec3 deBoors(int k, int n, int i, const std::vector<glm::vec3>& controls, const std::vector<float>& knots, float t);
    
};

#endif /* spline_h */
