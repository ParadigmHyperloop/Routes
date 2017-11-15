//
//  test_spline.cpp
//  Routes
//
//  Created by Logan on 11/14/17.
//

#include <bezier/bezier.h>
#include <spline/spline.h>
#include <boost/test/unit_test.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#define VEC_CLOSE_EQUAL(a, b, p) \
            BOOST_CHECK_CLOSE(a.x, b.x, p); \
            BOOST_CHECK_CLOSE(a.y, b.y, p); \
            BOOST_CHECK_CLOSE(a.z, b.z, p); \

BOOST_AUTO_TEST_CASE(test_spline_CPU) {
    
    // When a spline has degree n, it is essentially a bezier curve
    // Exploit this for some easy tests
    glm::vec3 start = glm::dvec3(0.0, 0.0, 0.0);
    glm::vec3 mid   = glm::dvec3(0.4, 2.1, 0.2);
    glm::vec3 end   = glm::dvec3(1.0, 1.0, 1.0);
    
    std::vector<glm::vec3> controls = {start, mid, end};
    std::vector<float> knots = {0.0, 0.0, 0.0, 2.0, 2.0, 2.0};
    
    // Make sure that the start and end are actually on the curve
    BOOST_CHECK(Spline::evaluateSpline(controls, knots, 0.0) == start);
    
    std::cout << glm::to_string(Spline::evaluateSpline(controls, knots, 2.0)) << std::endl;
    BOOST_CHECK(Spline::evaluateSpline(controls, knots, 2.0) == end);

    // Validate that a curve with n = k is a bezier curve.
    VEC_CLOSE_EQUAL(Spline::evaluateSpline(controls, knots, 1.0),  glm::dvec3(0.45, 1.3, 0.35),                  1);
    VEC_CLOSE_EQUAL(Spline::evaluateSpline(controls, knots, 0.25), Bezier::evaluateBezierCurve(controls, 0.125), 1);
    VEC_CLOSE_EQUAL(Spline::evaluateSpline(controls, knots, 1.0),  Bezier::evaluateBezierCurve(controls, 0.5),   1);
    VEC_CLOSE_EQUAL(Spline::evaluateSpline(controls, knots, 1.5),  Bezier::evaluateBezierCurve(controls, 0.75),  1);
    
    // Test for non-bezier like test.
    // These tests were derrived from the example to the library TinySpline which can be found at
    // https://github.com/msteinbeck/tinyspline
    controls = {glm::vec3(-1.75, -1.0, 0.0),
                glm::vec3(-1.5, -0.5, 0.0),
                glm::vec3(-1.5, 0.0, 0.0),
                glm::vec3(-1.25, 0.5, 0.0),
                glm::vec3(-0.75, 0.75, 0.0),
                glm::vec3( 0.0, 0.5, 0.0),
                glm::vec3( 0.5, 0.0, 0.0)};
    
    knots = {0, 0, 0, 0, 0.25, 0.5, 0.75, 1, 1, 1, 1};
    VEC_CLOSE_EQUAL(Spline::evaluateSpline(controls, knots, 0.4),  glm::vec3(-1.33833, 0.288333, 0.0),    1);
    VEC_CLOSE_EQUAL(Spline::evaluateSpline(controls, knots, 0.6),  glm::vec3(-1.03433, 0.584333, 0.0),    1);
    VEC_CLOSE_EQUAL(Spline::evaluateSpline(controls, knots, 0.9),  glm::vec3( -0.0413335, 0.437333, 0.0), 1);
    VEC_CLOSE_EQUAL(Spline::evaluateSpline(controls, knots, 1.0),  glm::vec3(0.5, 0.0, 0.0),              1);
    
}

