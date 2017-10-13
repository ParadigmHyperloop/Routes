#include <vector>
#include <algorithm>
#include "opencl/kernel.h"

namespace compute = boost::compute;

int test() { return 1; }

int main() {

    // generate random numbers on the host
    std::vector<float> host_vector(1000000);
    std::generate(host_vector.begin(), host_vector.end(), test);

    const compute::context& ctx =   Kernel::getContext();
    compute::command_queue& queue = Kernel::getQueue();

    // create vector on the device
    compute::vector<float> device_vector(1000000, ctx);
    compute::vector<float> result_vector(1000000, ctx);

    // copy data to the device
    compute::copy(host_vector.begin(), host_vector.end(), device_vector.begin(), queue);


    std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(

            kernel void add(global float* values, global float* results) {

                int cache[10];

                for (int i = 1; i < 10; i++) {

                    cache[i] = cache[i - 1] + 1;

                }

                size_t index = get_global_id(0);

                results[index] = cache[9];

            }

    );


    Kernel test_kernel = Kernel(source, "add");
    test_kernel.setArgs(device_vector.get_buffer(), result_vector.get_buffer());
    test_kernel.execute1D(0, device_vector.size() - 1);

    compute::copy(result_vector.begin(), result_vector.end(), host_vector.begin(), queue);

    std::cout << "Output " << host_vector[0] << std::endl;

    return 0;
}