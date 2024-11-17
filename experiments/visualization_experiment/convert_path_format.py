import argparse

# initialize parser
parser = argparse.ArgumentParser(description='Convert paths from MAPF-LNS output format to mapf-visualizer input format')
parser.add_argument('input_name', metavar='input_name', type=str, help='Path file that should be converted')
parser.add_argument('output_name', metavar='output_name', type=str, help='Name of the resulting file')

# parse the arguments
args = parser.parse_args()
args_vars = vars(args)
input_name = args_vars["input_name"]
output_name = args_vars["output_name"]

# read the path file 
f = open(input_name, "r")
agents_paths = []
if f:
    for line in f:
        line1 = line[:-1].split(":")
        line2 = line1[1].split("->")
        h = []
        for i in range(len(line2[:-1])):
            term = line2[i]
            x, y = term[1:-1].split(",")
            h.append("(" + str(y) + "," + str(x) + ")")
        print(h)
        agents_paths.append(h)
else:
    print("Error: Could not open '" + input_name + "'")
    exit(1)
f.close()

f = open(output_name, "w")
if f:
    i = 0
    end = False
    while not end:
        h = []
        end = True
        for j in range(len(agents_paths)):
            if i < len(agents_paths[j])-1:
                h.append(agents_paths[j][i])
                end = False
            else:
                h.append(agents_paths[j][-1])
        f.write(str(i) + ":" +",".join(h) + "\n")
        i += 1
else:
    print("Error: Could not open '" + input_name + "'")
    exit(1)
f.close()


