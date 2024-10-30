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

file_res = read_file("results.csv")
col_names = file_res[0]
file_res = file_res[1:]

res_STAS = file_res[0::2]
res_SIPP = file_res[1::2]

avg_expanded_stas = []
max_expanded_stas = []
avg_expanded_sipp = []
max_expanded_sipp = []


avg_runtime_stas= []
max_runtime_stas= []
avg_runtime_sipp= []
max_runtime_sipp= []

maps_rng = range(int(len(res_STAS)/20))
map_names = []
for i in maps_rng:
    name = res_STAS[20*i][16]
    idx = name.find("/scen-random/")
    name = name[idx+13:]
    print("Map name:", name)
    map_names.append(name)
    expanded_nodes_STAS = np.asarray([float(x[10]) for x in res_STAS[20*i:20*(i+1)]])
    expanded_nodes_SIPP = np.asarray([float(x[10]) for x in res_SIPP[20*i:20*(i+1)]])
    runtime_STAS = np.asarray([float(x[10]) for x in res_STAS[20*i:20*(i+1)]])
    runtime_SIPP = np.asarray([float(x[10]) for x in res_SIPP[20*i:20*(i+1)]])
    sipp_better = expanded_nodes_SIPP - expanded_nodes_STAS < 0
    stas_better = expanded_nodes_SIPP - expanded_nodes_STAS > 0
    print("SIPP better in terms of expanded nodes", sipp_better.sum(), "times.")
    print("STAS better in terms of expanded nodes", stas_better.sum(), "times.")
    sipp_better = runtime_SIPP - runtime_STAS < 0
    stas_better = runtime_SIPP - runtime_STAS > 0
    print("SIPP better in terms of runtime", sipp_better.sum(), "times.")
    print("STAS better in terms of runtime", stas_better.sum(), "times.")

    # expanded nodes
    avg_expanded_stas.append(np.average(expanded_nodes_STAS))
    max_expanded_stas.append(np.max(expanded_nodes_STAS))
    avg_expanded_sipp.append(np.average(expanded_nodes_SIPP))
    max_expanded_sipp.append(np.max(expanded_nodes_SIPP))

    # runtime
    avg_runtime_stas.append(np.average(runtime_STAS))
    max_runtime_stas.append(np.max(runtime_STAS))
    avg_runtime_sipp.append(np.average(runtime_SIPP))
    max_runtime_sipp.append(np.max(runtime_SIPP))

    
fig, ax = plt.subplots(ncols=2, figsize=(12, 6))
fig.tight_layout()
ax[0].plot(maps_rng, avg_expanded_stas, linewidth=2, label="Average STAS", color="blue")
ax[0].plot(maps_rng, avg_expanded_sipp, linewidth=2, label="Average SIPP", color="red")
ax[0].plot(maps_rng, max_expanded_stas, linewidth=2, label="Max STAS", color="blue", linestyle="dashed")
ax[0].plot(maps_rng, max_expanded_sipp, linewidth=2, label="Max SIPP", color="red", linestyle="dashed")
ax[0].grid()
ax[0].set_xlabel("Map")
ax[0].set_ylabel("Number of nodes")
ax[0].set_xticklabels(map_names, rotation=75)
ax[0].set_title("Comparison of STAS and SIPP in expanded nodes number")
ax[1].plot(maps_rng, avg_runtime_stas, linewidth=2, label="Average STAS", color="blue")
ax[1].plot(maps_rng, avg_runtime_sipp, linewidth=2, label="Average SIPP", color="red")
ax[1].plot(maps_rng, max_runtime_stas, linewidth=2, label="Max STAS", color="blue", linestyle="dashed")
ax[1].plot(maps_rng, max_runtime_sipp, linewidth=2, label="Max SIPP", color="red", linestyle="dashed")
ax[1].set_title("Comparison of STAS and SIPP in runtime")
ax[1].set_xlabel("Map")
ax[1].set_xticklabels(map_names, rotation=75)
ax[1].set_ylabel("Runtime")
ax[1].grid()
# fig.title("Comparison of Space Time Astar and SIPP")
plt.subplots_adjust(bottom=0.4)
plt.subplots_adjust(top=0.9)
plt.subplots_adjust(left=0.1, right=0.9, wspace=0.3)
ax[0].legend()
ax[1].legend()
plt.show()

