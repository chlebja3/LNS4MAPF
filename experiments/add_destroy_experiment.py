import csv
import sqlite3
from collections import defaultdict
import json

def read_file(name):
    lines = []
    with open(name, mode ='r') as file:
      csvFile = csv.reader(file)
      for line in csvFile:
          lines.append(line)
    return lines

file_res = read_file("results_destroy_lns2.csv")

col_names = file_res[0]
file_res = file_res[1:]

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()
counts = defaultdict(int)
added = 0
skipped = 0
algo_name = "SIPP_suboptimal_w_1,00"

for experiment in file_res:
# check whether the experiment is already in the database
    instance_name = experiment[17]
    agent_num = int(experiment[22])
    neighborhood_size = int(experiment[23])
    idx1 = len("../MAPF-benchmark/mapf-scen-random/scen-random/")
    idx2 = instance_name.find(".scen")
    instance_name_stripped = instance_name[idx1:idx2]
    idx3 = instance_name_stripped.find("-random")
    map_name = instance_name_stripped[:idx3]
    file_name = "destroy_" + str(agent_num) + "agents_"+ map_name + "_" + instance_name_stripped[idx3+1:] + "_SIPP_suboptimal_w_1,00_destroy_type_ORIGO_neighborhood_size_" + str(neighborhood_size)
    num = counts[file_name]
    counts[file_name] += 1
    file_name += "_" + str(num)

    sum_of_costs = []
    computation_times = []
    i = 0
    last = -1
    feasible = (int(experiment[2]) > 0 and int(experiment[8]) == 0) # feasible when solution cost is not -1 and number of restarts is 0
    if (feasible):
        file_res2 = read_file("./dest_exp/" + file_name + "-LNS.csv")
        for row in file_res2:
            if (i == 0): 
                i += 1
                continue
            sum_of_costs.append(row[1])
            if (i == 1):
                computation_times.append(float(row[2]))
            else:
                computation_times.append(float(row[2]) - last)
            i += 1
            last = float(row[2])


    algo_parameters = "[]"
    computation_time_cpu = float(experiment[20])
    computation_time_wall = float(experiment[0])
    expanded = int(experiment[10])
    generated = int(experiment[11]) + int(experiment[12])
    makespan = int(experiment[19])
    preprocessing_time_cpu = float(experiment[21])
    preprocessing_time_wall = float(experiment[14])
    seed = int(experiment[18])
    sum_of_cost = int(experiment[2])
    sum_of_dist = int(experiment[4])

    cursor.execute("""
    SELECT 1 FROM destroy WHERE 
        file_name = ?
    """, (file_name,))

    if cursor.fetchone() is None:  # If no record found, insert new entry
        # Dynamically create the SQL query
        cursor.execute("""
        INSERT INTO destroy (
            file_name, experiment_name, map_name, scene_name, num_agents,
algo_name, algo_parameters, sum_of_dist, feasible, init_sol_sum_of_cost, init_sol_makespan, experiment_time_cpu, experiment_time_wall, expanded, generated, preprocessing_time_cpu, preprocessing_time_wall, seed, sum_of_cost, makespan, iteration_time_cpu, iteration_time_wall, operators
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            file_name, "destroy", map_name, instance_name_stripped, agent_num, algo_name, algo_parameters, sum_of_dist, feasible, sum_of_cost, makespan, computation_time_cpu, computation_time_wall, expanded, generated, preprocessing_time_cpu, preprocessing_time_wall, seed, json.dumps(sum_of_costs), json.dumps([makespan]), json.dumps(computation_times), json.dumps(computation_times), json.dumps(["ADAPTIVE"])
        ))
        added += 1
    else:
        skipped += 1
conn.commit()
conn.close()
print("Experiment database updated successfully.")
print("Added: " + str(added) + "/" + str(added + skipped) + " experiments")
print("Skipped: " + str(skipped) + "/" + str(added + skipped) + " experiments")
