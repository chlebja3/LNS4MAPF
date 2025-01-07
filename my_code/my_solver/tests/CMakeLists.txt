# Find GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

# Create a test executable
add_executable(simple_tests simple_tests.cpp)

# Link GoogleTest and the MAPF library 
target_link_libraries(simple_tests PRIVATE MAPF_lib gtest_main)

# Enable CTest integration
add_test(NAME simple_tests COMMAND simple_tests)
