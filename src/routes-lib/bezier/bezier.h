//
// Created by Logan on 10/17/17.
//

#ifndef ROUTES_BEZIER_H
#define ROUTES_BEZIER_H

#include <glm/glm.hpp>
#include <iostream>
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
         * The complete array of binomial coefficients, should be of degree length + 1.
         */
        static const std::vector<int>& getBinomialCoefficients(int degree);

        /**
         * This evaluates a bezier curve at a given point on the curve.
         *
         * @param points
         * The array of points that serve as the control points for the curve.
         *
         * @param s
         * The parametric parameter; in other words how far along the curve to evaluate.
         * Should be in the range [0, 1].
         *
         * @return
         * The calculated point that is on the curve.
         */
        static glm::vec3 evaluateBezierCurve(const std::vector<glm::vec3>& points, float s);

        /**
         * A convenience for evaluating the entire bezier curve.
         *
         * @param points
         * The array of points that serve as the control points for the curve.
         *
         * @param num_desired
         * The number of times that the curve should be evaluated and also the number of
         * points that are to be returned.
         *
         * @return
         * num_desired points that are on the bezier curve.
         */
        static std::vector<glm::vec3> evaluateEntireBezierCurve(const std::vector<glm::vec3>& points, int num_desired);
    
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
         * The binomial coefficient.
         */
        static int calcBinomialCoefficient(int n, int i);

        /**
         * Calculates the aproximate length of a bezier curve. Calculates using the sum of the straight line distances of the given points.
         *
         * @param points
         * Points from evaluateBezierCurve or evaluateEntireBezierCurve that are on the curve.
         *
         * @return
         * The aproximate length of the bezier curve in whatever unit the given points were in.
         *
         */
        static float bezierLength(const std::vector<glm::vec3>& points);

    private:

        /**
         * This function is what actually does the evaluation of the bezier curve.
         * The resulting point is a point that is on the bezier curve.
         *
         * @param out_point
         * A reference who's content will be set to the evaluated point. Should start out as glm::vec3(0.0, 0.0, 0.0).
         *
         * @param s
         * The parametric parameter; in other words how far along the curve to evaluate.
         * Should be in the range [0, 1].
         *
         * @param degree
         * The degree of the bezier curve. Should be controls.size() - 1.
         *
         * @param controls
         * The control points of the curve that should be evaluated.
         *
         * @param binoms
         * The binomial coefficients for the curve of the given degree.
         *
         */
         static void doEvaluate(glm::vec3& out_point, float s, int degree, const std::vector<glm::vec3>& controls, const std::vector<int>& binoms);

        /**
         * Binomial coefficients don't change between two bezier curves of the same degree. Therefore
         * We save all the sets of the binomial coefficients that are calculated so we don't had to
         * do it again.
         */
        static std::map<int, std::vector<int>> binomial_coeffs;

};

#endif //ROUTES_BEZIER_H
