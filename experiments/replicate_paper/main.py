# an experiment to replicate the Experiment 5 from ADDRESS paper on den520d map

import subprocess
import re
import numpy as np

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
scen_num = 2 # number of scenes
#scen_num = 25 # number of scenes
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

# test subprocess
# res = subprocess.check_output(['echo', 'Test program output'])
# print(res[:-1].decode('UTF-8'))

# test construct scen path
# scen_path = construct_scen_path(scen_dir, map_name, 1)
# print("Scen path:", scen_path)

# test experiment lns
# print(construct_experiment(lns_path, map_path, scen_path, "a.out", agent_num, 10))

# check that lns solver works  
#./lns -m random-32-32-20.map -a random-32-32-20-random-1.scen -o test.csv -k 50 -t 60
# exp = construct_experiment(lns_path, mapf_benchmark_dir + "mapf-map/random-32-32-20.map", "random-32-32-20-random-1.scen", "test.csv", 50, 10)
# res = subprocess.check_output(exp)
# print(res[:-1].decode('UTF-8'))

# check that address solver works
# ./address -m Paris_1_256.map -a Paris_1_256-random-1.scen -o test -k 600 -t 60"
# exp = construct_experiment(address_path, mapf_benchmark_dir + "mapf-map/Paris_1_256.map", "Paris_1_256-random-1.scen", "test.csv", 600, 10)
# res = subprocess.check_output(exp)
# print(res[:-1].decode('UTF-8'))



res_lns = [] # (initial solution, final solution) or None if no solution found
res_add = []
# try various time limits
#for t in [15, 30, 60, 120]:
for t in [15, 30]:
    res_lns_t = []
    res_add_t = []
    print("Experiments for time limit:", t, "s")
    for s in range(1, scen_num+1):
        # construct scene path
        scen_path = construct_scen_path(scen_dir, map_name, s)
        print("Scene:", scen_path)

        # construct lns experiment
        exp = construct_experiment(lns_path, map_path, scen_path, results_file, agent_num, t)
        res = subprocess.check_output(exp)[:-1].decode('UTF-8')
        res = extract_solution_costs(res)
        res_lns_t.append(res)
        print("Result LNS: ", res)


        # construct address experiment
        exp = construct_experiment(address_path, map_path, scen_path, results_file, agent_num, t)
        res = subprocess.check_output(exp)[:-1].decode('UTF-8')
        res = extract_solution_costs(res)
        res_add_t.append(res)
        print("Result ADDRESS: ", res)
    res_lns.append(res_lns_t)
    res_add.append(res_add_t)

print("Results LNS:", res_lns)
print("Results ADDRESS:", res_add)

avg_lns = np.mean(np.asarray(res_lns), axis=1)
avg_add = np.mean(np.asarray(res_add), axis=1)
print("Average lns:", avg_lns)
print("Average add:", avg_add)

np.savetxt(results_file + "_LNS.txt", avg_lns)
np.savetxt(results_file + "_ADDRESS.txt", avg_add)


