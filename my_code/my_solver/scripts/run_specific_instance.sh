cd "$(dirname "$0")"
cd ..

# check which from the predefined instances to run
if [ $# -eq 0 ] || [ $1 == "den520d" ]
then
./build/simple_MAPF_solver -m ../../MAPF-benchmark/mapf-map/den520d.map -a ../../MAPF-benchmark/mapf-scen-random/scen-random/den520d-random-1.scen -k 500 $@
elif [ $1 == "dummy" ]
then 
./build/simple_MAPF_solver -m ../../my_map/dummy.map -a ../../my_map/dummy.scen -k 1 $@
elif [ $1 == "empty" ]
then
./scripts/run.sh -m ../../MAPF-benchmark/mapf-map/empty-16-16.map -a ../../MAPF-benchmark/mapf-scen-random/scen-random/empty-16-16-random-1.scen -k 20 $@
else 
  echo "Unknown instance"
fi
