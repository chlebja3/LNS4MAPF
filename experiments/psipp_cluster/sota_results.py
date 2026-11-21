#!/usr/bin/env python3
"""Results of the comparison with the state-of-the-art solvers (gen_cmds.py --grid sota, sota_run.sh).

Reads <results>/<map>_<k>_<solver>_<scen>_<seed>.json (one sota_collect.py line each) and writes thesis-style figures (one
panel per map, mean per number of agents) and markdown tables:
- success rate: a valid first solution within the time limit (also for shorter limits, read from the time of the first
  solution, which does not depend on the limit),
- time to the first solution (an unsolved instance counts as the time limit),
- GAP of the final solution to the best known solution (the best final solution of all solvers) on the instances
  solved by all solvers,
- primal integral: the mean over [0, limit] of the primal gap (soc(t) - best) / soc(t) of the solution the solver
  holds at time t, 1 before its first solution (0 = the best solution immediately, 1 = no solution),
- anytime curves of the mean primal gap over time.
A solution that fails the independent validation counts as no solution.

Usage: sota_results.py <results_dir> <out_dir> [--limits 60 10]
"""
import argparse
import json
import math
import os
import re

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

SOLVERS = {"ours_psipp_rp": "Ours (PSIPP + PSIPP repair)",
           "ours_psipp_rpd": "Ours (PSIPP + direct PSIPP repair)",
           "ours_psipp_rpdc10": "Ours (PSIPP + direct PSIPP repair, cost 10)",
           "ours_psipp": "Ours (PSIPP)",
           "ours_sipp": "Ours (SIPP, thesis)",
           "lns2": "MAPF-LNS2", "lns": "MAPF-LNS", "balance": "BALANCE", "address": "ADDRESS", "lacam3": "LaCAM3"}
ALGOS = list(SOLVERS.values())
MARKERS = ["*", "h", "p", "o", "s", "^", "v", "D", "P", "X"]
NAME_RE = re.compile(r"^(.+)_(\d+)_(" + "|".join(SOLVERS) + r")_(\d+)_(\d+)\.json$")
MAP_ORDER = ["empty-32-32", "empty-48-48", "random-32-32-10", "random-32-32-20", "random-64-64-20", "maze-32-32-4",
             "maze-128-128-2", "room-32-32-4", "room-64-64-8", "warehouse-10-20-10-2-1", "warehouse-20-40-10-2-1",
             "den520d", "ost003d", "lak303d", "Paris_1_256"]
INST = ["map", "k", "scen"]  # an instance
KEY = INST + ["seed"]  # one run of every solver
TIME_GRID = np.logspace(-2, math.log10(60), 200)


def load(results_dir: str) -> pd.DataFrame:
    records = []
    for name in os.listdir(results_dir):
        m = NAME_RE.match(name)
        if not m:
            continue
        with open(os.path.join(results_dir, name)) as f:
            r = json.loads(f.read())
        solved = r["success"] == 1 and r["valid"] == 1
        records.append({"map": m.group(1), "k": int(m.group(2)), "algo": SOLVERS[m.group(3)],
                        "scen": int(m.group(4)), "seed": int(m.group(5)), "rc": r["rc"], "success": r["success"], "valid": r["valid"],
                        "reason": r["reason"], "solved": int(solved), "t_first": r["t_first"] if solved else math.nan,
                        "soc": r["soc"] if solved else math.nan, "soc_reported": r["soc_reported"],
                        "curve": r["curve"] if solved else [], "time_limit": r["time_limit"],
                        "iterations": r.get("iterations", math.nan)})
    return pd.DataFrame(records)


def with_limit(df: pd.DataFrame, limit: float) -> pd.DataFrame:
    """Results as if the time limit was `limit`: solved iff the first solution came by then, the curve is cut."""
    d = df.copy()
    d["solved"] = ((d["solved"] == 1) & (d["t_first"] <= limit)).astype(int)
    d["curve"] = [[p for p in c if p[0] <= limit] if s else [] for c, s in zip(d["curve"], d["solved"])]
    d["soc"] = [c[-1][1] if c else math.nan for c in d["curve"]]
    d["t_first"] = d["t_first"].where(d["solved"] == 1)
    d["time_limit"] = limit
    return d


def add_best(d: pd.DataFrame) -> pd.DataFrame:
    """Best known solution of every instance (the best final solution of all solvers and seeds)."""
    d = d.copy()
    d["best"] = d.groupby(INST)["soc"].transform("min")
    return d


def primal_gap_at(curve: list, best: float, times: np.ndarray) -> np.ndarray:
    """Primal gap of the solution held at each time (1 before the first solution)."""
    gap = np.ones(len(times))
    for t, soc in curve:
        gap[times >= t] = (soc - best) / soc
    return gap


def primal_integral(curve: list, best: float, limit: float) -> float:
    if not curve:
        return 1.0
    area, prev_t, prev_gap = 0.0, 0.0, 1.0
    for t, soc in curve:
        area += (t - prev_t) * prev_gap
        prev_t, prev_gap = t, (soc - best) / soc
    area += (limit - prev_t) * prev_gap
    return area / limit


