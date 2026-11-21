#!/usr/bin/env python3
"""Results of the SIPP vs PSIPP initial-solution experiment (gen_cmds.py --grid paper).

Reads <results>/<map>_<k>_<variant>_<scen>_<seed>.json (a <same basename>.rc marks a crashed or killed run), writes
figures in the style of the thesis (one panel per map, mean per number of agents) and markdown tables with the paired
statistics (McNemar on success, Wilcoxon signed-rank on time and sum of costs of instances solved by both planners).
Success under a shorter time limit is read from the same runs: PP with restarts stops at its first solution and a run
is deterministic given its seed, so a run with limit T solves the instance iff the 60 s run solved it within T.

Usage: paper_results.py <results_dir> <out_dir> [--limits 60 10]
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
from scipy import stats

VARIANTS = {"sipp_olong": "SIPP", "psipp_cinf_cl0_sf1_olong1st": "PSIPP"}
ALGOS = ["SIPP", "PSIPP"]
NAME_RE = re.compile(r"^(.+)_(\d+)_(" + "|".join(VARIANTS) + r")_(\d+)_(\d+)\.(json|rc)$")
MAP_ORDER = ["empty-32-32", "empty-48-48", "random-32-32-10", "random-32-32-20", "random-64-64-20", "maze-32-32-4",
             "maze-128-128-2", "room-32-32-4", "room-64-64-8", "warehouse-10-20-10-2-1", "warehouse-20-40-10-2-1",
             "den520d", "ost003d", "lak303d", "Paris_1_256"]
KEY = ["map", "k", "scen", "seed"]


def load(results_dir: str) -> pd.DataFrame:
    """One row per run; crashed/killed runs (rc file, missing or broken json) count as unsolved."""
    rows = {}
    for name in os.listdir(results_dir):
        m = NAME_RE.match(name)
        if not m:
            continue
        key = (m.group(1), int(m.group(2)), VARIANTS[m.group(3)], int(m.group(4)), int(m.group(5)))
        row = rows.setdefault(key, {"crashed": 0})
        path = os.path.join(results_dir, name)
        if m.group(6) == "rc":
            row["crashed"] = 1
            continue
        with open(path) as f:
            lines = [line for line in f.read().splitlines() if line.startswith("{")]
        if not lines:
            row["crashed"] = 1
            continue
        row.update(json.loads(lines[-1]))
    records = []
    for (map_name, k, algo, scen, seed), r in rows.items():
        solved = r.get("success", 0) == 1 and r["crashed"] == 0
        records.append({"map": map_name, "k": k, "algo": algo, "scen": scen, "seed": seed,
                        "crashed": r["crashed"], "solved": int(solved), "valid": int(r.get("valid", 0) == 1),
                        "time": r.get("time", math.nan), "soc": r.get("soc", math.nan),
                        "restarts": r.get("attempts", math.nan) - 1, "fail_cycle": r.get("fail_cycle", math.nan),
                        "fail_no_path": r.get("fail_no_path", math.nan)})
    return pd.DataFrame(records)


def with_limit(df: pd.DataFrame, limit: float) -> pd.DataFrame:
    """Results as if the time limit was `limit` seconds."""
    d = df.copy()
    d["solved"] = ((d["solved"] == 1) & (d["time"] <= limit)).astype(int)
    return d


def wilson(successes: int, n: int) -> tuple[float, float]:
    """95 % Wilson score interval of a success rate."""
    if n == 0:
        return math.nan, math.nan
    z = 1.959964
    p = successes / n
    centre = (p + z * z / (2 * n)) / (1 + z * z / n)
    half = z * math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / (1 + z * z / n)
    return centre - half, centre + half


def paired(df: pd.DataFrame) -> pd.DataFrame:
    """One row per instance (map, k, scen, seed) with the columns of both planners."""
    wide = df.pivot_table(index=KEY, columns="algo", values=["solved", "time", "soc", "restarts"], aggfunc="first")
    wide.columns = [f"{v}_{a}" for v, a in wide.columns]
    return wide.reset_index()


def mcnemar(p: pd.DataFrame) -> tuple[int, int, float]:
    """Exact McNemar test: (#only SIPP solved, #only PSIPP solved, two-sided p-value)."""
    only_s = int(((p["solved_SIPP"] == 1) & (p["solved_PSIPP"] == 0)).sum())
    only_p = int(((p["solved_SIPP"] == 0) & (p["solved_PSIPP"] == 1)).sum())
    pval = stats.binomtest(only_s, only_s + only_p, 0.5).pvalue if only_s + only_p > 0 else 1.0
    return only_s, only_p, pval


def ratio_ci(ratios: np.ndarray, rng: np.random.Generator) -> tuple[float, float, float]:
    """Geometric mean of PSIPP/SIPP ratios with a 95 % bootstrap interval."""
    logs = np.log(ratios)
    boot = rng.choice(logs, size=(2000, len(logs)), replace=True).mean(axis=1)
    return float(np.exp(logs.mean())), float(np.exp(np.percentile(boot, 2.5))), float(np.exp(np.percentile(boot, 97.5)))


def wilcoxon(a: np.ndarray, b: np.ndarray) -> float:
    if len(a) < 2 or np.all(a == b):
        return math.nan
    return float(stats.wilcoxon(a, b).pvalue)


def fmt_p(p: float) -> str:
    if math.isnan(p):
        return "–"
    return "<0.001" if p < 0.001 else f"{p:.3f}"


def stat_rows(df: pd.DataFrame, group: list[str], rng: np.random.Generator) -> pd.DataFrame:
    """Paired statistics per group (empty group = all instances)."""
    p = paired(df)
    groups = p.groupby(group) if group else [((), p)]
    rows = []
    for g, sub in groups:
        n = len(sub)
        row = dict(zip(group, g if isinstance(g, tuple) else (g,)))
        for a in ALGOS:
            s = int(sub[f"solved_{a}"].sum())
            lo, hi = wilson(s, n)
            row[f"{a} solved"] = f"{s}/{n} ({s / n:.1%}, CI {lo:.1%}–{hi:.1%})"
        only_s, only_p, pval = mcnemar(sub)
        row["only SIPP"] = only_s
        row["only PSIPP"] = only_p
        row["McNemar p"] = fmt_p(pval)
        both = sub[(sub["solved_SIPP"] == 1) & (sub["solved_PSIPP"] == 1)]
        row["both"] = len(both)
        if len(both) > 0:
            t_s, t_p = both["time_SIPP"].to_numpy(), both["time_PSIPP"].to_numpy()
            gm, lo, hi = ratio_ci(np.maximum(t_p, 1e-3) / np.maximum(t_s, 1e-3), rng)
            row["time PSIPP/SIPP"] = f"{gm:.2f} ({lo:.2f}–{hi:.2f})"
            row["time p"] = fmt_p(wilcoxon(t_s, t_p))
            c_s, c_p = both["soc_SIPP"].to_numpy(), both["soc_PSIPP"].to_numpy()
            gm, lo, hi = ratio_ci(c_p / c_s, rng)
            row["SoC PSIPP/SIPP"] = f"{gm:.4f} ({lo:.4f}–{hi:.4f})"
            row["SoC p"] = fmt_p(wilcoxon(c_s, c_p))
        rows.append(row)
    return pd.DataFrame(rows)


def per_map_grid(maps: list[str]) -> tuple[plt.Figure, np.ndarray]:
    ncols = 3
    nrows = math.ceil(len(maps) / ncols)
    fig, axes = plt.subplots(nrows=nrows, ncols=ncols, figsize=(9, 2.33 * nrows), sharey=False)
    return fig, axes.flatten()


def plot_metric(stats_df: pd.DataFrame, column: str, ylabel: str, path: str, ylim: tuple | None = None) -> None:
    """Thesis-style figure: one panel per map, the mean of `column` per number of agents for each planner."""
    maps = [m for m in MAP_ORDER if m in set(stats_df["map"])]
    fig, axes = per_map_grid(maps)
    for ax, map_name in zip(axes, maps):
        map_group = stats_df[stats_df["map"] == map_name]
        for algo in ALGOS:
            algo_group = map_group[map_group["algo"] == algo].sort_values("k")
            ax.plot(algo_group["k"], algo_group[column], marker="o", label=algo)
        ax.set_title(map_name)
        ax.set_xlabel("Number of Agents [-]")
        ax.set_ylabel(ylabel)
        if ylim is not None:
            ax.set_ylim(*ylim)
        ax.grid(True)
    for ax in axes[len(maps):]:
        ax.axis("off")
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="lower center", bbox_to_anchor=(0.5, 0.005), ncol=len(ALGOS))
    plt.tight_layout()
    plt.subplots_adjust(bottom=0.08)
    fig.savefig(path)
    plt.close(fig)


