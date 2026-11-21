#!/usr/bin/env python3
"""Emit one shell command per (map, k, variant, seed); each writes <results>/<map>_<k>_<variant>_<seed>.json
(and <same basename>.rc with the exit code if the run failed).

Usage: gen_cmds.py <binary> <results_dir> [benchmark_root]
Paths may contain $HOME; they are expanded by the shell that runs the commands.
"""
import argparse

MAPS = {
    "empty-32-32": [350, 400, 450, 500],
    "random-32-32-20": [200, 225, 250, 275, 300],
    "warehouse-10-20-10-2-1": [350, 450, 550, 650],
    "ost003d": [600, 800, 1000],
    "room-64-64-8": [300, 400, 500, 600],
    "den520d": [600, 900, 1200],
    "Paris_1_256": [600, 900, 1200],
}
PUSH_COSTS = ["inf", "0", "2", "10", "50"]
SEEDS = range(1, 26)
TIME_LIMIT = 60
TIMEOUT = 150
MAX_PUSHES = 5


# agent orders of Prioritized Planning: (pp_order, order_first_only)
ORDERS = [("random", 0), ("short", 0), ("long", 0), ("short", 1), ("long", 1)]


def order_tag(order: str, first_only: int) -> str:
    return f"_o{order}{'1st' if first_only else ''}"


def push_cost_variants() -> list[dict]:
    result = [{"id": "sipp_orandom", "planner": "sipp", "max_pushes": 0, "push_cost": "inf", "closed": 0,
               "sipp_first": 1, "order": "random", "first_only": 0}]
    for cost in PUSH_COSTS:
        for closed in (0, 1):
            for sipp_first in (0, 1):
                result.append({"id": f"psipp_c{cost}_cl{closed}_sf{sipp_first}_orandom", "planner": "psipp",
                               "max_pushes": MAX_PUSHES, "push_cost": cost, "closed": closed,
                               "sipp_first": sipp_first, "order": "random", "first_only": 0})
    return result


def order_variants() -> list[dict]:
    """SIPP and the recommended PSIPP setting (push cost inf, one closed list, SIPP first) with every agent order."""
    result = []
    for order, first_only in ORDERS:
        tag = order_tag(order, first_only)
        result.append({"id": "sipp" + tag, "planner": "sipp", "max_pushes": 0, "push_cost": "inf", "closed": 0,
                       "sipp_first": 1, "order": order, "first_only": first_only})
        result.append({"id": "psipp_cinf_cl0_sf1" + tag, "planner": "psipp", "max_pushes": MAX_PUSHES,
                       "push_cost": "inf", "closed": 0, "sipp_first": 1, "order": order, "first_only": first_only})
    return result


# final SIPP vs PSIPP comparison: 15 maps, the best agent order of each planner
PAPER_MAPS = ["empty-32-32", "empty-48-48", "random-32-32-10", "random-32-32-20", "random-64-64-20", "maze-32-32-4",
              "maze-128-128-2", "room-32-32-4", "room-64-64-8", "warehouse-10-20-10-2-1", "warehouse-20-40-10-2-1",
              "den520d", "ost003d", "lak303d", "Paris_1_256"]
# candidate agent counts for the calibration (max = agents in the scenario file)
CALIB_K = {
    "empty-32-32": [200, 250, 300, 350, 400, 450, 500],
    "empty-48-48": [300, 400, 500, 600, 700, 800, 900, 1000],
    "random-32-32-10": [150, 200, 250, 300, 350, 400, 450],
    "random-32-32-20": [100, 150, 200, 250, 300, 350, 400],
    "random-64-64-20": [300, 400, 500, 600, 700, 800, 900, 1000],
    "maze-32-32-4": [25, 50, 75, 100, 150, 200, 250],
    "maze-128-128-2": [100, 200, 300, 400, 500, 600, 800, 1000],
    "room-32-32-4": [50, 100, 150, 200, 250, 300],
    "room-64-64-8": [200, 300, 400, 500, 600, 700, 800],
    "warehouse-10-20-10-2-1": [300, 400, 500, 600, 700, 800, 900, 1000],
    "warehouse-20-40-10-2-1": [400, 500, 600, 700, 800, 900, 1000],
    "den520d": [400, 500, 600, 700, 800, 900, 1000],
    "ost003d": [400, 500, 600, 700, 800, 900, 1000],
    "lak303d": [200, 300, 400, 500, 600, 700, 800, 1000],
    "Paris_1_256": [400, 500, 600, 700, 800, 900, 1000],
}
CALIB_SCENS = range(1, 4)
PAPER_SCENS = range(1, 26)
PAPER_SEEDS = range(1, 6)


