#!/bin/bash
# One run of the SOTA comparison: runs the solver single-threaded in a temporary directory, then validates and
# summarizes the result into one JSON file (sota_collect.py) and deletes the raw output.
# Usage: sota_run.sh <bin dir> <solver> <map> <scen> <k> <seed> <time_limit> <out json>
# Solvers: ours_psipp_rp, ours_psipp_rpd, ours_psipp_rpdc10, ours_psipp, ours_sipp, lns2, lns, balance, address, lacam3
set -u
BIN=$1; SOLVER=$2; MAP=$3; SCEN=$4; K=$5; SEED=$6; TL=$7; OUT=$8
HERE=$(cd "$(dirname "$0")" && pwd)
PY=${SOTA_PYTHON:-python3}
RUN=$(mktemp -d)
export OMP_NUM_THREADS=1
# memory limit of one run (LaCAM stores all explored configurations), a failed allocation is a failure of the run
ulimit -v $(( ${SOTA_MEM_GB:-8} * 1024 * 1024 ))
# hard limit: the solvers stop by themselves at the time limit, the preprocessing of large maps adds a little
HARD=$(( ${TL%.*} + 90 ))
LNS_COMMON="-m $MAP -a $SCEN -k $K -t $TL -o $RUN/res --stats $RUN/stats --outputPaths=$RUN/paths.txt"
case $SOLVER in
  ours_psipp_rp|ours_psipp_rpd|ours_psipp_rpdc10|ours_psipp|ours_sipp)
    timeout $HARD $BIN/anytime_bench $MAP $SCEN $K ${SOLVER#ours_} $SEED $TL $RUN/paths.txt > $RUN/out.json ;;
  lns2)  # MAPF-LNS2 defaults (PP + LNS2 repair, adaptive destroy), anytime improvement until the time limit
    timeout $HARD $BIN/lns2 $LNS_COMMON --seed $SEED --maxIterations 1000000000 > /dev/null ;;
  lns)  # MAPF-LNS (Li et al. 2021) with PP initial solution, as in the thesis (--seed added by sota_build.sh)
    timeout $HARD $BIN/lns -m $MAP -a $SCEN -k $K -t $TL -o $RUN/res.csv --stats $RUN/stats.csv \
      --outputPaths=$RUN/paths.txt --seed $SEED --initAlgo PP --neighborSize 8 --maxIterations 1000000000 > /dev/null ;;
  balance)  # BALANCE (Phan et al. 2024), settings from its README
    timeout $HARD $BIN/balance $LNS_COMMON --seed $SEED --maxIterations 1000000000 --banditAlgo=Thompson \
      --neighborCandidateSizes=5 > /dev/null ;;
  address)  # ADDRESS (2025), settings from its README
    timeout $HARD $BIN/address $LNS_COMMON --seed $SEED --maxIterations 1000000000 --destroyStrategy=RandomWalk \
      --algorithm=bernoulie --k=64 > /dev/null ;;
  lacam3)  # LaCAM3 (anytime LaCAM*), single thread
    timeout $HARD $BIN/lacam3 -m $MAP -i $SCEN -N $K -s $SEED -t ${TL%.*} -o $RUN/result.txt --no-multi-thread \
      --checkpoints-duration 0.5 > /dev/null ;;
  *) echo "unknown solver $SOLVER" >&2; exit 2 ;;
esac
RC=$?
$PY -I $HERE/sota_collect.py $SOLVER $MAP $SCEN $K $TL $RUN $RC > $OUT.tmp && mv $OUT.tmp $OUT
rm -rf $RUN
