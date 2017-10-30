cmake_minimum_required(VERSION 3.8)
project(Routes)

set(CMAKE_CXX_STANDARD 17)

# Include all of the .hpp, .cpp and .h files in the SRC folder
FILE(GLOB_RECURSE HPPS ${CMAKE_SOURCE_DIR}/src/*.hpp)
FILE(GLOB_RECURSE HS ${CMAKE_SOURCE_DIR}/src/*.h)
FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

FILE(GLOB_RECURSE LIBRARIES ${CMAKE_SOURCE_DIR}/lib/*.a)

# Add the sources to the executables
add_executable(Routes ${HPPS}
                      ${HS}
                      ${SOURCES})

IF (WIN32)

  set(GDAL_LIBRARY C:/MinGW/msys/1.0/local/lib/libgdal.a C:/MinGW/lib/libwsock32.a C:/MinGW/lib/libws2_32.a C:/MinGW/lib/libpsapi.a)
  set(GDAL_INCLUDE_DIR C:/MinGW/msys/1.0/local/include)

  find_package(OpenCL REQUIRED)
  include_directories(${OpenCL_INCLUDE_DIRS})

ELSE()

  # Find GDAL
  FIND_PACKAGE(GDAL REQUIRED)

  # Find OpenCL and link it
  FIND_PATH(OPENCL_INCLUDE_DIR CL/cl.h ~/NVIDIA_GPU_Computing_SDK/OpenCL/common/inc/)
  FIND_LIBRARY(OPENCL_LIBRARIES OpenCL ENV LD_LIBRARY_PATH )

ENDIF()

# Include stuff
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${GDAL_INCLUDE_DIR})

IF (WIN32)

  message(STATUS ${GDAL_LIBRARY})
  target_link_libraries(Routes ${OpenCL_LIBRARIES}
                               ${GDAL_LIBRARY}
                               ${LIBRARIES})

ELSE()

  target_link_libraries(Routes ${OPENCL_LIBRARIES}
                               ${GDAL_LIBRARY}
                               ${LIBRARIES})


ENDIF()