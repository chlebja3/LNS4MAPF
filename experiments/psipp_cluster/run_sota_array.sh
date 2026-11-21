#!/bin/bash
#SBATCH -p compute
#SBATCH -c 1
#SBATCH --mem=10G
#SBATCH --time=12:00:00
#SBATCH --array=0-499
#SBATCH -o /dev/null
# Usage: sbatch run_sota_array.sh <cmds file>   (SOTA comparison, one run is limited to 8 GB by sota_run.sh)
# Each line of the cmds file is a self-contained shell command writing its own result file. Task i runs lines i, i+n,
# i+2n, ... (n = number of array tasks); the cmds file is re-read for every line.
# Other node types: sbatch -p gpu --exclude=... --array=0-<n-1> run_sota_array.sh <cmds file>
CMDS=$1
export SOTA_PYTHON=${SOTA_PYTHON:-$HOME/miniconda3/bin/python}
N=$(wc -l < $CMDS)
for ((i=SLURM_ARRAY_TASK_ID; i<N; i+=SLURM_ARRAY_TASK_COUNT)); do
  sed -n "$((i+1))p" $CMDS | bash 2>/dev/null
done
