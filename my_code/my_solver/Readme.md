# MAPF solver with interactive visualizations
This project is a multi-agent pathfinding (MAPF) solver with interactive visualizations. The solver is based on the MAPF-LNS and MAPF-LNS2 solvers. The visualizations use the SFML library and the whole project uses the C++17 standard.  

# Dependencies
SFML 2.5.1

Boost 1.71.0

nlohmann-json3-dev 3.7.3-1

For tests and benchmarks (should be fetched automatically by CMake if not installed):

google-test 1.10.0

google benchmark 1.5.0-4build1

nativefiledialog-extended (added as submodule, might be needed to run `git submodule update --init --recursive`)

sqlite3 - for experiments only

# Build
```
./scripts/compile.sh "build type" "tests"
```

There are three build types:
- debug - no optimizations, all debug information, asserts on
- relwithdebinfo - optimizations, debug information, asserts on (default)
- release - optimizations, no debug information, asserts off

The second argument specifies whether to build unit and benchmark tests. The default is ON
- tests - build all tests (default)
- notest - do not build tests

Another option is to build manually

```
cd build
cmake ..
cmake --build .
```

To build with a specific build type, specify the CMAKE_BUILD_TYPE flag (options are Debug, Release, RelWithDebInfo):
```
cmake -DCMAKE_BUILD_TYPE=Release ..
```

To build without unit and benchmark tests, specify the ENABLE_TESTS flag (default is ON):
```
cmake -ENABLE_TESTS=OFF ..
```

# Run

```
./scripts/run.sh "arguments" 
```

To see all possible arguments run:
```
./scripts/run.sh -h
```

Another script is provided for convenience to run a specific instance:
```
./scripts/run_specific_instance.sh "instance name" "optional arguments"
```

There are three prepared instances that can be run:
- dummy - simple 3x3 map with 1 agent
- empty-16-16 - empty 16x16 map with 20 agents
- den520d - den520d map from the MAPF benchmark with 500 agents

## Demo 1: Simple 3x3 map, 1 agent
```
./scripts/run_specific_instance.sh dummy
```

## Demo 2: den520d map from the MAPF benchmark with 500 agents
```
./scripts/run_specific_instance.sh den520d
```

## Run with custom settings:

```
./build/mapf_solver -m 'path to map' -a 'path to scen file' -k 'number of agents' 
```

or 

```
./scripts/run.sh -m 'path to map' -a 'path to scen file' -k 'number of agents' 
```

# Tests
## Memory tests
To test memory leaks on a specific instance of the map den520d with 500 agents, run
```
./scripts/run_valgrind.sh den520d
```

## Unit tests
To run all unit tests, run
```
./scripts/run_tests.sh
```
To run specific tests, run
```
./scripts/run_specific_test.sh "test name"
```

For example to run all SIPP related tests:
```
./scripts/run_specific_test.sh SIPP*
```

## Benchmarks
```
./scripts/run_benchmark.sh "benchmark name"
```

Available benchmarks:
- sipp_bm - benchmark of the sipp algorithm on the den520d map with 500 agents
- vector_operations_bm - benchmark of some vector operations

## Profiling
To profile the program, run
```
./scripts/profile.sh
```

It is advised to compile the program in the RelWithDebInfo build type but with asserts turned off. I had some trouble with displaying the debug info in kcachegrind when compiled with clang, so it might be needed to switch to gcc

```
export CC=/usr/bin/gcc
export CXX=/usr/bin/g++
```

# Documentation
To generate the documentation, run
```
./scripts/generate_documentations.sh
```

To view the documentation, run
```
./scripts/show_documentation.sh
```

# License
MIT

# Author
Jan Chleboun (2025)
email: chlebja3@fel.cvut.cz


