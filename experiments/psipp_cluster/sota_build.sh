#!/bin/bash
# Clone the baseline solvers of the SOTA comparison at pinned commits and build them into <bin dir>.
# Usage: sota_build.sh <work dir> <bin dir>
# Boost and Eigen are searched in $SOTA_PREFIX (CMAKE_PREFIX_PATH), e.g. a conda environment on the cluster.
set -e
WORK=$(realpath -m "$1"); BIN=$(realpath -m "$2")
mkdir -p $WORK $BIN
cd $WORK
PREFIX_ARGS=""
if [ -n "${SOTA_PREFIX:-}" ]; then
  PREFIX_ARGS="-DCMAKE_PREFIX_PATH=$SOTA_PREFIX -DBOOST_ROOT=$SOTA_PREFIX -DBoost_NO_SYSTEM_PATHS=ON"
  LINK_ARGS="-DCMAKE_EXE_LINKER_FLAGS=-Wl,-rpath,$SOTA_PREFIX/lib"
fi

fetch() {  # <github repo> <dir> <commit>
  if [ ! -d $2 ]; then git clone -q --recursive https://github.com/$1 $2; fi
  git -C $2 checkout -q $3
  git -C $2 submodule update -q --init --recursive
}
fetch Jiaoyang-Li/MAPF-LNS2 MAPF-LNS2 1369823985a15944f9a339226d521f61605a6d17
fetch Jiaoyang-Li/MAPF-LNS MAPF-LNS 95785de66f8fbdb91fed871e1a56c8c563f39b1f
fetch thomyphan/anytime-mapf anytime-mapf 47bb25e9ba58b22172e1531cb26f25af8a4531e9
fetch JimyZ13/ADDRESS ADDRESS b3d35c431ce03834f95e834d87986f2f341394a1
fetch Kei18/lacam3 lacam3 1c3ecf8ade40298f16047544634e9c0c1cf3b93c

# MAPF-LNS has no option to save the paths, which we need for the independent validation: add --outputPaths
if ! grep -q outputPaths MAPF-LNS/src/driver.cpp; then
  sed -i 's|("stats", po::value<string>(), "output stats file")|("stats", po::value<string>(), "output stats file")\n\t\t("outputPaths", po::value<string>(), "output file for paths")|' MAPF-LNS/src/driver.cpp
  sed -i '0,/            lns.validateSolution();/s||            lns.validateSolution();\n        if (succ \&\& vm.count("outputPaths"))\n            lns.writePathsToFile(vm["outputPaths"].as<string>());|' MAPF-LNS/src/driver.cpp
fi

# MAPF-LNS always seeds with srand(0): add --seed (default 0, the original behaviour) for runs with several seeds
if ! grep -q '"seed"' MAPF-LNS/src/driver.cpp; then
  sed -i 's|("outputPaths", po::value<string>(), "output file for paths")|("outputPaths", po::value<string>(), "output file for paths")\n\t\t("seed", po::value<int>()->default_value(0), "Random seed")|' MAPF-LNS/src/driver.cpp
  sed -i 's|^\tsrand(0);|\tsrand(vm["seed"].as<int>());|' MAPF-LNS/src/driver.cpp
fi

# LaCAM3 optimizes the sum of loss (waits at the goal are free even when the agent leaves it later), its
# checkpoints and initial cost are therefore not the sum of costs; record the sum of costs of the current best
# solution instead (only logging changes, the search is untouched)
if ! grep -q "metrics.hpp" lacam3/lacam3/src/planner.cpp; then
  sed -i 's|#include "../include/planner.hpp"|#include "../include/planner.hpp"\n#include "../include/metrics.hpp"|' lacam3/lacam3/src/planner.cpp
  sed -i 's|checkpoints.push_back(H_goal != nullptr ? H_goal->f : CHECKPOINTS_NIL);|checkpoints.push_back(H_goal != nullptr ? get_sum_of_costs(backtrack(H_goal)) : CHECKPOINTS_NIL);|' lacam3/lacam3/src/planner.cpp
  sed -i 's|      cost_initial_solution = H->g;|      cost_initial_solution = get_sum_of_costs(backtrack(H));|' lacam3/lacam3/src/planner.cpp
  grep -c "get_sum_of_costs(backtrack" lacam3/lacam3/src/planner.cpp | grep -qx 2
fi

# the cluster has Eigen 5 (API compatible), which does not satisfy the version request 3.3 of the LNS family
for d in MAPF-LNS2 MAPF-LNS anytime-mapf ADDRESS; do
  sed -i 's|find_package (Eigen3 3.3 REQUIRED NO_MODULE)|find_package (Eigen3 REQUIRED NO_MODULE)|' $d/CMakeLists.txt
done

build() {  # <dir> <target> <out name>
  cmake -S $1 -B $1/build -DCMAKE_BUILD_TYPE=Release $PREFIX_ARGS ${LINK_ARGS:-} > $1/build.log
  make -C $1/build -j8 $2 >> $1/build.log
  cp $1/build/$2 $BIN/$3
}
build MAPF-LNS2 lns lns2
build MAPF-LNS lns lns
build anytime-mapf balance balance
build ADDRESS address address
build lacam3 main lacam3
for d in MAPF-LNS2 MAPF-LNS anytime-mapf ADDRESS lacam3; do echo "$d $(git -C $d rev-parse --short HEAD)"; done > $BIN/VERSIONS
cat $BIN/VERSIONS
ls -l $BIN
