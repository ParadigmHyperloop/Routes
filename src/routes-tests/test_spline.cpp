//
//  test_spline.cpp
//  Routes
//
//  Created by Logan on 11/14/17.
//

#include <bezier/bezier.h>
#include <opencl/kernel.h>
#include <spline/spline.h>
#include <boost/test/unit_test.hpp>

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

BOOST_AUTO_TEST_CASE(test_spline_GPU) {
    
    const boost::compute::context& ctx = Kernel::getContext();
    boost::compute::command_queue& queue = Kernel::getQueue();
    
    static const std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(
            
         // Generates a knot i for a B-Spline with the given degree p and order n
         float generateKnot(int i, int p, int n) {
          
             // Pad starting zeros
             if (i < p)
                 return 0.0;
             
             // Pad ending zeros
             if (i > n + 1)
                 return (float)p;
             
             // Calculate the regular value otherwise
             float i_adj = i - p;
             return i_adj / (n - 1) * (float)p;
             
         }
                      
         // Determines the starting value of k based on the number of knots
         int determineK(__local float* knots, int knot_count, float t) {
             
             // Find k, the range that t falls into. Since we are using closed curves we use s to count how many times we have
             // found a value equal to t. At the end of the curve we need to subtract s so that the final point is the last control
             // point.
             int k = 0;
             int s = 0;
             
             // Loop to find the range, k is incremented by the loop
             for (; k < knot_count; k++) {
                 
                 if (t == knots[k])
                     s++;
                 else if (t < knots[k])
                     break;
                 
             }
             
             // Eliminate branching with a little trickery
             return k - 1 - (int)(k != s) * s;
             
         }
         
         float4 deBoors(int k, int p, float4* controls, int offset, __local float* knots, float t) {
             
             // Because we dont have dynamic memory allocations in OpenCL, we create a static buffer bigger that we should need
             // This has the side effect of limiting the max degree of a B-Spline to 19.
             // After thats done we perform step one and copy over the temporary control points
             float4 d[20];
             for (int j = 0; j <= p; j++)
                 d[j] = controls[j + k - p + offset];
             
             // Perform the optimized version of De Boor's algorithm. Taken from https://en.wikipedia.org/wiki/De_Boor%27s_algorithm#Optimizations
             for (int r = 1; r <= p; r++) {
                 
                 for (int j = p; j >= r; j--) {
                     
                     float alpha = (t - knots[j + k - p]) / (knots[j + 1 + k - r] - knots[j + k - p]);
                     d[j] = (1.0f - alpha) * d[j - 1] + alpha * d[j];
                     
                 }
                 
             }
             
             return d[p];

         }
                                                                     
         // Evaluates the bezier curve made by the given control points and start and dest at parametric value t.
         // For this function, t should be in the range [0, 1] and will be converted to [0, p]
         float4 evaluateBSpline(float4* controls, int offset, int p, float t, __local float* knots, int knot_count) {
             
             // Convert t which is in the range [0, 1] to [0,p]
             float t_new = t * (float)p;
             return deBoors(determineK(knots, knot_count, t_new), p, controls, offset, knots, t_new);
             
         }
         
         __kernel void spline(__global float4* out, __global int* binomial_coeffs, __local float* knots) {
             
             // Since B-Splines of degree n are basically bezier curve, we can reuse the test cases
             // for the bezier curve more or less
             float4 controls[3];
             controls[0] = (float4)(0.0, 0.0, 0.0, 0.0);
             controls[1] = (float4)(0.4, 2.1, 0.2, 0.0);
             controls[2] = (float4)(1.0, 1.0, 1.0, 0.0);
             
             // Get the knots, p = 2, n = 2
             for (int i = 0; i < 6; i++)
                 knots[i] = generateKnot(i, 2, 2);
             
             out[0] = evaluateBSpline(controls, 0, 2, 0.5, knots, 6);
             
         }
     
     );
    
    boost::compute::vector<glm::vec4> buffer = boost::compute::vector<glm::vec4>(1, ctx);
    std::vector<glm::vec4> buffer_CPU = std::vector<glm::vec4>(1);
    
    std::vector<int> coeff_CPU = Bezier::getBinomialCoefficients(2);
    boost::compute::vector<int> coeff = boost::compute::vector<int>(coeff_CPU.size(), ctx);
    
    boost::compute::copy(coeff_CPU.begin(), coeff_CPU.end(), coeff.begin(), queue);
    
    static Kernel kernel = Kernel(source, "spline");
    kernel.setArgs(buffer.get_buffer(), coeff.get_buffer(), boost::compute::local_buffer<float>(6));
    kernel.execute1D(0, 1);
    
    boost::compute::copy(buffer.begin(), buffer.end(), buffer_CPU.begin(), queue);
    
    // Computers always lie
    VEC_CLOSE_EQUAL(buffer_CPU[0], glm::vec3(0.45, 1.3, 0.35), 1)
    
}
