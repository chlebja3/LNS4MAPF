import csv
import matplotlib.pyplot as plt
import re
import numpy as np


def read_file(name):
    lines = []
    with open(name, mode ='r')as file:
      csvFile = csv.reader(file)
      for line in csvFile:
          lines.append(line)
    return lines

file_res = read_file("results_mcarp.csv")
col_names = file_res[0]
file_res = file_res[1:]

res_mcarp = file_res[0::2]
res_SIPP = file_res[1::2]

avg_expanded_mcarp = []
max_expanded_mcarp = []
avg_expanded_sipp = []
max_expanded_sipp = []


avg_runtime_mcarp= []
max_runtime_mcarp= []
avg_runtime_sipp= []
max_runtime_sipp= []

scen_num=15
maps_rng = range(int(len(res_mcarp)/scen_num))
map_names = []
skipped = 0
for i in maps_rng:
    name = res_mcarp[scen_num*i][16]
    idx = name.find("/scen-random/")
    idx2 = name.find(".scen")
    name = name[idx+13:idx2-2]
    print("Map name:", name)
    if name in {"brc202d-random", "den312d-random", "empty-32-32-random"}:
        skipped += 1
        continue
    map_names.append(name)
    # generated + reopened
    expanded_nodes_MCARP = np.asarray([float(x[11]) + float(x[12]) for x in res_mcarp[scen_num*i:scen_num*(i+1)]])
    expanded_nodes_SIPP = np.asarray([float(x[11]) + float(x[12]) for x in res_SIPP[scen_num*i:scen_num*(i+1)]])
    # expanded_nodes_SIPP = np.asarray([float(x[10]) for x in res_SIPP[20*i:20*(i+1)]])
    # expanded_nodes_MCARP = np.asarray([float(x[10]) for x in res_mcarp[20*i:20*(i+1)]])
    runtime_MCARP = np.asarray([float(x[18]) for x in res_mcarp[scen_num*i:scen_num*(i+1)]])
    runtime_SIPP = np.asarray([float(x[18]) for x in res_SIPP[scen_num*i:scen_num*(i+1)]])
    # runtime_MCARP = np.asarray([float(x[7]) for x in res_mcarp[20*i:20*(i+1)]])
    # runtime_SIPP = np.asarray([float(x[7]) for x in res_SIPP[20*i:20*(i+1)]])

    delay_mcarp = np.asarray([(float(x[1]) - float(x[4])) for x in res_mcarp[scen_num*i:scen_num*(i+1)]])
    inv_mcarp = delay_mcarp < 0
    print("MCARP did not find solution:", np.sum(inv_mcarp))
    delay_SIPP = np.asarray([(float(x[1]) - float(x[4])) for x in res_SIPP[scen_num*i:scen_num*(i+1)]])
    inv_SIPP = delay_SIPP < 0
    both_valid = np.logical_and(~inv_SIPP, ~inv_mcarp) 
    print("SIPP did not find solution:", np.sum(inv_SIPP))


    sipp_better = expanded_nodes_SIPP[both_valid] - expanded_nodes_MCARP[both_valid] < 0
    mcarp_better = expanded_nodes_SIPP[both_valid] - expanded_nodes_MCARP[both_valid] > 0
    print("SIPP better in terms of expanded nodes", sipp_better.sum(), "times.")
    print("MCARP better in terms of expanded nodes", mcarp_better.sum(), "times.")
    sipp_better = runtime_SIPP[both_valid] - runtime_MCARP[both_valid] < 0
    mcarp_better = runtime_SIPP[both_valid] - runtime_MCARP[both_valid] > 0
    print("SIPP better in terms of runtime", sipp_better.sum(), "times.")
    print("MCARP better in terms of runtime", mcarp_better.sum(), "times.")

    # expanded nodes
    avg_expanded_mcarp.append(np.average(expanded_nodes_MCARP[both_valid]))
    max_expanded_mcarp.append(np.max(expanded_nodes_MCARP[both_valid]))
    avg_expanded_sipp.append(np.average(expanded_nodes_SIPP[both_valid]))
    max_expanded_sipp.append(np.max(expanded_nodes_SIPP[both_valid]))

    # runtime
    avg_runtime_mcarp.append(np.average(runtime_MCARP[both_valid]))
    max_runtime_mcarp.append(np.max(runtime_MCARP[both_valid]))
    avg_runtime_sipp.append(np.average(runtime_SIPP[both_valid]))
    max_runtime_sipp.append(np.max(runtime_SIPP[both_valid]))

    
fig, ax = plt.subplots(ncols=2, figsize=(12, 6))
fig.tight_layout()
ax[0].plot(maps_rng[:-skipped], avg_expanded_mcarp, linewidth=2, label="Average MCARP", color="blue")
ax[0].plot(maps_rng[:-skipped], avg_expanded_sipp, linewidth=2, label="Average SIPP", color="red")
ax[0].plot(maps_rng[:-skipped], max_expanded_mcarp, linewidth=2, label="Max MCARP", color="blue", linestyle="dashed")
ax[0].plot(maps_rng[:-skipped], max_expanded_sipp, linewidth=2, label="Max SIPP", color="red", linestyle="dashed")
ax[0].grid()
ax[0].set_xlabel("Map")
ax[0].set_ylabel("Number of nodes")
ax[0].set_xticks(maps_rng[:-skipped])
ax[0].set_xticklabels(map_names, rotation=75)
ax[0].set_title("Comparison of MCARP and SIPP in generated + reopened nodes number")
ax[1].plot(maps_rng[:-skipped], avg_runtime_mcarp, linewidth=2, label="Average MCARP", color="blue")
ax[1].plot(maps_rng[:-skipped], avg_runtime_sipp, linewidth=2, label="Average SIPP", color="red")
ax[1].plot(maps_rng[:-skipped], max_runtime_mcarp, linewidth=2, label="Max MCARP", color="blue", linestyle="dashed")
ax[1].plot(maps_rng[:-skipped], max_runtime_sipp, linewidth=2, label="Max SIPP", color="red", linestyle="dashed")
ax[1].set_title("Comparison of MCARP and SIPP in time to find initial solution")
ax[1].set_xticks(maps_rng[:-skipped])
ax[1].set_xlabel("Map")
ax[1].set_xticklabels(map_names, rotation=75)
ax[1].set_ylabel("Runtime")
ax[1].grid()
# fig.title("Comparison of Space Time Astar and SIPP")
plt.subplots_adjust(bottom=0.4)
plt.subplots_adjust(top=0.95)
plt.subplots_adjust(left=0.1, right=0.9, wspace=0.3)
ax[0].legend()
ax[1].legend()
plt.show()