def per_map_grid(maps: list[str]) -> tuple[plt.Figure, np.ndarray]:
    ncols = 3
    nrows = math.ceil(len(maps) / ncols)
    fig, axes = plt.subplots(nrows=nrows, ncols=ncols, figsize=(9, 2.33 * nrows + 0.9), sharey=False)
    return fig, axes.flatten()


def finish(fig: plt.Figure, axes: np.ndarray, n_maps: int, path: str) -> None:
    for ax in axes[n_maps:]:
        ax.axis("off")
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="lower center", bbox_to_anchor=(0.5, 0.005), ncol=4)
    plt.tight_layout()
    plt.subplots_adjust(bottom=1.6 / fig.get_figheight())  # three legend rows below the panels
    fig.savefig(path)
    plt.close(fig)


def plot_metric(stats_df: pd.DataFrame, column: str, ylabel: str, path: str, ylim: tuple | None = None,
                logy: bool = False) -> None:
    """Thesis-style figure: one panel per map, `column` per number of agents for each solver."""
    maps = [m for m in MAP_ORDER if m in set(stats_df["map"])]
    fig, axes = per_map_grid(maps)
    for ax, map_name in zip(axes, maps):
        map_group = stats_df[stats_df["map"] == map_name]
        for algo, marker in zip(ALGOS, MARKERS):
            algo_group = map_group[map_group["algo"] == algo].sort_values("k")
            ax.plot(algo_group["k"], algo_group[column], marker=marker, markersize=4, label=algo,
                    linewidth=2.2 if algo.startswith("Ours (PSIPP") else 1.2)
        ax.set_title(map_name)
        ax.set_xlabel("Number of Agents [-]")
        ax.set_ylabel(ylabel)
        if ylim is not None:
            ax.set_ylim(*ylim)
        if logy:
            ax.set_yscale("log")
        ax.grid(True)
    finish(fig, axes, len(maps), path)


def plot_anytime(d: pd.DataFrame, path: str) -> None:
    """Mean primal gap over time per map (all numbers of agents together), log time axis."""
    maps = [m for m in MAP_ORDER if m in set(d["map"])]
    limit = d["time_limit"].iloc[0]
    times = TIME_GRID[TIME_GRID <= limit]
    fig, axes = per_map_grid(maps)
    for ax, map_name in zip(axes, maps):
        sub = d[d["map"] == map_name]
        for algo, marker in zip(ALGOS, MARKERS):
            runs = sub[sub["algo"] == algo]
            if runs.empty:
                continue
            gaps = np.mean([primal_gap_at(c, b, times) for c, b in zip(runs["curve"], runs["best"])], axis=0)
            ax.plot(times, gaps, label=algo, linewidth=2.2 if algo.startswith("Ours (PSIPP") else 1.2)
        ax.set_title(map_name)
        ax.set_xscale("log")
        ax.set_xlabel("Time [s]")
        ax.set_ylabel("Primal gap [-]")
        ax.set_ylim(-0.02, 1.02)
        ax.grid(True)
    finish(fig, axes, len(maps), path)


def common_solved(d: pd.DataFrame) -> pd.DataFrame:
    solved = d[d["solved"] == 1]
    counts = solved.groupby(KEY)["algo"].transform("nunique")
    return solved[counts == len(ALGOS)]


def metrics(d: pd.DataFrame) -> pd.DataFrame:
    """Per-run metrics: penalized time to the first solution, final GAP, primal integral."""
    d = add_best(d)
    limit = d["time_limit"].iloc[0]
    d["t_first_pen"] = d["t_first"].fillna(limit)
    d["gap"] = (d["soc"] - d["best"]) / d["best"]
    d["pi"] = [primal_integral(c, b, limit) for c, b in zip(d["curve"], d["best"])]
    return d


def figures(d: pd.DataFrame, limit: float, out_dir: str) -> None:
    tag = f"{limit:g}s"
    by_k = d.groupby(["map", "k", "algo"]).agg(success=("solved", "mean"), t_first=("t_first_pen", "mean"),
                                               pi=("pi", "mean")).reset_index()
    plot_metric(by_k, "success", "Success rate [-]", f"{out_dir}/sota_success_rate_{tag}.pdf", (-0.05, 1.05))
    plot_metric(by_k, "t_first", "Time to first sol. [s]", f"{out_dir}/sota_time_first_{tag}.pdf", logy=True)
    plot_metric(by_k, "pi", "Primal integral [-]", f"{out_dir}/sota_primal_integral_{tag}.pdf", (-0.02, 1.02))
    common = common_solved(d)
    gap = common.groupby(["map", "k", "algo"]).agg(gap=("gap", "mean")).reset_index()
    plot_metric(gap, "gap", "GAP [-]", f"{out_dir}/sota_gap_{tag}.pdf")
    plot_anytime(d, f"{out_dir}/sota_anytime_{tag}.pdf")


