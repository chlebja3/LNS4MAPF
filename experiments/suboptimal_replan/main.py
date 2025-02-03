# an experiment to compare original implementation and improved implementation
import subprocess
import re
import numpy as np
import tqdm

def construct_experiment(solver_path, map_path, scen_path, output_file_name, agent_num, t, w):
    """ Construct the input of subprocess.check_output to perform the experiment"""
    if w is None:
        return [solver_path, '-m', map_path, '-a', scen_path, "-o", output_file_name + ".csv", "-k", str(agent_num), '-t', str(t), "--initAlgo", "PP", "--singleAgentReplan", "SIPP", "--maxIterations",  str(100000), "--initLNS", "0"]
    else:
        return [solver_path, '-m', map_path, '-a', scen_path, "-o", output_file_name + ".csv", "-k", str(agent_num), '-t', str(t), "--initAlgo", "PP", "--singleAgentReplan", "SIPP", "--maxIterations",  str(100000), "--initLNS", "0", "--w", str(w)]

def construct_scen_path(scen_dir, map_name, k):
    """ Construct the path to a scen file """
    return scen_dir + map_name + "-random-" + str(k) + ".scen"

# parameters
agent_num = 500 # number of agents
scen_num = 20 # number of scenes
t = 20
results_file = "results"
mapf_benchmark_dir = "../../MAPF-benchmark/"
scen_dir = mapf_benchmark_dir + "mapf-scen-random/scen-random/"
solver_path2 = "../../my_code/LNS2_anytime_replan/lns"
solver_path = "../../my_code/suboptimal_replan/lns"

print("Number of agents:", agent_num)
# map_names = ["Berlin_1_256", "Boston_0_256", "brc202d", "den312d", "den520d", "empty-32-32", "ht_chantry", "ht_mansion_n", "lak303d", "maze-128-128-10", "random-64-64-20", "warehouse-20-40-10-2-1"]
map_names = ["Berlin_1_256", "Boston_0_256", "brc202d", "den520d"]


# list all maps
for map_name in tqdm.tqdm(map_names, desc="outer", position=0):
    # print("Map:", map_name)
    map_path = mapf_benchmark_dir + "mapf-map/" + map_name + ".map"
    results = [] 

    for s in tqdm.tqdm(range(1, scen_num+1), desc="inner loop", position=1, leave=False):
        # construct scene path
        scen_path = construct_scen_path(scen_dir, map_name, s)

        exp = construct_experiment(solver_path2, map_path, scen_path, results_file, agent_num, t, None)
        # print(" ".join(exp))
        # exit()
        try:
            res = subprocess.check_output(exp)[:-1].decode('UTF-8')
        except:
            print("Improved Solver returned error for map: ", map_name, ", scene: ", s)
            print(" ".join(map(str, exp)))

        for w in [1.0, 1.1, 1.5, 2, 5]:
        # for w in [1.0, 1.1]:
            # construct experiment for improved
            exp = construct_experiment(solver_path, map_path, scen_path, results_file, agent_num, t, w)
            # print(" ".join(exp))
            try:
                res = subprocess.check_output(exp)[:-1].decode('UTF-8')
            except:
                print("Improved Solver returned error for map: ", map_name, ", scene: ", s)
                print(" ".join(map(str, exp)))


