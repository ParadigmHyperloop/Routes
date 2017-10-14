#include <vector>
#include "genetics/genetics.h"


int main() {

    // Test elevation object
    ElevationData* data = new ElevationData("../data/imgn35w119_1.img");
    std::cout << "Min: " << data->getMinElevation() << " Max:" << data->getMaxElevation() << std::endl;
    std::cout << data->convertPixelsToMeters(glm::ivec2(data->getWidth(), 0.0)).x << std::endl;

//    std::vector<float> host_vector(data.getWidth() * data.getHeight());
//    const compute::context& ctx =   Kernel::getContext();
//    compute::command_queue& queue = Kernel::getQueue();
//
//    compute::vector<float> result_vector(host_vector.size(), ctx);
//
//
//
//    std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(
//
//            __kernel void image(__read_only image2d_t image, __global float* result, int width, int height) {
//
//                const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
//
//                size_t i = get_global_id(0);
//
//                float x = (float)(i % width);
//                float y = (float)(i / width);
//
//                for (int j = 0; j < 3700; j++)
//                    result[i] = read_imagef(image, sampler, (float2)(x, y)).x;
//
//            }
//
//    );
//
//
//    Kernel test_kernel = Kernel(source, "image");
//    test_kernel.setArgs(data.getOpenCLImage(), result_vector.get_buffer(), data.getWidth(), data.getHeight());
//
//    std::cout << "Starting kernel\n";
//    test_kernel.execute1D(0, data.getWidth() * data.getHeight());
//
//    compute::copy(result_vector.begin(), result_vector.end(), host_vector.begin(), queue);
//
//    std::cout << "Output " << host_vector[6000] << std::endl;

    Pod pod = Pod(339.0);
    Population pop = Population(200, 8, *data);

    Genetics::solve(pop, pod, glm::vec4(0.0, 0.0, 550.0, 0.0), glm::vec4(70000.0, 80000.0, 1300.0, 0.0), 100);

    delete data;

    return 0;
}