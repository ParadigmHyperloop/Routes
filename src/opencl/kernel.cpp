//
// Created by Logan on 10/12/17.
//

#include "kernel.h"

boost::compute::device                           Kernel::_opencl_device;
boost::compute::context                          Kernel::_opencl_context;
boost::compute::command_queue                    Kernel::_opencl_queue;
boost::shared_ptr<boost::compute::program_cache> Kernel::_global_cache;

bool Kernel::_is_initialized = Kernel::initOpenCL();

bool Kernel::initOpenCL() {

    // Get the compute device
    _opencl_device = boost::compute::system::default_device();

    // Create a context on the device
    _opencl_context = boost::compute::context(_opencl_device);

    // Create a command queue for the context on the device
    _opencl_queue = boost::compute::command_queue(_opencl_context, _opencl_device);

    // Get the cache
    _global_cache = boost::compute::program_cache::get_global_cache(_opencl_context);

    return true;

}

Kernel::Kernel(const std::string& program, const std::string name) {

    // Compile the program
    // Check if there was an exeption
    try {

        _opencl_program =  _global_cache->get_or_build("__routes" + name, "", program, _opencl_context);
        _opencl_program_valid = true;

    } catch (boost::compute::opencl_error error) {

        // Let the user know there was an error.
        // Boost compute doesn't seem to return much useful information as to why it failed, so just print there was an error.
        // Also make sure that we remember that this program is invalid.
        std::cout << "Program failed to compile: " << name << std::endl;
        _opencl_program_valid = false;
        return;

    }

    // Create a kernel with the program
    _opencl_kernel = _opencl_program.create_kernel(name);

}

Kernel::Kernel(const boost::compute::program& program, const std::string name) {

    // We already have the program so just tell it to create a new kernel
    _opencl_program = program;
    _opencl_kernel = _opencl_program.create_kernel(name);

}

void Kernel::execute1D(size_t start_index, size_t num_iterations, size_t work_size) {

    // Add a work order onto the kernel with the parameters that were given
    if (_opencl_program_valid)
        _opencl_queue.enqueue_1d_range_kernel(_opencl_kernel, start_index, num_iterations, work_size);

}