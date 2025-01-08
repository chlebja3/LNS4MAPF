# Find GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2 # latest release
)

# Find GoogleBenchmark
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.1 # Use the latest stable release
)
FetchContent_MakeAvailable(googletest benchmark)

# Create a unit test executable
add_executable(simple_tests simple_tests.cpp)

# Link GoogleTest and the MAPF library 
target_link_libraries(simple_tests PRIVATE MAPF_lib gtest_main)

# Create a benchmark tests 
add_executable(benchmark_tests benchmark_tests.cpp)
target_link_libraries(benchmark_tests PRIVATE MAPF_lib benchmark::benchmark benchmark::benchmark_main)

# Enable CTest integration
add_test(NAME simple_tests COMMAND simple_tests)
