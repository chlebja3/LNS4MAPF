#!/bin/bash
#SBATCH -p compute
#SBATCH -c 1
#SBATCH --mem=4G
#SBATCH --time=12:00:00
#SBATCH --array=0-499
#SBATCH -o /dev/null
# Usage: sbatch run_array.sh <cmds file>
# Each line of the cmds file is a self-contained shell command writing its own result file.
CMDS=$1
N=$(wc -l < $CMDS)
for ((i=SLURM_ARRAY_TASK_ID; i<N; i+=500)); do
  sed -n "$((i+1))p" $CMDS | bash 2>/dev/null
done
