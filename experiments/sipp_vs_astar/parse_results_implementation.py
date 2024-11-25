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

file_res = read_file("results_implementation.csv")
col_names = file_res[0]
file_res = file_res[1:]

res_orig = file_res[0::2]
res_improved = file_res[1::2]

avg_iterations_orig = []
max_iterations_orig = []
avg_iterations_improved = []
max_iterations_improved = []


avg_runtime_orig= []
max_runtime_orig= []
avg_runtime_improved= []
max_runtime_improved= []

maps_rng = range(int(len(res_orig)/20))
map_names = []
for i in maps_rng:
    name = res_orig[20*i][16]
    idx = name.find("/scen-random/")
    idx2 = name.find(".scen")
    name = name[idx+13:idx2-2]
    print("Map name:", name)
    map_names.append(name)
    # generated + reopened
    iterations_orig = np.asarray([float(x[5]) for x in res_orig[20*i:20*(i+1)]])
    iterations_improved = np.asarray([float(x[5]) for x in res_improved[20*i:20*(i+1)]])
    # iterations_improved = np.asarray([float(x[10]) for x in res_improved[20*i:20*(i+1)]])
    # iterations_orig = np.asarray([float(x[10]) for x in res_orig[20*i:20*(i+1)]])
    runtime_orig = np.asarray([float(x[17]) for x in res_orig[20*i:20*(i+1)]])
    runtime_improved = np.asarray([float(x[17]) for x in res_improved[20*i:20*(i+1)]])
    # runtime_orig = np.asarray([float(x[7]) for x in res_orig[20*i:20*(i+1)]])
    # runtime_improved = np.asarray([float(x[7]) for x in res_improved[20*i:20*(i+1)]])

    improved_better = iterations_improved - iterations_orig > 0
    orig_better = iterations_improved - iterations_orig < 0
    print("improved better in terms of iterations", improved_better.sum(), "times.")
    print("orig better in terms of iterations", orig_better.sum(), "times.")
    improved_better = runtime_improved - runtime_orig < 0
    orig_better = runtime_improved - runtime_orig > 0
    print("improved better in terms of runtime", improved_better.sum(), "times.")
    print("orig better in terms of runtime", orig_better.sum(), "times.")

    # iterations nodes
    avg_iterations_orig.append(np.average(iterations_orig))
    max_iterations_orig.append(np.max(iterations_orig))
    avg_iterations_improved.append(np.average(iterations_improved))
    max_iterations_improved.append(np.max(iterations_improved))

    # runtime
    avg_runtime_orig.append(np.average(runtime_orig))
    max_runtime_orig.append(np.max(runtime_orig))
    avg_runtime_improved.append(np.average(runtime_improved))
    max_runtime_improved.append(np.max(runtime_improved))

    
fig, ax = plt.subplots(ncols=2, figsize=(12, 6))
fig.tight_layout()
ax[0].plot(maps_rng, avg_iterations_orig, linewidth=2, label="Average orig", color="blue")
ax[0].plot(maps_rng, avg_iterations_improved, linewidth=2, label="Average improved", color="red")
ax[0].plot(maps_rng, max_iterations_orig, linewidth=2, label="Max orig", color="blue", linestyle="dashed")
ax[0].plot(maps_rng, max_iterations_improved, linewidth=2, label="Max improved", color="red", linestyle="dashed")
ax[0].grid()
ax[0].set_xlabel("Map")
ax[0].set_ylabel("Iterations")
ax[0].set_xticks(maps_rng)
ax[0].set_xticklabels(map_names, rotation=75)
ax[0].set_title("Comparison of original and improved implementation in iteration number")
ax[1].plot(maps_rng, avg_runtime_orig, linewidth=2, label="Average orig", color="blue")
ax[1].plot(maps_rng, avg_runtime_improved, linewidth=2, label="Average improved", color="red")
ax[1].plot(maps_rng, max_runtime_orig, linewidth=2, label="Max orig", color="blue", linestyle="dashed")
ax[1].plot(maps_rng, max_runtime_improved, linewidth=2, label="Max improved", color="red", linestyle="dashed")
ax[1].set_title("Comparison of original and improved implementation in average replanning time")
ax[1].set_xticks(maps_rng)
ax[1].set_xlabel("Map")
ax[1].set_xticklabels(map_names, rotation=75)
ax[1].set_ylabel("Runtime")
ax[1].grid()
# fig.title("Comparison of Space Time Astar and improved")
plt.subplots_adjust(bottom=0.4)
plt.subplots_adjust(top=0.95)
plt.subplots_adjust(left=0.1, right=0.9, wspace=0.3)
ax[0].legend()
ax[1].legend()
plt.show()