def paper_variants() -> list[dict]:
    """SIPP with longest-first order, PSIPP (push cost inf, one closed list, SIPP first) with longest-first order in
    the first attempt only (the best order of each planner in the order experiment)."""
    return [{"id": "sipp_olong", "planner": "sipp", "max_pushes": 0, "push_cost": "inf", "closed": 0,
             "sipp_first": 1, "order": "long", "first_only": 0},
            {"id": "psipp_cinf_cl0_sf1_olong1st", "planner": "psipp", "max_pushes": MAX_PUSHES, "push_cost": "inf",
             "closed": 0, "sipp_first": 1, "order": "long", "first_only": 1}]


def paper_command(binary: str, results_dir: str, b: str, m: str, k: int, v: dict, scen: int, seed: int,
                  time_limit: float) -> str:
    """One run writing <results>/<map>_<k>_<variant>_<scen>_<seed>.json."""
    base = f"{results_dir}/{m}_{k}_{v['id']}_{scen}_{seed}"
    return (f"rm -f {base}.rc; timeout {TIMEOUT} {binary} {b}/mapf-map/{m}.map "
            f"{b}/mapf-scen-random/scen-random/{m}-random-{scen}.scen {k} {v['planner']} {v['max_pushes']} {seed} "
            f"{time_limit:g} {v['push_cost']} {v['closed']} {v['sipp_first']} {v['order']} {v['first_only']} "
            f"> {base}.json 2>/dev/null || echo $? > {base}.rc")


SOTA_SOLVERS = ["ours_psipp_rp", "ours_psipp_rpd", "ours_psipp_rpdc10", "ours_psipp", "ours_sipp", "lns2", "lns", "balance", "address", "lacam3"]


def sota_command(sota_dir: str, results_dir: str, b: str, m: str, k: int, solver: str, scen: int, seed: int,
                 time_limit: float) -> str:
    """One run of the SOTA comparison writing <results>/<map>_<k>_<solver>_<scen>_<seed>.json (skipped if it exists).
    sota_dir holds sota_run.sh and sota_collect.py, sota_dir/bin the solver binaries."""
    out = f"{results_dir}/{m}_{k}_{solver}_{scen}_{seed}.json"
    return (f"[ -f {out} ] || bash {sota_dir}/sota_run.sh {sota_dir}/bin {solver} {b}/mapf-map/{m}.map "
            f"{b}/mapf-scen-random/scen-random/{m}-random-{scen}.scen {k} {seed} {time_limit:g} {out}")


def read_paper_k(path: str) -> dict[str, list[int]]:
    """Agent counts per map chosen by the calibration, one line per map: <map> <k1> <k2> ..."""
    result = {}
    with open(path) as f:
        for line in f:
            parts = line.split()
            if parts:
                result[parts[0]] = [int(x) for x in parts[1:]]
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("binary")
    parser.add_argument("results_dir")
    parser.add_argument("benchmark_root", nargs="?", default="$HOME/mapf_bench/MAPF-benchmark")
    parser.add_argument("--time-limit", type=float, default=TIME_LIMIT)
    parser.add_argument("--grid", choices=["push_cost", "order", "calib", "paper", "sota"], default="push_cost")
    parser.add_argument("--paper-k", help="file with the agent counts per map (for --grid paper and sota)")
    args = parser.parse_args()
    if args.grid == "sota":  # the binary argument is the directory with the SOTA scripts and bin/
        ks = read_paper_k(args.paper_k)
        for seed in PAPER_SEEDS:
            for scen in PAPER_SCENS:
                for m in PAPER_MAPS:
                    for k in ks[m]:
                        for solver in SOTA_SOLVERS:
                            print(sota_command(args.binary, args.results_dir, args.benchmark_root, m, k, solver, scen,
                                               seed, args.time_limit))
        return
    if args.grid in ("calib", "paper"):
        ks = CALIB_K if args.grid == "calib" else read_paper_k(args.paper_k)
        scens = CALIB_SCENS if args.grid == "calib" else PAPER_SCENS
        seeds = [1] if args.grid == "calib" else PAPER_SEEDS
        for m in PAPER_MAPS:
            for k in ks[m]:
                for scen in scens:
                    for seed in seeds:
                        for v in paper_variants():
                            print(paper_command(args.binary, args.results_dir, args.benchmark_root, m, k, v, scen,
                                                seed, args.time_limit))
        return
    variants = push_cost_variants if args.grid == "push_cost" else order_variants
    b = args.benchmark_root
    for m, ks in MAPS.items():
        for k in ks:
            for seed in SEEDS:
                for v in variants():
                    out = f"{args.results_dir}/{m}_{k}_{v['id']}_{seed}.json"
                    print(f"rm -f {out[:-len('.json')]}.rc; timeout {TIMEOUT} {args.binary} {b}/mapf-map/{m}.map "
                          f"{b}/mapf-scen-random/scen-random/{m}-random-{seed}.scen {k} {v['planner']} "
                          f"{v['max_pushes']} {seed} {args.time_limit:g} {v['push_cost']} {v['closed']} "
                          f"{v['sipp_first']} {v['order']} {v['first_only']} > {out} 2>/dev/null || echo $? > {out[:-len('.json')]}.rc")


if __name__ == "__main__":
    main()
