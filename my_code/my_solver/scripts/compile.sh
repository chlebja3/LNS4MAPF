cd "$(dirname "$0")"
cd ../build
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++-18

# check whether RELEASE or DEBUG
if [ $# -eq 0 ] || [ $1 == "relwithdebinfo" ]
then
  BUILD_TYPE=RelWithDebInfo
elif [ $1 == "release" ]
then
  BUILD_TYPE=Release
elif [ $1 == "debug" ]
then
  BUILD_TYPE=Debug
fi

echo "Build type: ${BUILD_TYPE}"


cmake .. -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
cmake --build . 

