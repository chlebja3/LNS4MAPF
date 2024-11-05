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

avg_sareplan_stas = []
max_sareplan_stas = []
avg_sareplan_sipp = []
max_sareplan_sipp = []

maps_rng = range(int(len(res_STAS)/20))
map_names = []
invalid_sol_stas = [0 for i in maps_rng]
invalid_sol_sipp = [0 for i in maps_rng]
for i in maps_rng:
    name = res_STAS[20*i][16]
    idx = name.find("/scen-random/")
    idx2 = name.find(".scen")
    name = name[idx+13:idx2-2]
    print("Map name:", name)
    map_names.append(name)
    delay_STAS = np.asarray([(float(x[1]) - float(x[4])) for x in res_STAS[20*i:20*(i+1)]])
    inv_STAS = delay_STAS < 0
    print("STAS did not find solution:", np.sum(inv_STAS))
    delay_SIPP = np.asarray([(float(x[1]) - float(x[4])) for x in res_SIPP[20*i:20*(i+1)]])
    inv_SIPP = delay_SIPP < 0
    both_valid = np.logical_and(~inv_SIPP, ~inv_STAS) 
    print("SIPP did not find solution:", np.sum(inv_SIPP))
    # sareplan 
    sareplan_STAS = np.asarray([float(x[17]) for x in res_STAS[20*i:20*(i+1)]])
    sareplan_SIPP = np.asarray([float(x[17]) for x in res_SIPP[20*i:20*(i+1)]])
    sipp_better = sareplan_SIPP[both_valid] - sareplan_STAS[both_valid] < 0
    stas_better = sareplan_SIPP[both_valid] - sareplan_STAS[both_valid] > 0
    print("SIPP better in terms of average single agent replan time", sipp_better.sum(), "times.")
    print("STAS better in terms of average single agent replan time", stas_better.sum(), "times.")

    avg_sareplan_stas.append(np.average(sareplan_STAS[both_valid]))
    max_sareplan_stas.append(np.max(sareplan_STAS[both_valid]))
    avg_sareplan_sipp.append(np.average(sareplan_SIPP[both_valid]))
    max_sareplan_sipp.append(np.max(sareplan_SIPP[both_valid]))
    
fig, ax = plt.subplots(ncols=1, figsize=(6, 6))
fig.tight_layout()
ax.plot(maps_rng, avg_sareplan_stas, linewidth=2, label="Average STAS", color="blue")
ax.plot(maps_rng, avg_sareplan_sipp, linewidth=2, label="Average SIPP", color="red")
ax.plot(maps_rng, max_sareplan_stas, linewidth=2, label="Max STAS", color="blue", linestyle="dashed")
ax.plot(maps_rng, max_sareplan_sipp, linewidth=2, label="Max SIPP", color="red", linestyle="dashed")
ax.grid()
ax.set_xlabel("Map")
ax.set_ylabel("Average single agent replan time")
ax.set_xticks(maps_rng)
ax.set_xticklabels(map_names, rotation=75)
ax.set_title("Comparison of STAS and SIPP in average single agent replan time")
# fig.title("Comparison of Space Time Astar and SIPP")
plt.subplots_adjust(bottom=0.4)
plt.subplots_adjust(top=0.95)
plt.subplots_adjust(left=0.1, right=0.9, wspace=0.3)
ax.legend()
ax.legend()
plt.show()

