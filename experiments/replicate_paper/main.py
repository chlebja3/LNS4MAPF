# an experiment to replicate the Experiment 5 from ADDRESS paper on den520d map

import subprocess

def construct_experiment(solver_path, map_path, scen_path, output_file_name, agent_num, t):
    """ Construct the input of subprocess.check_output to perform the experiment"""
    if "lns" in solver_path:
        return [solver_path, '-m', map_path, '-a', scen_path, "-o", output_file_name + ".csv", "-k", str(agent_num), '-t', str(t)]
    else:
        return [solver_path, '-m', map_path, '-a', scen_path, "-o", output_file_name, "-k", str(agent_num), '-t', str(t), "--k", str(64), "--maxIterations", str(100000)]

def construct_scen_path(scen_dir, map_name, k):
    """ Construct the path to a scen file """
    return scen_dir + map_name + "-random-" + str(k) + ".scen"

# parameters
map_name = "den520d"
agent_num = 700 # number of agents
results_file = "results"
mapf_benchmark_dir = "../../MAPF-benchmark/"
scen_dir = mapf_benchmark_dir + "mapf-scen-random/scen-random/"
map_path = mapf_benchmark_dir + "mapf-map/" + map_name + ".map"
lns_path = "../../papers/MAPF-LNS/lns"
address_path = "../../papers/ADDRESS/address"

print("Number of agents:", agent_num)
print("Map:", map_name)
# print("Map path:", map_path)
# print("Scen dir:", scen_dir)

res = subprocess.check_output(['echo', 'Test program output'])
print(res[:-1].decode('UTF-8'))

scen_path = construct_scen_path(scen_dir, map_name, 1)
print("Scen path:", scen_path)

print(construct_experiment(lns_path, map_path, scen_path, "a.out", agent_num, 10))

# check that lns solver works  
#./lns -m random-32-32-20.map -a random-32-32-20-random-1.scen -o test.csv -k 50 -t 60
# exp = construct_experiment(lns_path, mapf_benchmark_dir + "mapf-map/random-32-32-20.map", "random-32-32-20-random-1.scen", "test.csv", 50, 10)
# res = subprocess.check_output(exp)
# print(res[:-1].decode('UTF-8'))

# check that address solver works
# ./address -m Paris_1_256.map -a Paris_1_256-random-1.scen -o test -k 600 -t 60"
exp = construct_experiment(address_path, mapf_benchmark_dir + "mapf-map/Paris_1_256.map", "Paris_1_256-random-1.scen", "test.csv", 600, 10)
res = subprocess.check_output(exp)
print(res[:-1].decode('UTF-8'))



# try various time limits
for t in [15, 30, 60, 120]:
    pass 

