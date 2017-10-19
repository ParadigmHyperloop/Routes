//
// Created by Logan on 10/17/17.
//

#include "bezier.h"

std::map<int, std::vector<int>> Bezier::binomial_coeffs;

const std::vector<int>& Bezier::getBinomialCoefficients(int degree) {

    // We use degree + 1 because a bezier curve has degree + 1 terms
    binomial_coeffs[degree] = std::vector<int>(degree + 1);

    // Fill it
    for (int i = 0; i < degree + 1; i++)
        binomial_coeffs[degree][i] = calcBinomialCoefficient(degree, i);

    return binomial_coeffs[degree];

}

glm::vec3 Bezier::evaluateBezierCurve(const std::vector<glm::vec3>& points, float s) {

    // Get the degree of the curve and the binomials that correspond to it 
    int degree = points.size() - 1;
    const std::vector<int>& binoms = getBinomialCoefficients(degree);

    float one_minus_s = 1.0 - s;
    glm::vec3 point = glm::vec3(0.0);

    // Evaluate using the explicit definition of a bezier curve https://en.wikipedia.org/wiki/Bézier_curve#Explicit_definition
    for (int i = 0; i <  points.size(); i++) {

        float t_and_one_minus = glm::pow(one_minus_s, degree - i) * glm::pow(s, i);
	    point += points[i] * t_and_one_minus * (float)binoms[i];

    }

    return point;


}

std::vector<glm::vec3> Bezier::evaluateEntireBezierCurve(const std::vector<glm::vec3>& points, int num_desired) {

    // Get the degree of the curve and the binomials that correspond to it 
    int degree = points.size() - 1;
    const std::vector<int>& binoms = getBinomialCoefficients(degree);

    // Figure out how far along the curve each point is. We use num_desired - 1 as the divisor so that we make sure we evaluate at 1
    std::vector<glm::vec3> points_calc = std::vector<glm::vec3>(num_desired);

    for (int p = 0; p < num_desired; p++) {

         float s = (float)p / (float)(num_desired - 1);
         float one_minus_s = 1.0 - s;
         glm::vec3 point = glm::vec3(0.0);

         // Evaluate using the explicit definition of a bezier curve https://en.wikipedia.org/wiki/Bézier_curve#Explicit_definition
         for (int i = 0; i < points.size(); i++) {

             float t_and_one_minus = glm::pow(one_minus_s, degree - i) * glm::pow(s, i);
	         point += points[i] * t_and_one_minus * (float)binoms[i];

         }

         points_calc[p] = point;

    }

    return points_calc;

}

int Bezier::calcBinomialCoefficient(int n, int i) {

    // Special case to prevent divide by zero
    if (!i || n == i)
        return 1;

    // Formula taken from https://en.wikipedia.org/wiki/Binomial_coefficient#Multiplicative_formula
    int ni_falling = n;
    int i_fac = i;

    // Calculate i!
    for (int j = 1; j < i - 1; j++)
        i_fac *= (i - j);

    // Calculate n falling factorial i
    for (int j = 1; j < i; j++)
        ni_falling *= (n - j);

    return ni_falling / i_fac;

}