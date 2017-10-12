cmake_minimum_required(VERSION 3.8)
project(Routes)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp)
add_executable(Routes ${SOURCE_FILES})

FIND_PATH(OPENCL_INCLUDE_DIR CL/cl.h ~/NVIDIA_GPU_Computing_SDK/OpenCL/common/inc/ )
FIND_LIBRARY(OPENCL_LIBRARIES OpenCL ENV LD_LIBRARY_PATH )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Routes ${OPENCL_LIBRARIES})