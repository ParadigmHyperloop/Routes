cmake_minimum_required(VERSION 3.8)
project(Routes)

set(CMAKE_CXX_STANDARD 17)

# Include all of the .hpp, .cpp and .h files in the SRC folder
FILE(GLOB_RECURSE HS ${CMAKE_SOURCE_DIR}/src/routes-lib/*.h*)
FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/routes-lib/*.cpp)

FILE(GLOB_RECURSE LIBRARIES ${CMAKE_SOURCE_DIR}/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX})
message(WARNING ${LIBRARIES})

# Add the sources to the executables
add_library(Routes STATIC ${HS}
                          ${SOURCES})

IF (WIN32)

  set(GDAL_LIBRARY C:/warmerda/bld/lib/gdal_i.lib)
  set(GDAL_INCLUDE_DIR C:/warmerda/bld/include/)

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
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/include
                           ${GDAL_INCLUDE_DIR})

add_custom_target(copy)

IF (WIN32)

  message(STATUS ${GDAL_LIBRARY})
  target_link_libraries(Routes ${OpenCL_LIBRARIES}
                               ${GDAL_LIBRARY}
                               ${LIBRARIES})

  add_custom_command(TARGET copy PRE_BUILD
                     COMMAND ${CMAKE_COMMAND} -E
                     copy C:/warmerda/bld/bin/gdal202.dll
                     ${CMAKE_BINARY_DIR}/gdal202.dll)

ELSE()

  target_link_libraries(Routes ${OPENCL_LIBRARIES}
                               ${GDAL_LIBRARY}
                               ${LIBRARIES}
                               -lpthread)

ENDIF()

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
target_link_libraries(Routes-Exec ${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}Routes${CMAKE_STATIC_LIBRARY_SUFFIX})

# Link in the other libraries
IF (WIN32)

  message(WARNING ${GDAL_LIBRARY})
  target_link_libraries(Routes-Exec ${OpenCL_LIBRARIES}
                                    ${GDAL_LIBRARY}
                                    ${LIBRARIES})

ELSE()

  target_link_libraries(Routes-Exec ${OPENCL_LIBRARIES}
                                    ${GDAL_LIBRARY}
                                    ${LIBRARIES}
                                    -lpthread)

ENDIF()

# Server executable
FILE(GLOB_RECURSE HS ${CMAKE_SOURCE_DIR}/src/routes-server/*.h*)
FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/routes-server/*.cpp)

add_executable(Routes-Server ${HS}
                             ${SOURCES})

add_dependencies(Routes-Server Routes)

# Build it with the library
include_directories(Routes-Server ${CMAKE_BINARY_DIR}/include)
target_link_libraries(Routes-Server ${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}Routes${CMAKE_STATIC_LIBRARY_SUFFIX})

# Link in the other libraries
IF (WIN32)

  message(STATUS ${GDAL_LIBRARY})
  target_link_libraries(Routes-Server ${OpenCL_LIBRARIES}
                                      ${GDAL_LIBRARY}
                                      ${LIBRARIES})

ELSE()

  target_link_libraries(Routes-Server ${OPENCL_LIBRARIES}
                                      ${GDAL_LIBRARY}
                                      ${LIBRARIES}
                                      -lpthread)

ENDIF()

# Create a test suite
enable_testing()

FILE(GLOB_RECURSE HS ${CMAKE_SOURCE_DIR}/src/routes-tests/*.h*)
FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/routes-tests/*.cpp)

add_dependencies(Routes-Tests Routes)

# Build it with the library
include_directories(Routes-Tests ${CMAKE_BINARY_DIR}/include)
target_link_libraries(Routes-Tests ${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}Routes${CMAKE_STATIC_LIBRARY_SUFFIX})

# Link in the other libraries
IF (WIN32)

  message(STATUS ${GDAL_LIBRARY})
  target_link_libraries(Routes-Tests ${OpenCL_LIBRARIES}
                                      ${GDAL_LIBRARY}
                                      ${LIBRARIES})

ELSE()

  target_link_libraries(Routes-Tests ${OPENCL_LIBRARIES}
                                      ${GDAL_LIBRARY}
                                      ${LIBRARIES}
                                      -lpthread)

ENDIF()

add_test(Routes-TestSuite Routes-Tests)

# Add the check target because the built in test one is a pain
# Taken from https://stackoverflow.com/questions/5709914/using-cmake-how-do-i-get-verbose-output-from-ctest/31124523#31124523
add_custom_target(check 
        COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS Routes-Tests)

# Fix the Xcode build so that everything is put into the binary directory
set_target_properties(Routes PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(Routes PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set_target_properties(Routes PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

set_target_properties(Routes-Server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(Routes-Server PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set_target_properties(Routes-Server PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

set_target_properties(Routes-Exec PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(Routes-Exec PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set_target_properties(Routes-Exec PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

set_target_properties(Routes-Tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(Routes-Tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set_target_properties(Routes-Tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

