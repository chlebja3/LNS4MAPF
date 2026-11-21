"""Collect the result of one solver run of the SOTA comparison into one JSON line.

Parses the raw output of the solver (our anytime_bench, MAPF-LNS2 / MAPF-LNS / BALANCE / ADDRESS, or LaCAM3),
validates the solution independently (starts, goals, moves on free cells, vertex and swap conflicts including
agents parked at their goals) and recomputes its sum of costs.

Usage: sota_collect.py <solver> <map> <scen> <k> <time_limit> <run dir> <rc>
"""
import csv
import json
import os
import re
import sys

LNS2_FAMILY = {"lns2", "balance", "address"}
PAIR = re.compile(r"\((\d+),(\d+)\)")
LACAM_CHECKPOINT = 0.5  # --checkpoints-duration of LaCAM3 in sota_run.sh [s]


def read_map(path: str) -> tuple[int, int, list[str]]:
    with open(path) as f:
        lines = f.read().split("\n")
    height = int(lines[1].split()[1])
    width = int(lines[2].split()[1])
    grid = [lines[4 + r][:width] for r in range(height)]
    return height, width, grid


def read_scen(path: str, k: int) -> tuple[list[tuple[int, int]], list[tuple[int, int]]]:
    starts, goals = [], []
    with open(path) as f:
        f.readline()
        for line in f:
            if len(starts) == k:
                break
            parts = line.split("\t")
            # scen columns: bucket, map, width, height, start x (col), start y (row), goal x, goal y, optimal length
            starts.append((int(parts[5]), int(parts[4])))
            goals.append((int(parts[7]), int(parts[6])))
    assert len(starts) == k, "scenario has fewer agents than requested"
    return starts, goals


def paths_lns2_format(path: str) -> list[list[tuple[int, int]]]:
    """'Agent i:(row,col)->(row,col)->...' (MAPF-LNS2 family and our solver)."""
    paths = []
    with open(path) as f:
        for line in f:
            if line.startswith("Agent"):
                paths.append([(int(r), int(c)) for r, c in PAIR.findall(line.split(":", 1)[1])])
    return paths


