#include <vector>
#include <algorithm>
#include "opencl/kernel.h"
#include <gdal.h>

namespace compute = boost::compute;

int test() { return 1; }

int main()
{
    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    // generate random numbers on the host
    std::vector<float> host_vector(1000000);
    std::generate(host_vector.begin(), host_vector.end(), test);

    // create vector on the device
    compute::vector<float> device_vector(1000000, ctx);
    compute::vector<float> result_vector(1000000, ctx);

    // copy data to the device
    compute::copy(
        host_vector.begin(), host_vector.end(), device_vector.begin(), queue
    );


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

    boost::shared_ptr<boost::compute::program_cache> global_cache = boost::compute::program_cache::get_global_cache(ctx);
    std::string options;
    std::string key = "__iwocl16_saxpy";

    boost::compute::program program;

    try {
         program = global_cache->get_or_build(key, options, source, ctx);
    } catch (boost::compute::opencl_error error) {

        std::cout << "Error: " << error.what() << std::endl;
        return 0;

    }

    boost::compute::kernel kernel = program.create_kernel("add"); kernel.set_args(device_vector.get_buffer(), result_vector.get_buffer());
    queue.enqueue_1d_range_kernel(kernel, 0, device_vector.size() - 1, 0);


    // copy data back to the host
    //host_vector.clear();
    //host_vector.resize(1000000);

    compute::copy(
            result_vector.begin(), result_vector.end(), host_vector.begin(), queue
    );

    std::cout << "Size " << host_vector[0] << std::endl;

    return 0;
}