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
weights = ["orig", 1.0, 1.1, 1.5, 2, 10]
res = [file_res[i::len(weights)] for i in range(len(weights))]

avg_delay = [[] for i in range(len(weights))]
max_delay = [[] for i in range(len(weights))]

avg_time = [[] for i in range(len(weights))]
max_time = [[] for i in range(len(weights))]

avg_iterations = [[] for i in range(len(weights))]
max_iterations = [[] for i in range(len(weights))]

avg_reopened = [[] for i in range(len(weights))]
max_reopened = [[] for i in range(len(weights))]

num_runs = 20

maps_rng = range(int(len(res[0])/num_runs))
map_names = []
for i in maps_rng:
    name = res[0][num_runs*i][16]
    idx = name.find("/scen-random/")
    idx2 = name.find(".scen")
    name = name[idx+13:idx2-2]
    print("Map name:", name)
    map_names.append(name)
    for j in range(len(res)):
        iterations = np.asarray([float(x[5]) for x in res[j][num_runs*i:num_runs*(i+1)]])
        time = np.asarray([float(x[17]) for x in res[j][num_runs*i:num_runs*(i+1)]])
        reopened = np.asarray([float(x[12]) for x in res[j][num_runs*i:num_runs*(i+1)]])
        delay = np.asarray([(float(x[1]) - float(x[4])) for x in res[j][num_runs*i:num_runs*(i+1)]])

        avg_iterations[j].append(np.average(iterations))
        max_iterations[j].append(np.max(iterations))
        avg_time[j].append(np.average(time))
        max_time[j].append(np.max(time))
        avg_delay[j].append(np.average(delay))
        max_delay[j].append(np.max(delay))
        avg_reopened[j].append(np.average(reopened))
        max_reopened[j].append(np.max(reopened))
    
fig, ax = plt.subplots(ncols=2, figsize=(12, 6))
fig.tight_layout()
colors = plt.cm.get_cmap("viridis", len(res))
for j in range(len(res)):
    ax[0].plot(maps_rng, avg_iterations[j], linewidth=2, label="Average w=" + str(weights[j]), color=colors(j))
    ax[0].plot(maps_rng, max_iterations[j], linewidth=2, label="Max w=" + str(weights[j]), color=colors(j), linestyle="dashed")
ax[0].grid()
ax[0].set_xlabel("Map")
ax[0].set_ylabel("Iterations")
ax[0].set_xticks(maps_rng)
ax[0].set_xticklabels(map_names, rotation=75)
ax[0].set_title("Comparison of suboptimal replanners in iteration number")

for j in range(len(res)):
    ax[1].plot(maps_rng, avg_time[j], linewidth=2, label="Average w=" + str(weights[j]), color=colors(j))
    ax[1].plot(maps_rng, max_time[j], linewidth=2, label="Max w=" + str(weights[j]), color=colors(j), linestyle="dashed")

ax[1].set_title("Comparison of original and improved implementation in average replanning time")
ax[1].set_xticks(maps_rng)
ax[1].set_xlabel("Map")
ax[1].set_xticklabels(map_names, rotation=75)
ax[1].set_ylabel("Replan time")
ax[1].grid()
plt.subplots_adjust(bottom=0.4)
plt.subplots_adjust(top=0.95)
plt.subplots_adjust(left=0.1, right=0.9, wspace=0.3)
ax[0].legend()
ax[1].legend()

fig2, ax2 = plt.subplots(ncols=2, figsize=(12, 6))
fig2.tight_layout()
for j in range(len(res)):
    ax2[0].plot(maps_rng, avg_delay[j], linewidth=2, label="Average w=" + str(weights[j]), color=colors(j))
    ax2[0].plot(maps_rng, max_delay[j], linewidth=2, label="Max w=" + str(weights[j]), color=colors(j), linestyle="dashed")
ax2[0].grid()
ax2[0].set_xlabel("Map")
ax2[0].set_ylabel("Delay")
ax2[0].set_xticks(maps_rng)
ax2[0].set_xticklabels(map_names, rotation=75)
ax2[0].set_title("Comparison of suboptimal replanners in delay")

for j in range(len(res)):
    ax2[1].plot(maps_rng, avg_reopened[j], linewidth=2, label="Average w=" + str(weights[j]), color=colors(j))
    ax2[1].plot(maps_rng, max_reopened[j], linewidth=2, label="Max w=" + str(weights[j]), color=colors(j), linestyle="dashed")

ax2[1].set_title("Comparison of suboptimal replanners in number of reopened nodes")
ax2[1].set_xticks(maps_rng)
ax2[1].set_xlabel("Map")
ax2[1].set_xticklabels(map_names, rotation=75)
ax2[1].set_ylabel("Reopened")
ax2[1].grid()
# fig.title("Comparison of Space Time Astar and improved")
plt.subplots_adjust(bottom=0.4)
plt.subplots_adjust(top=0.95)
plt.subplots_adjust(left=0.1, right=0.9, wspace=0.3)
ax2[0].legend()
ax2[1].legend()
plt.show()

