import os
import matplotlib.pyplot as plt
import csv
import numpy as np

def read_file(name):
    lines = []
    with open(name, mode ='r') as file:
      csvFile = csv.reader(file)
      for line in csvFile:
          lines.append(line)
    return lines

map_name = "brc202d"
scen_num = 1 
w = ["w_1_0_", "w_1_1_", "w_1_5_", "w_2_0_", "w_5_0_", "w_10_0_", "w_100_0_"]
file_names = ["./" + map_name + "/" + w[i] + str(scen_num) + ".csv" for i in range(len(w))]

delays = []
for f_n in file_names:
    lines = read_file(f_n)
    lines = lines[1:]
    delay = np.asarray([(float(x[1]) - float(x[4])) for x in lines])
    delays.append(delay)

fig, ax = plt.subplots(ncols=1, figsize=(6, 6))
fig.tight_layout()
weights = [1, 1.1, 1.5, 2, 5, 10, 100]
colors = plt.cm.get_cmap("viridis", len(delays))
# time_coefs = [0.0346831, 0.0231933, 0.0187435, 0.0127232, 0.00896849, 0.00857854, 0.00837094, 0.0347515, 0.0231554, 0.0186118, 0.0127423, 0.00902275, 0.00868921, 0.00835005] # den520d scen 1
time_coefs = [0.250208, 0.190228, 0.157142, 0.113852, 0.0647054, 0.0564485, 0.0502255 ] # brc202d

for j in range(len(delays)):
    it = (np.arange(delays[j].shape[0]) + 1) * time_coefs[j]
    ax.plot(it, delays[j], linewidth=2, label="Average w=" + str(weights[j]), color=colors(j))
ax.grid()
ax.set_xlabel("Iterations multiplied by average iteration time")
ax.set_ylabel("Delay")
ax.set_title("Comparison of suboptimal replanners in delay evolution:" + map_name + " scen: " + str(scen_num+1))
ax.legend()
plt.show()

