# an experiment to compare SIPP and Space Time Astar
import subprocess
import re
import numpy as np
import tqdm

def construct_experiment(solver_path, map_path, scen_path, output_file_name, agent_num, t, sipp=0):
    """ Construct the input of subprocess.check_output to perform the experiment"""
    return [solver_path, '-m', map_path, '-a', scen_path, "-o", output_file_name + ".csv", "-k", str(agent_num), '-t', str(t), "--initAlgo", "PP", "--sipp", str(sipp), "--maxIterations",  str(100000)]

def construct_scen_path(scen_dir, map_name, k):
    """ Construct the path to a scen file """
    return scen_dir + map_name + "-random-" + str(k) + ".scen"

def parse_subprocess_output(res):
    nums = re.findall(r"[-+]?(?:\d*\.*\d+)", res)
    return np.asarray(list(map(float, nums)))

# parameters
agent_num = 500 # number of agents
scen_num = 20 # number of scenes
t = 30
results_file = "results"
mapf_benchmark_dir = "../../MAPF-benchmark/"
scen_dir = mapf_benchmark_dir + "mapf-scen-random/scen-random/"
solver_path = "../../papers/MAPF-LNS2/lns"

print("Number of agents:", agent_num)
map_names = ["Berlin_1_256", "Boston_0_256", "brc202d", "den312d", "den520d", "empty-32-32", "ht_chantry", "ht_mansion_n", "lak303d", "maze-128-128-10", "random-64-64-20", "warehouse-20-40-10-2-1"]


# list all maps
results_sipp = []
results_stas = []
for map_name in tqdm.tqdm(map_names, desc="outer", position=0):
    print("Map:", map_name)
    map_path = mapf_benchmark_dir + "mapf-map/" + map_name + ".map"
    results_sipp_m = [] 
    results_stas_m = [] 

    for s in tqdm.tqdm(range(1, scen_num+1), desc="inner loop", position=1, leave=False):
        # construct scene path
        scen_path = construct_scen_path(scen_dir, map_name, s)
        # print("Scene:", scen_path)

        # construct experiment for STAstar
        exp = construct_experiment(solver_path, map_path, scen_path, results_file, agent_num, t, sipp=0)
        try:
            res = subprocess.check_output(exp)[:-1].decode('UTF-8')
        except:
            print("ASTAR Solver returned error for map: ", map_name, ", scene: ", s)
            print(" ".join(map(str, exp)))
            continue
        res = parse_subprocess_output(res)
        results_stas_m.append(res)

        # construct experiment for SIPP
        exp = construct_experiment(solver_path, map_path, scen_path, results_file, agent_num, t, sipp=1)
        try:
            res = subprocess.check_output(exp)[:-1].decode('UTF-8')
        except:
            print("SIPP Solver returned error for map: ", map_name, ", scene: ", s)
            results_stas_m = results_stas_m[:-1]
            continue
        res = parse_subprocess_output(res)
        results_sipp_m.append(res)

    results_sipp.append(results_sipp_m)
    results_stas.append(results_stas_m)

results_sipp = np.asarray(results_sipp)
results_stas = np.asarray(results_stas)

mean_sipp = np.mean(np.asarray(results_sipp), axis=1)
mean_stas = np.mean(np.asarray(results_stas), axis=1)

max_sipp = np.max(np.asarray(results_sipp), axis=1)
max_stas = np.max(np.asarray(results_stas), axis=1)

np.savetxt(results_file + "_mean_SIPP.txt", mean_sipp.reshape((len(map_names), 5)))
np.savetxt(results_file + "_mean_STAS.txt", mean_stas.reshape((len(map_names), 5)))
np.savetxt(results_file + "_max_SIPP.txt", max_sipp.reshape((len(map_names), 5)))
np.savetxt(results_file + "_max_STAS.txt", max_stas.reshape((len(map_names), 5)))