def common_solved(df: pd.DataFrame) -> pd.DataFrame:
    """Runs of instances solved by both planners (as in the thesis, quality and time are compared on these)."""
    solved = df[df["solved"] == 1]
    counts = solved.groupby(KEY)["algo"].transform("nunique")
    return solved[counts == len(ALGOS)]


def figures(df: pd.DataFrame, limit: float, out_dir: str) -> None:
    tag = f"{limit:g}s"
    success = df.groupby(["map", "k", "algo"]).agg(success=("solved", "mean")).reset_index()
    plot_metric(success, "success", "Success rate [-]", f"{out_dir}/success_rate_{tag}.pdf", (-0.05, 1.05))
    common = common_solved(df)
    by_k = common.groupby(["map", "k", "algo"]).agg(time=("time", "mean"), soc=("soc", "mean"),
                                                    restarts=("restarts", "mean")).reset_index()
    plot_metric(by_k, "time", "Time [s]", f"{out_dir}/time_{tag}.pdf")
    plot_metric(by_k, "soc", "Sum of costs [-]", f"{out_dir}/sum_of_costs_{tag}.pdf")
    plot_metric(by_k, "restarts", "Number of restarts [-]", f"{out_dir}/restarts_{tag}.pdf")
    # GAP to the better of the two planners on the commonly solved instances
    common = common.copy()
    common["BKS"] = common.groupby(KEY)["soc"].transform("min")
    common["GAP"] = (common["soc"] - common["BKS"]) / common["BKS"]
    gap = common.groupby(["map", "k", "algo"]).agg(gap=("GAP", "mean")).reset_index()
    plot_metric(gap, "gap", "GAP [-]", f"{out_dir}/gap_{tag}.pdf")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("results_dir")
    parser.add_argument("out_dir")
    parser.add_argument("--limits", type=float, nargs="+", default=[60, 10])
    args = parser.parse_args()
    os.makedirs(args.out_dir, exist_ok=True)
    rng = np.random.default_rng(0)
    df = load(args.results_dir)
    lines = ["# SIPP vs PSIPP: initial solution by prioritized planning with restarts", ""]
    lines.append(f"runs: {len(df)}, crashed/killed: {int(df['crashed'].sum())}, "
                 f"solved but invalid: {int(((df['solved'] == 1) & (df['valid'] == 0)).sum())}")
    expected = df.groupby(KEY)["algo"].nunique()
    lines.append(f"instances: {len(expected)}, instances missing a planner: {int((expected < len(ALGOS)).sum())}")
    lines.append("")
    for limit in args.limits:
        d = with_limit(df, limit)
        figures(d, limit, args.out_dir)
        lines.append(f"## Time limit {limit:g} s")
        lines.append("")
        lines.append("### All instances")
        lines.append(stat_rows(d, [], rng).to_markdown(index=False))
        lines.append("")
        lines.append("### Per map")
        lines.append(stat_rows(d, ["map"], rng).set_index("map").loc[
            [m for m in MAP_ORDER if m in set(d["map"])]].reset_index().to_markdown(index=False))
        lines.append("")
        lines.append("### Per map and number of agents (success rate SIPP / PSIPP)")
        sr = d.groupby(["map", "k", "algo"])["solved"].mean().unstack("algo").reset_index()
        sr["SIPP / PSIPP"] = sr.apply(lambda r: f"{r['SIPP']:.2f} / {r['PSIPP']:.2f}", axis=1)
        for map_name in [m for m in MAP_ORDER if m in set(sr["map"])]:
            sub = sr[sr["map"] == map_name]
            lines.append(f"- {map_name}: " + ", ".join(f"{k}: {v}" for k, v in zip(sub["k"], sub["SIPP / PSIPP"])))
        lines.append("")
    # restarts and failure causes (60 s)
    d = with_limit(df, max(args.limits))
    lines.append("## Restarts and failure causes")
    lines.append(d.groupby("algo").agg(mean_restarts_solved=("restarts", lambda s: s[d.loc[s.index, "solved"] == 1].mean()),
                                       mean_restarts_unsolved=("restarts", lambda s: s[d.loc[s.index, "solved"] == 0].mean()),
                                       fail_cycle=("fail_cycle", "sum"), fail_no_path=("fail_no_path", "sum"))
                 .reset_index().to_markdown(index=False))
    with open(f"{args.out_dir}/report.md", "w") as f:
        f.write("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
