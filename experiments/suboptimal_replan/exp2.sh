../../my_code/suboptimal_replan/lns -m ../../MAPF-benchmark/mapf-map/den520d.map -a ../../MAPF-benchmark/mapf-scen-random/scen-random/den520d-random-1.scen -o exp2 -k 500 -t 30 --initAlgo PP --singleAgentReplan SIPP --maxIterations 100000 --initLNS 0 --w 1.0 --stats 1
mv 1-LNS.csv w_1_0_0.csv
../../my_code/suboptimal_replan/lns -m ../../MAPF-benchmark/mapf-map/den520d.map -a ../../MAPF-benchmark/mapf-scen-random/scen-random/den520d-random-1.scen -o exp2 -k 500 -t 30 --initAlgo PP --singleAgentReplan SIPP --maxIterations 100000 --initLNS 0 --w 1.1 --stats 1
mv 1-LNS.csv w_1_1_0.csv
../../my_code/suboptimal_replan/lns -m ../../MAPF-benchmark/mapf-map/den520d.map -a ../../MAPF-benchmark/mapf-scen-random/scen-random/den520d-random-1.scen -o exp2 -k 500 -t 30 --initAlgo PP --singleAgentReplan SIPP --maxIterations 100000 --initLNS 0 --w 1.5 --stats 1
mv 1-LNS.csv w_1_5_0.csv
../../my_code/suboptimal_replan/lns -m ../../MAPF-benchmark/mapf-map/den520d.map -a ../../MAPF-benchmark/mapf-scen-random/scen-random/den520d-random-1.scen -o exp2 -k 500 -t 30 --initAlgo PP --singleAgentReplan SIPP --maxIterations 100000 --initLNS 0 --w 2.0 --stats 1
mv 1-LNS.csv w_2_0_0.csv
../../my_code/suboptimal_replan/lns -m ../../MAPF-benchmark/mapf-map/den520d.map -a ../../MAPF-benchmark/mapf-scen-random/scen-random/den520d-random-1.scen -o exp2 -k 500 -t 30 --initAlgo PP --singleAgentReplan SIPP --maxIterations 100000 --initLNS 0 --w 5.0 --stats 1
mv 1-LNS.csv w_5_0_0.csv
../../my_code/suboptimal_replan/lns -m ../../MAPF-benchmark/mapf-map/den520d.map -a ../../MAPF-benchmark/mapf-scen-random/scen-random/den520d-random-1.scen -o exp2 -k 500 -t 30 --initAlgo PP --singleAgentReplan SIPP --maxIterations 100000 --initLNS 0 --w 10.0 --stats 1
mv 1-LNS.csv w_10_0_0.csv
../../my_code/suboptimal_replan/lns -m ../../MAPF-benchmark/mapf-map/den520d.map -a ../../MAPF-benchmark/mapf-scen-random/scen-random/den520d-random-1.scen -o exp2 -k 500 -t 30 --initAlgo PP --singleAgentReplan SIPP --maxIterations 100000 --initLNS 0 --w 100.0 --stats 1
mv 1-LNS.csv w_100_0_0
