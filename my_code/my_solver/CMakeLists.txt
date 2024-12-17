cmake_minimum_required(VERSION 3.16)

# Project name and version
project(simple_MAPF_solver VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set a default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# include directories
include_directories("inc")

# Add the executable
add_executable(simple_MAPF_solver src/main.cpp)

# Find Boost
find_package(Boost REQUIRED COMPONENTS program_options system filesystem)

# Find Eigen3 for PIBT
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

# add include directories
include_directories( ${Boost_INCLUDE_DIRS} )

# link
target_link_libraries(simple_MAPF_solver ${Boost_LIBRARIES} Eigen3::Eigen)
