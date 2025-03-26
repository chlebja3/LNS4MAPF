import csv
import sqlite3
from collections import defaultdict

def read_file(name):
    lines = []
    with open(name, mode ='r') as file:
      csvFile = csv.reader(file)
      for line in csvFile:
          lines.append(line)
    return lines

file_res = read_file("results_lns2.csv")

col_names = file_res[0]
file_res = file_res[1:]

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()
counts = defaultdict(int)
algo_name =  "SIPP_lns_orig"
added = 0
skipped = 0

for experiment in file_res:
# check whether the experiment is already in the database
    instance_name = experiment[17]
    agent_num = int(experiment[22])
    idx1 = len("../MAPF-benchmark/mapf-scen-random/scen-random/")
    idx2 = instance_name.find(".scen")
    instance_name_stripped = instance_name[idx1:idx2]
    experiment_name = "sipp_pp_" + str(agent_num) + "agents_"+ instance_name_stripped + algo_name
    num = counts[experiment_name]
    counts[experiment_name] += 1
    experiment_name += "_" + str(num)
    idx3 = instance_name_stripped.find("-random")
    map_name = instance_name_stripped[:idx3]

    feasible = (int(experiment[2]) != -1 and int(experiment[8]) == 0) # feasible when solution cost is not -1 and number of restarts is 0

    cursor.execute("""
    SELECT 1 FROM experiments WHERE 
        file_name = ?
    """, (experiment_name,))

    if cursor.fetchone() is None:  # If no record found, insert new entry
        # Dynamically create the SQL query
        cursor.execute("""
        INSERT INTO experiments (
            file_name, algo_name, algo_parameters, computation_time_cpu, computation_time_wall,
            expanded, experiment_name, feasible, generated, makespan, map_name, num_agents,
            preprocessing_time_cpu, preprocessing_time_wall, scene_name, seed,
            sum_of_cost, sum_of_dist
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            experiment_name, 
            algo_name, "[]", float(experiment[20]), float(experiment[0]), int(experiment[10]), "sipp_pp", feasible, int(experiment[11]) + int(experiment[12]), int(experiment[19]),
            map_name, agent_num, float(experiment[21]), float(experiment[14]),
            instance_name_stripped, int(experiment[18]), int(experiment[2]), int(experiment[4]) 
        ))
        added += 1
    else:
        skipped += 1
conn.commit()
conn.close()
print("Experiment database updated successfully.")
print("Added: " + str(added) + "/" + str(added + skipped) + " experiments")
print("Skipped: " + str(skipped) + "/" + str(added + skipped) + " experiments")
