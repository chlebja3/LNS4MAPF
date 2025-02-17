set(CMAKE_PREFIX_PATH "/usr/local")
include(FetchContent)

# Find GoogleTest
find_package(GTest)
# fetch google test if not installed globally
if (NOT GTest_FOUND)
  message(STATUS "Could not find googletest, fetching.")
  FetchContent_Declare(
    GTest 
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2 # latest release
    )
    FetchContent_MakeAvailable(GTest)
endif()

# Find GoogleBenchmark
find_package(benchmark)
if (NOT benchmark_FOUND)
message(STATUS "Could not find google benchmark, fetching.")
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.1 # Use the latest stable release
  )
FetchContent_MakeAvailable(benchmark)
endif()

file(GLOB SOURCES "src/*.cpp" )
# remove benchmark sources from the list
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark_tests.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/path_conversion_benchmark.cpp")

# Create a unit test executable
add_executable(unit_tests ${SOURCES})

# add includes 
include_directories("inc")

# Link GoogleTest and the MAPF library
target_link_libraries(unit_tests PRIVATE MAPF_lib GTest::gtest_main gmock)

# Create a benchmark tests
add_executable(benchmark_tests src/benchmark_tests.cpp src/test_utils.cpp)
target_link_libraries(benchmark_tests PRIVATE MAPF_lib benchmark::benchmark benchmark::benchmark_main)


# Create a benchmark for path conversion
add_executable(path_conversion_benchmark src/path_conversion_benchmark.cpp src/test_utils.cpp)
target_link_libraries(path_conversion_benchmark PRIVATE MAPF_lib benchmark::benchmark benchmark::benchmark_main)

# Enable CTest integration
add_test(NAME unit_tests COMMAND unit_tests)
