cd "$(dirname "$0")"
cd ../build
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++-18
cmake .. -DENABLE_TESTS=ON
cmake --build . 

