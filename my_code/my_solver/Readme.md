# MAPF solver with interactive visualizations

This directory holds the solver itself: an anytime MAPF-LNS solver with SIPP as the low-level
planner and an interactive SFML + ImGui visualizer (C++17).

**The documentation lives in the [README at the root of the repository](../../README.md)** —
dependencies, build, the command line options, the prepared instances, the tests and benchmarks,
the experiment harness and a visual tour of the GUI.

The short version:

```bash
./scripts/install_dependencies.sh   # apt packages + git submodules
./scripts/compile.sh                # -> ./build/simple_MAPF_solver
./scripts/run_specific_instance.sh den312d
./scripts/run.sh -m <map> -a <scenario> -k <number of agents>
./scripts/run.sh -h                 # all options
./scripts/run_tests.sh
```

## License

MIT, see [`LICENSE`](../../LICENSE).

## Author

Jan Chleboun (2025), <chlebja3@fel.cvut.cz>