def summary_table(d: pd.DataFrame, group: list[str]) -> pd.DataFrame:
    """Success rate, mean penalized time to first solution, mean primal integral and GAP on commonly solved."""
    common = common_solved(d)
    rows = []
    groups = d.groupby(group) if group else [((), d)]
    for g, sub in groups:
        key = dict(zip(group, g if isinstance(g, tuple) else (g,)))
        csub = common.merge(pd.DataFrame([key]), on=group) if group else common
        for algo in ALGOS:
            a = sub[sub["algo"] == algo]
            ca = csub[csub["algo"] == algo]
            rows.append({**key, "solver": algo, "runs": len(a), "success": f"{a['solved'].mean():.1%}",
                         "time first [s]": f"{a['t_first_pen'].mean():.2f}",
                         "primal integral": f"{a['pi'].mean():.4f}",
                         "GAP common": f"{ca['gap'].mean():.4f}" if len(ca) else "–", "n common": len(ca)})
    return pd.DataFrame(rows)


def head_to_head(d: pd.DataFrame, ours: str) -> pd.DataFrame:
    """Instances where `ours` is better / equal / worse than each other solver (no solution is worst)."""
    wide = d.pivot_table(index=KEY, columns="algo", values="soc", aggfunc="first")
    rows = []
    a = wide[ours].fillna(math.inf)
    for algo in ALGOS:
        if algo == ours:
            continue
        b = wide[algo].fillna(math.inf)
        both_unsolved = (a == math.inf) & (b == math.inf)
        rows.append({"vs": algo, "better": int((a < b).sum()), "equal": int(((a == b) & ~both_unsolved).sum()),
                     "worse": int((a > b).sum()), "both unsolved": int(both_unsolved.sum())})
    return pd.DataFrame(rows)


def seed_table(d: pd.DataFrame) -> pd.DataFrame:
    """Robustness over the seeds: success rate and primal integral of every seed, their mean and std over the seeds."""
    per_seed = d.groupby(["algo", "seed"]).agg(success=("solved", "mean"), pi=("pi", "mean")).reset_index()
    rows = []
    for algo in ALGOS:
        a = per_seed[per_seed["algo"] == algo]
        rows.append({"solver": algo, "seeds": len(a),
                     "success mean": f"{a['success'].mean():.1%}", "success std": f"{a['success'].std():.2%}",
                     "success min-max": f"{a['success'].min():.1%}-{a['success'].max():.1%}",
                     "primal integral mean": f"{a['pi'].mean():.4f}", "primal integral std": f"{a['pi'].std():.4f}",
                     "primal integral min-max": f"{a['pi'].min():.4f}-{a['pi'].max():.4f}"})
    return pd.DataFrame(rows)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("results_dir")
    parser.add_argument("out_dir")
    parser.add_argument("--limits", type=float, nargs="+", default=[60, 10])
    args = parser.parse_args()
    os.makedirs(args.out_dir, exist_ok=True)
    df = load(args.results_dir)
    lines = ["# Comparison with the state-of-the-art solvers", ""]
    lines.append(f"runs: {len(df)}, killed (rc != 0): {int((df['rc'] != 0).sum())}, "
                 f"solved but invalid: {int(((df['success'] == 1) & (df['valid'] == 0)).sum())}, "
                 f"reported SoC != validated SoC: {int(((df['solved'] == 1) & (df['soc'] != df['soc_reported'])).sum())}")
    per_solver = df.groupby("algo").size()
    lines.append("runs per solver: " + ", ".join(f"{a}: {per_solver.get(a, 0)}" for a in ALGOS))
    invalid = df[(df["success"] == 1) & (df["valid"] == 0)]
    for _, r in invalid.head(20).iterrows():
        lines.append(f"- invalid: {r['algo']} {r['map']} k={r['k']} scen={r['scen']} seed={r['seed']}: {r['reason']}")
    killed = df[df["rc"] != 0].groupby(["algo", "rc"]).size()
    if len(killed):
        lines.append("killed runs per solver and exit code: " + ", ".join(f"{a} rc={rc}: {n}" for (a, rc), n in killed.items()))
    lines.append("")
    for limit in args.limits:
        d = metrics(with_limit(df, limit))
        figures(d, limit, args.out_dir)
        lines.append(f"## Time limit {limit:g} s")
        lines.append("")
        lines.append("### All instances")
        lines.append(summary_table(d, []).to_markdown(index=False))
        lines.append("")
        for ours in ALGOS[:4]:
            lines.append(f"### {ours} head to head (final sum of costs, no solution is the worst)")
            lines.append(head_to_head(d, ours).to_markdown(index=False))
            lines.append("")
        lines.append("### Seeds")
        lines.append(seed_table(d).to_markdown(index=False))
        lines.append("")
        lines.append("### Per map")
        table = summary_table(d, ["map"])
        table["map"] = pd.Categorical(table["map"], MAP_ORDER)
        lines.append(table.sort_values(["map"], kind="stable").to_markdown(index=False))
        lines.append("")
    with open(f"{args.out_dir}/sota_report.md", "w") as f:
        f.write("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
