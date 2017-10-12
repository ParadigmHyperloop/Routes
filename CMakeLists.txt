cmake_minimum_required(VERSION 3.8)
project(Routes)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp)
add_executable(Routes ${SOURCE_FILES})

target_link_libraries(Routes ${OPENCL_LIBRARIES})