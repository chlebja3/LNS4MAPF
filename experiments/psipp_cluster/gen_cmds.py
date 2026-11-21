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


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("binary")
    parser.add_argument("results_dir")
    parser.add_argument("benchmark_root", nargs="?", default="$HOME/mapf_bench/MAPF-benchmark")
    parser.add_argument("--time-limit", type=float, default=TIME_LIMIT)
    parser.add_argument("--grid", choices=["push_cost", "order"], default="push_cost")
    args = parser.parse_args()
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
