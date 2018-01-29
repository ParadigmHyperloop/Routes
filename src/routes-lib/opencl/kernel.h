//
// Created by Logan on 10/12/17.
//

#ifndef ROUTES_KERNEL_H
#define ROUTES_KERNEL_H

#define BOOST_COMPUTE_DEBUG_KERNEL_COMPILATION

#include <boost/compute.hpp>
#include <fstream>
#include <glm/glm.hpp>

/** */

/**
 *  Kernel is a class that will create and retain an OpenCL kernel.
 *  Given a std::string, it will automatically compile the program.
 *  Kernel statically manages the OpenCL context.
 */
class Kernel {

    public:

        /**
         * Default constructor, compiles the program to the kernel.
         * @param program
         * The program to be compiled as a string.
         *
         * @param name
         * The name of the kernel function inside the program
         */
        Kernel(const std::string& program, const std::string& name);


        /**
         * Compiles a kernel from an external file
         *
         * @param stream
         * The stream that points to the location on disk that the kernel program should be compiled from
         *
         * @param name
         * The name of the kernel function inside the program
         */
        Kernel(std::ifstream stream, const std::string& name);

        /**
         * An already compiled program to use for this kernel.
         *
         * @param program
         * The compiled program.
         *
         * @param name
         * The name of the kernel function inside the program
         */
        Kernel(const boost::compute::program& program, const std::string& name);

        /**
         * Sets the arguments on the kernel.
         * Equivalent to calling _opencl_kernel.set_args(args)
         *
         * @tparam T
         * Variadic template parameters for multiple arguments.
         *
         * @param args
         * References to the data to be passed into the kernel.
         */
        template<class... T>
        void setArgs(T&&... args) {

            // Make sure to check if the program was valid or not
            if (_opencl_program_valid)
                _opencl_kernel.set_args(args...);
        }

        /**
        * Runs the program on the kernel using data that was already passed to the kernel.
        *
        * @param start_index
        * The index in the OpenCL program this set of work should start at.
        *
        * @param num_iterations
        * The number of times that the OpenCL program will itterate for.
        * Final index will be start_index + num_iterations - 1
        *
        * @param work_size
        * The size of each work group to be iterated over. When not specified,
        * OpenCL will do it's best to figure out what it should use.
        */
        void execute1D(size_t start_index, size_t num_iterations, size_t work_size = 0);

        /**
        * Runs the program on the kernel using data that was already passed to the kernel.
        *
        * @param start_index
        * The index in the OpenCL program this set of work should start at.
        *
        * @param num_iterations
        * The number of times that the OpenCL program will itterate for.
        * Final index will be start_index + num_iterations - 1
        *
        * @param work_size
        * The size of each work group to be iterated over. When not specified,
        * OpenCL will do it's best to figure out what it should use.
        */
        void execute2D(const glm::vec<2, size_t>& start_index,
                       const glm::vec<2, size_t>& num_iterations,
                       const glm::vec<2, size_t>& work_size = glm::vec<2, size_t>(0, 0));

        inline static const boost::compute::context& getContext() { return _opencl_context; }
        inline static boost::compute::command_queue& getQueue()   { return _opencl_queue; }

        /**
         * Gets a const reference to the program that was compiled to create this kernel.
         * This can be used to create another kernel with the same program.
         *
         * @return
         * The const reference to this kernel's program.
         */
        inline const boost::compute::program& getProgram() const { return _opencl_program; }
    
        /**
         * Gets a const reference to the current device (hopefully a GPU).
         *
         * @return
         * The const reference to the device
         *
         */
        inline static const boost::compute::device& getDevice() { return _opencl_device; }

    protected:

        /** The OpenCL program that a Kernel object should run. */
        boost::compute::program _opencl_program;

        /** The OpenCL kernel; computations are run here */
        boost::compute::kernel _opencl_kernel;

        /** A value that stores whether or not the program has been successfully compiled. */
        bool _opencl_program_valid = false;

    private:

        /**
         *  Initializes the static OpenCL device, context and queue.
         *  Should never be called by a Kernel object.
         */
        static bool initOpenCL();

        /**
         *  Dummy boolean to allow initOpenCL() during static initialization.
         *  Should never be used by a Kernel object.
         */
        static bool _is_initialized;

        /**
         *  The OpenCL compute device on which all Kernel computations are performed on.
         *  Optimally should be a GPU.
         */
        static boost::compute::device _opencl_device;

        /** The OpenCL compute context on which all Kernel computations are performed on. */
        static boost::compute::context _opencl_context;

        /** The OpenCL compute queue on which all Kernel computations are performed on. */
        static boost::compute::command_queue _opencl_queue;

        /**
        *  The OpenCL global cache utilized by all Kernel objects.
        *  This is how parameters are passed from the CPU to the compute device.
        */
        static boost::shared_ptr<boost::compute::program_cache> _global_cache;

        /**
         * Compiles the program for this kernel from the given string.
         *
         * @param program
         * The source of the kernel program
         *
         * @param name
         * The name of the kernel inside of the source
         */
        void compileProgram(const std::string& program, const std::string& name);

};


#endif //ROUTES_KERNEL_H
