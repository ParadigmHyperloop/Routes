cmake_minimum_required(VERSION 3.8)
project(Routes)

set(CMAKE_CXX_STANDARD 17)

# Include all of the .hpp, .cpp and .h files in the SRC folder
FILE(GLOB_RECURSE HS ${CMAKE_SOURCE_DIR}/src/routes-lib/*.h*)
FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/routes-lib/*.cpp)

FILE(GLOB_RECURSE LIBRARIES ${CMAKE_SOURCE_DIR}/lib/*.a)

# Add the sources to the executables
add_library(Routes STATIC ${HS}
			                 ${SOURCES})

set_target_properties(Routes PROPERTIES PUBLIC_HEADER ${HPPS} ${HS})

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

target_link_libraries(Routes ${OpenCL_LIBRARIES}
                                ${GDAL_LIBRARY}
                                ${LIBRARIES})

add_custom_target(copy)

foreach(Header ${HS})

  string(REPLACE ${CMAKE_SOURCE_DIR}/src/routes-lib "" REPLACED ${Header})
  string(CONCAT TO_WRITE ${CMAKE_BINARY_DIR}/include ${REPLACED})

  message(STATUS ${TO_WRITE})

  add_custom_command(TARGET copy PRE_BUILD
                     COMMAND ${CMAKE_COMMAND} -E
                     copy ${Header} ${TO_WRITE})

endforeach()

add_dependencies(Routes copy)

# Routes executable
FILE(GLOB_RECURSE HS ${CMAKE_SOURCE_DIR}/src/routes-exec/*.h*)
FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/routes-exec/*.cpp)

add_executable(Routes-Exec ${HS}
                           ${SOURCES})

add_dependencies(Routes-Exec Routes)

# Build it with the library
include_directories(Routes-Exec ${CMAKE_BINARY_DIR}/include)
target_link_libraries(Routes-Exec ${CMAKE_BINARY_DIR}/libRoutes.a)

# Link in the other libraries
target_link_libraries(Routes-Exec ${OpenCL_LIBRARIES}
                                  ${GDAL_LIBRARY}
                                  ${LIBRARIES})

