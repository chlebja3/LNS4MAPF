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

avg_delay_stas = []
max_delay_stas = []
avg_delay_sipp = []
max_delay_sipp = []

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
    # delay 
    sipp_better = delay_SIPP[both_valid] - delay_STAS[both_valid] < 0
    stas_better = delay_SIPP[both_valid] - delay_STAS[both_valid] > 0
    print("SIPP better in terms of delay", sipp_better.sum(), "times.")
    print("STAS better in terms of delay", stas_better.sum(), "times.")

    avg_delay_stas.append(np.average(delay_STAS[both_valid]))
    max_delay_stas.append(np.max(delay_STAS[both_valid]))
    avg_delay_sipp.append(np.average(delay_SIPP[both_valid]))
    max_delay_sipp.append(np.max(delay_SIPP[both_valid]))
    
fig, ax = plt.subplots(ncols=1, figsize=(6, 6))
fig.tight_layout()
ax.plot(maps_rng, avg_delay_stas, linewidth=2, label="Average STAS", color="blue")
ax.plot(maps_rng, avg_delay_sipp, linewidth=2, label="Average SIPP", color="red")
ax.plot(maps_rng, max_delay_stas, linewidth=2, label="Max STAS", color="blue", linestyle="dashed")
ax.plot(maps_rng, max_delay_sipp, linewidth=2, label="Max SIPP", color="red", linestyle="dashed")
ax.grid()
ax.set_xlabel("Map")
ax.set_ylabel("Delay")
ax.set_xticks(maps_rng)
ax.set_xticklabels(map_names, rotation=75)
ax.set_title("Comparison of STAS and SIPP in delay of the final solution")
# fig.title("Comparison of Space Time Astar and SIPP")
plt.subplots_adjust(bottom=0.4)
plt.subplots_adjust(top=0.95)
plt.subplots_adjust(left=0.1, right=0.9, wspace=0.3)
ax.legend()
ax.legend()
plt.show()