def paths_lns_format(path: str, width: int) -> list[list[tuple[int, int]]]:
    """First line the number of agents, then one line of 'location,' per agent (MAPF-LNS, location = row*width+col)."""
    with open(path) as f:
        n = int(f.readline())
        paths = [[(int(v) // width, int(v) % width) for v in f.readline().strip().strip(",").split(",")] for _ in range(n)]
    return paths


def paths_lacam_format(lines: list[str], k: int) -> list[list[tuple[int, int]]]:
    """'solution=' followed by one line 't:(x,y),(x,y),...' per timestep (LaCAM, x = col)."""
    start = lines.index("solution=") + 1
    paths = [[] for _ in range(k)]
    for line in lines[start:]:
        if ":" not in line:
            continue
        config = PAIR.findall(line.split(":", 1)[1])
        assert len(config) == k, "LaCAM configuration of a wrong size"
        for i, (x, y) in enumerate(config):
            paths[i].append((int(y), int(x)))
    return paths


def validate(paths: list[list[tuple[int, int]]], grid: list[str], starts: list[tuple[int, int]],
             goals: list[tuple[int, int]]) -> tuple[bool, str, int]:
    """Returns (valid, reason, sum of costs). The cost of an agent is the time it reaches its goal for the last time."""
    if len(paths) != len(starts):
        return False, "wrong number of paths", -1
    height, width = len(grid), len(grid[0])
    soc = 0
    for i, p in enumerate(paths):
        if not p or p[0] != starts[i] or p[-1] != goals[i]:
            return False, f"agent {i} wrong start or goal", -1
        for t, (r, c) in enumerate(p):
            if not (0 <= r < height and 0 <= c < width) or grid[r][c] not in ".GS":
                return False, f"agent {i} on an obstacle at t={t}", -1
            if t > 0 and abs(r - p[t - 1][0]) + abs(c - p[t - 1][1]) > 1:
                return False, f"agent {i} jumps at t={t}", -1
        cost = len(p) - 1
        while cost > 0 and p[cost - 1] == goals[i]:
            cost -= 1
        soc += cost
    horizon = max(len(p) for p in paths)
    for t in range(horizon):
        occupied = {}
        for i, p in enumerate(paths):
            v = p[min(t, len(p) - 1)]
            if v in occupied:
                return False, f"vertex conflict of agents {occupied[v]} and {i} at t={t}", -1
            occupied[v] = i
        if t == 0:
            continue
        moves = {}
        for i, p in enumerate(paths):
            a, b = p[min(t - 1, len(p) - 1)], p[min(t, len(p) - 1)]
            if a != b:
                moves[(a, b)] = i
        for (a, b), i in moves.items():
            if (b, a) in moves:
                return False, f"swap conflict of agents {i} and {moves[(b, a)]} at t={t}", -1
    return True, "", soc


def read_csv(path: str) -> list[dict]:
    with open(path) as f:
        return list(csv.DictReader(f))


def anytime_curve(t_first: float, initial_soc: int, stats_path: str) -> list[list[float]]:
    """[time, soc] of the first solution and every improvement from a MAPF-LNS(2)-style --stats file."""
    curve = [[t_first, initial_soc]]
    if os.path.exists(stats_path):
        with open(stats_path) as f:
            for row in csv.DictReader(f):
                soc = int(row["sum of costs"])
                if soc < curve[-1][1]:
                    curve.append([float(row["runtime"]), soc])
    return curve


def collect_lns_family(run: str, result_file: str, stats_file: str) -> tuple[float, list[list[float]]]:
    """Time of the first solution and the anytime curve from the -o result and --stats files."""
    result = read_csv(os.path.join(run, result_file))[-1]
    t_first = float(result["runtime of initial solution"])
    return t_first, anytime_curve(t_first, int(result["initial solution cost"]), os.path.join(run, stats_file))


def main() -> None:
    solver, map_path, scen_path, k, time_limit, run, rc = sys.argv[1:8]
    k, time_limit, rc = int(k), float(time_limit), int(rc)
    out = {"solver": solver, "map": os.path.basename(map_path), "scen": os.path.basename(scen_path), "k": k,
           "time_limit": time_limit, "rc": rc, "success": 0, "valid": 0, "t_first": -1, "soc": -1,
           "soc_reported": -1, "reason": "", "curve": []}
    _, width, grid = read_map(map_path)
    starts, goals = read_scen(scen_path, k)
    paths = None
    if solver.startswith("ours"):
        with open(os.path.join(run, "out.json")) as f:
            text = f.read().strip()
        ours = json.loads(text) if text else {"success": 0}  # empty when killed by the hard timeout
        if ours["success"]:
            out["t_first"], out["curve"] = ours["t_first"], ours["curve"]
            out["iterations"], out["init_attempts"] = ours["iterations"], ours["init_attempts"]
            paths = paths_lns2_format(os.path.join(run, "paths.txt"))
    elif solver in LNS2_FAMILY or solver == "lns":
        if os.path.exists(os.path.join(run, "paths.txt")):
            out["t_first"], out["curve"] = (collect_lns_family(run, "res-LNS.csv", "stats-LNS.csv") if solver != "lns"
                                            else collect_lns_family(run, "res.csv", "stats.csv"))
            paths = (paths_lns2_format(os.path.join(run, "paths.txt")) if solver != "lns"
                     else paths_lns_format(os.path.join(run, "paths.txt"), width))
    elif solver == "lacam3":
        log_path = os.path.join(run, "result.txt")
        if os.path.exists(log_path):
            with open(log_path) as f:
                lines = f.read().split("\n")
            info = dict(line.split("=", 1) for line in lines if "=" in line and not line.startswith("solution"))
            if info.get("solved") == "1":
                out["t_first"] = float(info["comp_time_initial_solution"]) / 1000
                curve = [[out["t_first"], int(info["cost_initial_solution"])]]
                # costs are sums of costs thanks to the patch in sota_build.sh; a checkpoint holds the cost of the
                # current solution at its time, so a change is dated at most one period late. LaCAM3 minimizes the
                # sum of loss, the sum of costs of its current solution may therefore also increase: the curve follows
                # the solution LaCAM3 would return at that time, not the best one seen
                for i, c in enumerate(info["checkpoints"].strip(",").split(",")):
                    if int(c) >= 0 and int(c) != curve[-1][1]:
                        curve.append([max(i * LACAM_CHECKPOINT, out["t_first"]), int(c)])
                if int(info["soc"]) != curve[-1][1]:
                    curve.append([float(info["comp_time"]) / 1000, int(info["soc"])])
                out["curve"] = curve
                paths = paths_lacam_format(lines, k)
    else:
        raise ValueError(f"unknown solver {solver}")

    if paths is not None and out["t_first"] <= time_limit:
        out["success"] = 1
        out["soc_reported"] = out["curve"][-1][1]
        valid, reason, soc = validate(paths, grid, starts, goals)
        out["valid"], out["reason"], out["soc"] = int(valid), reason, soc
    print(json.dumps(out))


if __name__ == "__main__":
    main()
