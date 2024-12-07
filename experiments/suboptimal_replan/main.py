# an experiment to compare original implementation and improved implementation
import subprocess
import re
import numpy as np
import tqdm

def construct_experiment(solver_path, map_path, scen_path, output_file_name, agent_num, t):
    """ Construct the input of subprocess.check_output to perform the experiment"""
    return [solver_path, '-m', map_path, '-a', scen_path, "-o", output_file_name + ".csv", "-k", str(agent_num), '-t', str(t), "--initAlgo", "PP", "--singleAgentReplan", "SIPP", "--maxIterations",  str(100000), "--initLNS", "0"]

def construct_scen_path(scen_dir, map_name, k):
    """ Construct the path to a scen file """
    return scen_dir + map_name + "-random-" + str(k) + ".scen"

def parse_subprocess_output(res):
    nums = re.findall(r"[-+]?(?:\d*\.*\d+)", res)
    return np.asarray(list(map(float, nums)))

# parameters
agent_num = 500 # number of agents
scen_num = 10 # number of scenes
t = 20
results_file = "results"
mapf_benchmark_dir = "../../MAPF-benchmark/"
scen_dir = mapf_benchmark_dir + "mapf-scen-random/scen-random/"
solver_path1 = "../../my_code/LNS2_anytime_replan/lns"
solver_path2 = "../../my_code/better_res_table/lns"

print("Number of agents:", agent_num)
# map_names = ["Berlin_1_256", "Boston_0_256", "brc202d", "den312d", "den520d", "empty-32-32", "ht_chantry", "ht_mansion_n", "lak303d", "maze-128-128-10", "random-64-64-20", "warehouse-20-40-10-2-1"]
map_names = ["Berlin_1_256", "Boston_0_256", "brc202d", "den520d", "empty-32-32"]


# list all maps
results_orig = []
results_improved = []
for map_name in tqdm.tqdm(map_names, desc="outer", position=0):
    print("Map:", map_name)
    map_path = mapf_benchmark_dir + "mapf-map/" + map_name + ".map"
    results_orig_m = [] 
    results_improved_m = [] 

    for s in tqdm.tqdm(range(1, scen_num+1), desc="inner loop", position=1, leave=False):
        # construct scene path
        scen_path = construct_scen_path(scen_dir, map_name, s)
        # print("Scene:", scen_path)

        # construct experiment for improved
        exp = construct_experiment(solver_path1, map_path, scen_path, results_file, agent_num, t)
        try:
            res = subprocess.check_output(exp)[:-1].decode('UTF-8')
        except:
            print("Improved Solver returned error for map: ", map_name, ", scene: ", s)
            print(" ".join(map(str, exp)))
            continue
        res = parse_subprocess_output(res)
        results_improved_m.append(res)

        # construct experiment for SIPP
        exp = construct_experiment(solver_path2, map_path, scen_path, results_file, agent_num, t)
        try:
            res = subprocess.check_output(exp)[:-1].decode('UTF-8')
        except:
            print("Original Solver returned error for map: ", map_name, ", scene: ", s)
            results_orig = results_improved_m[:-1]
            continue
        res = parse_subprocess_output(res)
        results_orig_m.append(res)

    results_orig.append(results_orig_m)
    results_improved.append(results_improved_m)

results_orig = np.asarray(results_orig)
results_improved = np.asarray(results_improved)

mean_orig = np.mean(np.asarray(results_orig), axis=1)
mean_improved = np.mean(np.asarray(results_improved), axis=1)

max_orig = np.max(np.asarray(results_orig), axis=1)
max_improved = np.max(np.asarray(results_improved), axis=1)

print("Mean orig")
print(mean_orig)
print("Mean improved")
print(mean_improved)

print("Max orig")
print(max_orig)
print("Max improved")
print(max_improved)


