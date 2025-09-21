cd "$(dirname "$0")"
cd ..
if [ $# -eq 0 ]
then
valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./build/simple_MAPF_solver -m ../../MAPF-benchmark/mapf-map/empty-32-32.map -a ../../MAPF-benchmark/mapf-scen-random/scen-random/empty-32-32-random-1.scen -k 450 -G 0 -i 0 --destroy_operator ADAPTIVE -n 20 --use_psip True -s 0
else
valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./build/simple_MAPF_solver @
fi
