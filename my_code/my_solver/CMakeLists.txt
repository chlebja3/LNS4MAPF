cmake_minimum_required(VERSION 3.16)

# enable testing if the user does not specify otherwise
option(ENABLE_TESTS "Enable tests" ON)

# Project name and version
project(simple_MAPF_solver VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# static code analysis with clang-tidy
# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")

# Set a default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# include directories
include_directories("inc")

# collect all source files
file(GLOB SOURCES "src/*.cpp" )
#file(GLOB IMGUI_SOURCES "/third_party_libraries/imgui-sfml/imgui-SFML.cpp" "/third_party_libraries/imgui/*.cpp" "/third_party_libraries/imgui/*.h" "/third_party_libraries/imgui-sfml/*.h")

# Manually specify ImGui and ImGui-SFML source files
set(IMGUI_SOURCES
    third_party_libraries/imgui/imgui.cpp
    third_party_libraries/imgui/imgui_draw.cpp
    third_party_libraries/imgui/imgui_widgets.cpp
    third_party_libraries/imgui/imgui_tables.cpp
    third_party_libraries/imgui/imgui_demo.cpp
    third_party_libraries/imgui-sfml/imgui-SFML.cpp
    third_party_libraries/imgui/backends/imgui_impl_opengl3.cpp
)
# Remove main.cpp from the list
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# turn on compile commands export to allow code completion see my include files
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# allow threads
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -pedantic")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS program_options system filesystem)

# Find Eigen3 for PIBT
find_package (Eigen3 REQUIRED NO_MODULE)

# Find SFML
find_package(SFML REQUIRED COMPONENTS graphics window system )

find_package(OpenGL REQUIRED)  # Ensure OpenGL is found

# Add include directories for ImGui and ImGui-SFML
include_directories(third_party_libraries/imgui third_party_libraries/imgui-sfml)

# add include directories
include_directories( ${Boost_INCLUDE_DIRS} )

# create a library, so that it can be used in the unit testing
add_library(MAPF_lib ${SOURCES} ${IMGUI_SOURCES} )

# Add the executable
add_executable(simple_MAPF_solver src/main.cpp)

target_include_directories(MAPF_lib PRIVATE "third_party_libraries/imgui" "third_party_libraries/imgui-sfml")

# link the libraries
target_link_libraries(MAPF_lib ${Boost_LIBRARIES} Eigen3::Eigen sfml-graphics sfml-window sfml-system OpenGL::GL)
target_link_libraries(simple_MAPF_solver MAPF_lib sfml-graphics sfml-window sfml-system OpenGL::GL )

# Add subdirectory for tests
if (${ENABLE_TESTS})
  enable_testing()
  add_subdirectory(tests)
endif()


