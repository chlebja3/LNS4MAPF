#!/bin/bash
# Build the headless benchmark driver for the cluster (no GUI libraries needed).
# Usage: build.sh <solver srcdir (my_code/my_solver)> <out-binary> [driver in src/experiments, default psipp_init_bench]
set -e
SRC=$(realpath "$1"); OUT=$(realpath -m "$2"); DRIVER=${3:-psipp_init_bench}
HERE=$(cd "$(dirname "$0")" && pwd)
E=$HOME/envs/mapf
OBJ=$(mktemp -d)
FLAGS="-O3 -DNDEBUG -std=c++17 -fopenmp -w -I$HERE/nfd_stub -I$SRC/inc -I$SRC/third_party_libraries -isystem $E/include"
FILES=$(ls $SRC/src/*.cpp | grep -vE "/(Visualizer|main|Experiment|experiment_utils|Computation)\.cpp$")
pids=()
for f in $FILES; do
  g++ $FLAGS -c $f -o $OBJ/$(basename $f .cpp).o &
  pids+=($!)
done
for p in "${pids[@]}"; do wait $p; done
g++ $FLAGS $SRC/src/experiments/$DRIVER.cpp $OBJ/*.o -o $OUT -L$E/lib -Wl,-rpath,$E/lib -lboost_regex -lboost_filesystem -lpthread
rm -rf $OBJ
echo built $OUT
