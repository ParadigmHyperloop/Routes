//
//  tests.cpp
//  Routes
//
//  Created by Logan on 11/10/17.
//

#define BOOST_TEST_MODULE Routes-Test-Elevation
#include <boost/test/unit_test.hpp>
#include <routes.h>

BOOST_AUTO_TEST_CASE(test_elevation_conversions) {
    
    // Load up a fake route to get some data
    ElevationData data = ElevationData(glm::vec2(-119.001666666700030,35.001666666664143),
                                       glm::vec2(-118.5000,34.08877925439021));
    
    // Check some conversions
    BOOST_CHECK(glm::dvec2(0.0, 0.0) == data.longitudeLatitudeToMeters(data.metersToLongitudeLatitude(glm::vec2(0.0, 0.0))));
    BOOST_CHECK(glm::ivec2(0, 0)     == data.longitudeLatitudeToPixels(data.pixelsToLongitudeLatitude(glm::ivec2(0, 0))));
    
    BOOST_CHECK(glm::ivec2(0, 0)     == data.metersToPixels(glm::dvec2(0.0, 0.0)));
    BOOST_CHECK(glm::dvec2(0.0, 0.0) == data.convertPixelsToMeters(glm::ivec2(0, 0)));
    
}

BOOST_AUTO_TEST_CASE(test_elevation_sampling) {
    
    // Load up a fake route to get some data
    ElevationData data = ElevationData(glm::vec2(-119.001666666700030,35.001666666664143),
                                       glm::vec2(-118.5000,34.08877925439021));
    
    // Sample using regular GDAL data
    glm::vec3 sample_CPU = data.metersToMetersAndElevation(data.longitudeLatitudeToMeters(glm::dvec2(-118.701666666700030,34.901666666664143)));
    
    // Generate the program
    // Get stuff we need to execute a kernel on
    const boost::compute::context& ctx = Kernel::getContext();
    boost::compute::command_queue& queue = Kernel::getQueue();
    
    static const std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(
    
         __kernel void sample(__read_only image2d_t image, __global float* samples, float2 coord) {
             
              const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
             
             // Sample from given coord
             samples[0] = read_imagef(image, sampler, coord).x;
             
         }
    
    );
    
    // Figure out where to sample from
    glm::dvec2 sample_coord_g = glm::vec2(sample_CPU.x, sample_CPU.y) - data.getCroppedOriginMeters();
    glm::dvec2 size = data.getCroppedSizeMeters();
    boost::compute::float2_ sample_coord = boost::compute::float2_(sample_coord_g.x / size.x, sample_coord_g.y / size.y);
    
    boost::compute::vector<float> buffer = boost::compute::vector<float>(1, ctx);
    std::vector<float> buffer_CPU = std::vector<float>(1);

    static Kernel kernel = Kernel(source, "sample");
    kernel.setArgs(data.getOpenCLImage(), buffer.get_buffer(), sample_coord);
    kernel.execute1D(0, 1);

    boost::compute::copy(buffer.begin(), buffer.end(), buffer_CPU.begin(), queue);

    BOOST_CHECK_CLOSE(buffer_CPU[0], sample_CPU.z, 5.0);
    
    
}
