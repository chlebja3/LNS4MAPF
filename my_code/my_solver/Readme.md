# MAPF solver with interactive visualizations
This project is a multi-agent pathfinding (MAPF) solver with interactive visualizations. The solver is based on the MAPF-LNS and MAPF-LNS2 solvers. The visualizations use the SFML library.

# Dependencies
SFML 2.5.1

Boost 1.71.0

# Build
```
./scripts/compile.sh
```

or build manually

```
cd build
cmake ..
cmake --build .
```

# Run
## Demo 1: Simple 3x3 map, 1 agent
```
./scripts/run_dummy.sh
```

## Demo 2: den520d map from the MAPF benchmark with 200 agents
```
./scripts/run_den520d.sh
```

## Run with custom settings:

```
./build/mapf_solver -m 'path to map' -a 'path to scen file' -k 'number of agents' 
```

or 

```
./scripts/run.sh -m 'path to map' -a 'path to scen file' -k 'number of agents' 
```

# License
MIT

# Author
Jan Chleboun (2025)
email: chlebja3@fel.cvut.cz


