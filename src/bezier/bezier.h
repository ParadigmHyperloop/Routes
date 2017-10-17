//
// Created by Logan on 10/17/17.
//

#ifndef ROUTES_BEZIER_H
#define ROUTES_BEZIER_H

#include <glm/glm.hpp>
#include <vector>
#include <map>

/** This class provides utils to evaluate a bezier curve, rather than rewrite the code in more than one class. */
class Bezier {

    public:

        /**
         * Binomial coefficients are needed for the evaluation of a bezier curve.
         * This function calculates all of them for the given degree.
         *
         * @param degree
         * The degree of the bezier curve, this is 1 - num_points
         *
         * @return
         * The complete array of binomial coefficients, should be of degree length.
         */
        static std::vector<int> getBinomailCoefficients(int degree);

        /**
         * This evaluates a bezier curve at a given point on the curve.
         *
         * @param points
         * The array of points that serve as the control points for the curve.
         *
         * @param num_points
         * The number of points contained in the points array.
         *
         * @param s
         * The parametric parameter; in other words how far along the curve to evlauate.
         * Should be in the range [0, 1].
         *
         * @return
         * The calculated point that is on the curve.
         */
        static glm::vec4 evaluateBezierCurve(glm::vec4* points, int num_points, float s);

        /**
         * A convenience for evaluating the entire bezier curve.
         *
         * @param points
         * The array of points that serve as the control points for the curve.
         *
         * @param num_points
         * The number of points contained in the points array.
         *
         * @param num_desired
         * The number of times that the curve should be evlauated and also the number of
         * points that are to be returned.
         *
         * @return
         * The evaluated points.
         */
        static std::vector<glm::vec4> evaluateEntireBezierCurve(glm::vec4* points, int num_points, int num_desired);


    private:

        /**
         * Used to calculated a binomial coefficient for the given n and i.
         *
         * @param n
         * The size of the set to choose from.
         *
         * @param i
         * The size of the subset to get.
         *
         * @return
         * The binomil coefficient.
         */
        int calcBinomialCoefficient(int n, int i);

        /**
         * Binomial coefficients don't change between two bezier curves of the same degree. Therefore
         * We save all the sets of the binomial coefficients that are calculated so we don't had to
         * do it again.
         */
        static std::map<int, std::vector<int>> binomial_coeffs;

};

#endif //ROUTES_BEZIER_H
