cmake_minimum_required(VERSION 3.8)
project(Routes)

set(CMAKE_CXX_STANDARD 17)

# Include all of the .hpp, .cpp and .h files in the SRC folder
FILE(GLOB_RECURSE HPPS ${CMAKE_SOURCE_DIR}/src/*.hpp)
FILE(GLOB_RECURSE HS ${CMAKE_SOURCE_DIR}/src/*.h)
FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Add the sources to the executables
add_executable(Routes ${HPPS}
                      ${HS}
                      ${SOURCES} src/pod/pod.cpp src/pod/pod.h)

# Find GDAL and link it
FIND_PACKAGE(GDAL REQUIRED)

# Find OpenCL and link it
FIND_PATH(OPENCL_INCLUDE_DIR CL/cl.h ~/NVIDIA_GPU_Computing_SDK/OpenCL/common/inc/)
FIND_LIBRARY(OPENCL_LIBRARIES OpenCL ENV LD_LIBRARY_PATH )

# Link libraries
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${GDAL_INCLUDE_DIR})

target_link_libraries(Routes ${OPENCL_LIBRARIES}
                             ${GDAL_LIBRARY})