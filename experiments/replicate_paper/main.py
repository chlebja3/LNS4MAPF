# an experiment to replicate the Experiment 5 from ADDRESS paper on den520d map

import subprocess

# parameters
map_name = "den520d"
m = 700 # number of agents
mapf_benchmark_dir = "../../MAPF-benchmark/"
scen_dir = mapf_benchmark_dir + "mapf-scen-random/scen-random/"
map_path = mapf_benchmark_dir + "mapf-map/" + map_name + ".map"

print("Number of agents:", m)
print("Map:", map_name)
# print("Map path:", map_path)
# print("Scen dir:", scen_dir)

res = subprocess.check_output(['echo', 'Test program output'])
print(res[:-1].decode('UTF-8'))

# try various time limits
for t in [15, 30, 60, 120]:
    pass 

