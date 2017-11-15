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
         * The control points that make up the B-Spline.
         *
         * @param knots
         * The knot vector.
         *
         * @param t
         * The parametric parameter of the B-spline curve. Should be in the range [*knots.begin(), *knots.end()].
         */
        static glm::vec3 evaluateSpline(const std::vector<glm::vec3>& controls, const std::vector<float>& knots, float t);
    
    private:
    
        /**
         * Performs De Boor's algorithm recursively to evaluate a B-Spline.
         * Algorithm can be found at https://en.wikipedia.org/wiki/De_Boor%27s_algorithm
         * There appears to be different representation of variables depending on what article you are reading. The parameters here match the ones
         * outlined in the Wikipedia article.
         *
         * @param i
         * The i value in De Boor's algorithm according to the Wikipedia article.
         *
         * @param r
         * The r value in De Boor's algorithm according to the Wikipedia article.
         *
         * @param p
         * The degree of the B-spline.
         *
         * @param controls
         * The control points that make up the B-Spline.
         *
         * @param knots
         * The knot vector.
         *
         * @param t
         * The parametric parameter of the B-spline curve. Should be in the range [*knots.begin(), *knots.end()].
         */
        static glm::vec3 deBoors(int i, int r, int p, const std::vector<glm::vec3>& controls, const std::vector<float>& knots, float t);
    
        /**
         * This determines which range created by the knots t lies in.
         *
         * @param knots
         * The knot vector.
         *
         * @param t
         * The parametric parameter of the B-spline curve. Should be in the range [*knots.begin(), *knots.end()].
         */
        static int determineK(const std::vector<float>& knots, float t);
    
};

#endif /* spline_h */
