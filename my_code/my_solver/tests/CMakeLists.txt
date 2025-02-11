set(CMAKE_PREFIX_PATH "/usr/local")
include(FetchContent)

# Find GoogleTest
find_package(GTest QUIET)
# fetch google test if not installed globally
if (NOT GTest_FOUND)
  message(STATUS "Could not find googletest, fetching.")
  FetchContent_Declare(
    GTest 
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2 # latest release
    )
FetchContent_MakeAvailable(googletest)
endif()

# Find GoogleBenchmark
find_package(benchmark QUIET)
if (NOT benchmark_FOUND)
message(STATUS "Could not find google benchmark, fetching.")
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.1 # Use the latest stable release
  )
FetchContent_MakeAvailable(benchmark)
endif()

# Create a unit test executable
add_executable(simple_tests simple_tests.cpp utils_tests.cpp astar_tests.cpp)

# Link GoogleTest and the MAPF library
target_link_libraries(simple_tests PRIVATE MAPF_lib GTest::GTest GTest::Main gmock)

# Create a benchmark tests
add_executable(benchmark_tests benchmark_tests.cpp)
target_link_libraries(benchmark_tests PRIVATE MAPF_lib benchmark::benchmark benchmark::benchmark_main)

# Enable CTest integration
add_test(NAME simple_tests COMMAND simple_tests)
