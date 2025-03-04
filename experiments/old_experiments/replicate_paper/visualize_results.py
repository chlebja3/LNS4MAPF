import csv
import matplotlib.pyplot as plt
import re

def read_file(name):
    lines = []
    with open(name, mode ='r')as file:
      csvFile = csv.reader(file)
      for line in csvFile:
          lines.append(line)
    return lines


# read LNS data
times_LNS = []
data_LNS = []
file_LNS = read_file("results.csv")

for i in range(len(file_LNS)):
  line = file_LNS[i]
  for j in range(len(line)):
      if "Average" in line[j]:
          num = int(re.findall(r'\d+', line[j])[0])
          times_LNS.append(num)
          data_LNS.append(float(file_LNS[i+1][j]))

# read ADDRESS data
times_ADD = []
data_ADD = []
file_ADD = read_file("results-LNS.csv")

for i in range(len(file_ADD)):
  line = file_ADD[i]
  for j in range(len(line)):
      if "Average" in line[j]:
          num = int(re.findall(r'\d+', line[j])[0])
          times_ADD.append(num)
          data_ADD.append(float(file_ADD[i+1][j]))

# check the data
if times_ADD != times_LNS:
    print("LNS and ADDRESS times are not the same!")
    exit()

# visualize the data
plt.plot(times_LNS, data_LNS, linewidth=2, color="b", label="LNS")
plt.plot(times_ADD, data_ADD, linewidth=2, color="r", label="ADDRESS")
plt.grid()
plt.legend()
plt.title("Comparison of MAPF-LNS and ADDRESS on den520d map with 700 agents")
plt.xlabel("Time [s]")
plt.ylabel("Sum of delays [-]")
plt.show()


