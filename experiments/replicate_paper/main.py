# an experiment to replicate the Experiment 5 from ADDRESS paper on den520d map

import subprocess
import re
import numpy as np
import tqdm

def construct_experiment(solver_path, map_path, scen_path, output_file_name, agent_num, t):
    """ Construct the input of subprocess.check_output to perform the experiment"""
    if "lns" in solver_path:
        return [solver_path, '-m', map_path, '-a', scen_path, "-o", output_file_name + ".csv", "-k", str(agent_num), '-t', str(t), "--initAlgo", "PP"]
    else:
        return [solver_path, '-m', map_path, '-a', scen_path, "-o", output_file_name, "-k", str(agent_num), '-t', str(t), "--k", str(32), "--maxIterations", str(1000000), "--banditAlgo", "Thompson", "--algorithm", "topk-epsilon", "--epsilon", "0.5"]

def construct_scen_path(scen_dir, map_name, k):
    """ Construct the path to a scen file """
    return scen_dir + map_name + "-random-" + str(k) + ".scen"

def extract_solution_costs(res):
    sol_idx = re.search("solution cost =", res)
    if sol_idx is None:
        print("LNS did not find solution")
        return None
    else:
        sol_idx = sol_idx.span()[-1]
    res = res[sol_idx+1:]
    nums = re.findall(r'\d+', res)
    return int(nums[1]), int(nums[0])


# parameters
map_name = "den520d"
agent_num = 700 # number of agents
scen_num = 20 # number of scenes
results_file = "results"
mapf_benchmark_dir = "../../MAPF-benchmark/"
scen_dir = mapf_benchmark_dir + "mapf-scen-random/scen-random/"
map_path = mapf_benchmark_dir + "mapf-map/" + map_name + ".map"
lns_path = "../../papers/MAPF-LNS/lns"
address_path = "../../papers/ADDRESS/address"

print("Number of agents:", agent_num)
print("Map:", map_name)

res_lns = [] # (initial solution, final solution) or None if no solution found
res_add = []

# try various time limits
for t in tqdm.tqdm([15, 30, 60, 120], desc=" outer", position=0):
#for t in tqdm.tqdm([120], desc=" outer", position=0):
    res_lns_t = []
    res_add_t = []
    # print("Experiments for time limit:", t, "s")
    for s in tqdm.tqdm(range(1, scen_num+1), desc=" inner loop", position=1, leave=False):
        # construct scene path
        scen_path = construct_scen_path(scen_dir, map_name, s)
        # print("Scene:", scen_path)

        # construct lns experiment
        exp = construct_experiment(lns_path, map_path, scen_path, results_file, agent_num, t)
        res = subprocess.check_output(exp)[:-1].decode('UTF-8')
        res = extract_solution_costs(res)
        res_lns_t.append(res)
        # print("Result LNS: ", res)


        # construct address experiment
        exp = construct_experiment(address_path, map_path, scen_path, results_file, agent_num, t)
        res = subprocess.check_output(exp)[:-1].decode('UTF-8')
        res = extract_solution_costs(res)
        res_add_t.append(res)
        # print("Result ADDRESS: ", res)
    res_lns.append(res_lns_t)
    res_add.append(res_add_t)

avg_lns = np.mean(np.asarray(res_lns), axis=1)
avg_add = np.mean(np.asarray(res_add), axis=1)
print("Average lns:", avg_lns)
print("Average add:", avg_add)

# np.savetxt(results_file + "_LNS.txt", avg_lns)
# np.savetxt(results_file + "_ADDRESS.txt", avg_add)


