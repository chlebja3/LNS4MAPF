#!/usr/bin/env python3
"""Emit one shell command per (map, k, variant, seed); each writes <results>/<map>_<k>_<variant>_<seed>.json.

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


def variants() -> list[dict]:
    result = [{"id": "sipp", "planner": "sipp", "max_pushes": 0, "push_cost": "inf", "closed": 0, "sipp_first": 1}]
    for cost in PUSH_COSTS:
        for closed in (0, 1):
            for sipp_first in (0, 1):
                result.append({"id": f"psipp_c{cost}_cl{closed}_sf{sipp_first}", "planner": "psipp",
                               "max_pushes": MAX_PUSHES, "push_cost": cost, "closed": closed,
                               "sipp_first": sipp_first})
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("binary")
    parser.add_argument("results_dir")
    parser.add_argument("benchmark_root", nargs="?", default="$HOME/mapf_bench/MAPF-benchmark")
    parser.add_argument("--time-limit", type=float, default=TIME_LIMIT)
    args = parser.parse_args()
    b = args.benchmark_root
    for m, ks in MAPS.items():
        for k in ks:
            for seed in SEEDS:
                for v in variants():
                    out = f"{args.results_dir}/{m}_{k}_{v['id']}_{seed}.json"
                    print(f"timeout {TIMEOUT} {args.binary} {b}/mapf-map/{m}.map "
                          f"{b}/mapf-scen-random/scen-random/{m}-random-{seed}.scen {k} {v['planner']} "
                          f"{v['max_pushes']} {seed} {args.time_limit:g} {v['push_cost']} {v['closed']} "
                          f"{v['sipp_first']} > {out} 2>/dev/null")


if __name__ == "__main__":
    main()
