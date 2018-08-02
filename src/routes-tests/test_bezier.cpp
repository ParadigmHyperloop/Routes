//
//  test_bezier.cpp
//  Routes
//
//  Created by Logan on 11/11/17.
//

#include <boost/test/unit_test.hpp>
#include <routes.h>

#define VEC_CLOSE_EQUAL(a, b, p) \
            BOOST_CHECK_CLOSE(a.x, b.x, p); \
            BOOST_CHECK_CLOSE(a.y, b.y, p); \
            BOOST_CHECK_CLOSE(a.z, b.z, p); \

BOOST_AUTO_TEST_CASE(test_bezier_CPU) {
 
    // Test evaluation of the point
    // This tests linear interpolation basically
    glm::vec3 start = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 end   = glm::vec3(1.0, 1.0, 1.0);
    
    BOOST_CHECK(Bezier::evaluateBezierCurve({start, end}, 0.5) == glm::vec3(0.5, 0.5, 0.5));
    BOOST_CHECK(Bezier::evaluateBezierCurve({start, end}, 0.2) == glm::mix(start, end, 0.2));
    BOOST_CHECK(Bezier::evaluateBezierCurve({start, end}, 0.7) == glm::mix(start, end, 0.7));
    
    // Check a quadratic curve
    glm::vec3 mid = glm::vec3(0.4, 2.1, 0.2);
    BOOST_CHECK(Bezier::evaluateBezierCurve({start, mid, end}, 0.5) == glm::vec3(0.45, 1.3, 0.35));
    
    // Check multi evaluation with a quadratic curve
    std::vector<glm::vec3> evaluated_multi = Bezier::evaluateEntireBezierCurve({start, mid, end}, 3);

    std::cout << evaluated_multi[0].x << ", " << evaluated_multi[0].y << ", " << evaluated_multi[0].z << std::endl;
    std::cout << evaluated_multi[1].x << ", " << evaluated_multi[1].y << ", " << evaluated_multi[1].z << std::endl;
    std::cout << evaluated_multi[2].x << ", " << evaluated_multi[2].y << ", " << evaluated_multi[2].z << std::endl;

    BOOST_CHECK(evaluated_multi[0] == start);
    BOOST_CHECK(evaluated_multi[1] == glm::vec3(0.45, 1.3, 0.35));
    BOOST_CHECK(evaluated_multi[2] == end);
    
}

BOOST_AUTO_TEST_CASE(test_bezier_binom) {
    
    // Check binomial coeffs because we ran into some issues with overflows
    BOOST_CHECK_EQUAL(Bezier::calcBinomialCoefficient(20, 1), 20);
    BOOST_CHECK_EQUAL(Bezier::calcBinomialCoefficient(20, 10), 184756);
    BOOST_CHECK_EQUAL(Bezier::calcBinomialCoefficient(20, 19), 20);
    BOOST_CHECK_EQUAL(Bezier::calcBinomialCoefficient(20, 20), 1);
    
    BOOST_CHECK_EQUAL(Bezier::calcBinomialCoefficient(30, 15), 155117520);
    BOOST_CHECK_EQUAL(Bezier::calcBinomialCoefficient(40, 40), 1);
    
}

BOOST_AUTO_TEST_CASE(test_bezier_GPU) {
    
    const boost::compute::context& ctx = Kernel::getContext();
    boost::compute::command_queue& queue = Kernel::getQueue();
    
    static const std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(
            
        // Evaluates the bezier curve made by the given control points and start and dest at parametric value s
        float4 evaluateBezierCurve(float4* controls, int offset, int points, float s, __global int* binomial_coeffs) {
         
             float one_minus_s = 1.0 - s;
            
             // Degree is num points - 1
             int degree = points - 1;
            
             float4 out_point = (float4)(0.0, 0.0, 0.0, 0.0);
            
             // Middle terms, iterate for num points
             for (int i = 0; i < points; i++) {
                 
                 // Evaluate for x y and z
                 float multiplier = pown(one_minus_s, degree - i) * pown(s, i);
                 
                 // We subtract one here so that we use the correct control point since i starts at 1
                 out_point += controls[i + offset] * multiplier * (float)binomial_coeffs[i];
                 
             }
            
             return out_point;
         
        }
                                                                     
         __kernel void bezier(__global float4* out, __global int* binomial_coeffs) {
             
             float4 controls[3];
             controls[0] = (float4)(0.0, 0.0, 0.0, 0.0);
             controls[1] = (float4)(0.4, 2.1, 0.2, 0.0);
             controls[2] = (float4)(1.0, 1.0, 1.0, 0.0);

             out[0] = evaluateBezierCurve(controls, 0, 3, 0.5, binomial_coeffs);
             
         }
     
    );
    
    boost::compute::vector<glm::vec4> buffer = boost::compute::vector<glm::vec4>(1, ctx);
    std::vector<glm::vec4> buffer_CPU = std::vector<glm::vec4>(1);
    
    std::vector<int> coeff_CPU = Bezier::getBinomialCoefficients(2);
    boost::compute::vector<int> coeff = boost::compute::vector<int>(coeff_CPU.size(), ctx);
    
    boost::compute::copy(coeff_CPU.begin(), coeff_CPU.end(), coeff.begin(), queue);
    
    static Kernel kernel = Kernel(source, "bezier");
    kernel.setArgs(buffer.get_buffer(), coeff.get_buffer());
    kernel.execute1D(0, 1);
    
    boost::compute::copy(buffer.begin(), buffer.end(), buffer_CPU.begin(), queue);
    
    // Computers always lie
    VEC_CLOSE_EQUAL(buffer_CPU[0], glm::vec3(0.45, 1.3, 0.35), 1)
    
}
