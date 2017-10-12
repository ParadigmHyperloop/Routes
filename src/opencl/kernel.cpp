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

    std::cout << "Hello world from kernel!";

    return true;

}

Kernel::Kernel(const std::string& program) {



}

Kernel::Kernel(const boost::compute::program& program) {


}

void Kernel::execute1D(size_t start_index, size_t num_iterations, size_t work_size) {


}