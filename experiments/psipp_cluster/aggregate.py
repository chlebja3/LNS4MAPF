#!/usr/bin/env python3
"""Aggregate benchmark result files.

Usage: aggregate.py <results_dir> [--expected-n N] [--time-limit S] | aggregate.py --selftest

A run is a <map>_<k>_<variant>_<seed>.json file plus an optional .rc file with the exit code of a failed run
(124 = timeout, anything else = crash). Timeouts, crashes and unparsable files count as failures.
"""
import argparse
import glob
import json
import os
import re
import statistics
import sys

NAME_RE = re.compile(r"^(?P<map>.+)_(?P<k>\d+)_(?P<variant>(?:sipp|psipp_c[^_]+_cl[01]_sf[01])(?:_o(?:random|short|long)(?:1st)?)?)_(?P<seed>\d+)\.json$")
COUNTERS = ["fail_no_path", "fail_cycle", "pushed_replanned"]
TIMEOUT_RC = 124


def parse_name(name: str) -> dict:
    match = NAME_RE.match(name)
    assert match, f"unparsable file name {name}"
    return {"map": match["map"], "k": int(match["k"]), "variant": match["variant"], "seed": int(match["seed"])}


def make_row(name: str, text: str, rc: int | None) -> dict:
    row = parse_name(name)
    lines = text.strip().splitlines()
    try:  # the result is the last line; the instance loader may print warnings before it
        data = json.loads(lines[-1]) if lines else {}
    except json.JSONDecodeError:  # a run killed by timeout leaves a truncated file
        data = {}
    row["parsed"] = isinstance(data, dict) and "success" in data
    if row["parsed"]:
        row.update({key: value for key, value in data.items() if key not in row})
    row["rc"] = rc
    row["timeout"] = rc == TIMEOUT_RC
    row["crash"] = rc is not None and rc != TIMEOUT_RC
    if rc is not None or not row["parsed"]:
        row["success"] = 0
    return row


def load(paths: list[str]) -> list[dict]:
    rows = []
    for path in paths:
        with open(path) as f:
            text = f.read()
        rc_path = path[:-len(".json")] + ".rc"
        rc = None
        if os.path.exists(rc_path):
            with open(rc_path) as f:
                rc = int(f.read().strip())
        rows.append(make_row(os.path.basename(path), text, rc))
    return rows


def median(values: list[float]) -> float | None:
    return statistics.median(values) if values else None


def mean(values: list[float]) -> float | None:
    return statistics.mean(values) if values else None


def summarize(rows: list[dict], default_time_limit: float = 60.0) -> list[dict]:
    groups = {}
    for row in rows:
        groups.setdefault((row["map"], row["k"], row["variant"]), []).append(row)
    result = []
    for (m, k, variant), rs in sorted(groups.items()):
        parsed = [r for r in rs if r["parsed"]]
        succ = [r for r in rs if r["success"]]
        valid_succ = [r for r in succ if r.get("valid", 0)]
        par2 = [r["time"] if r["success"] else 2 * r.get("time_limit", default_time_limit) for r in rs]
        out = {"map": m, "k": k, "variant": variant, "n": len(rs), "parsed": len(parsed),
               "succ_rate": len(valid_succ) / len(rs),
               "invalid": len(succ) - len(valid_succ),
               "timeouts": sum(r["timeout"] for r in rs), "crashes": sum(r["crash"] for r in rs),
               "med_time_succ": median([r["time"] for r in valid_succ]), "par2": mean(par2),
               "med_attempts": median([r["attempts"] for r in parsed if "attempts" in r])}
        for c in COUNTERS:
            out[c] = mean([r[c] for r in parsed if c in r])
        failures = sum(r.get("fail_no_path", 0) + r.get("fail_cycle", 0) for r in parsed)
        attempts = sum(r.get("attempts", 0) for r in parsed)
        out["cycle_frac"] = sum(r.get("fail_cycle", 0) for r in parsed) / failures if failures else None
        out["pushed_per_attempt"] = sum(r.get("pushed_replanned", 0) for r in parsed) / attempts if attempts else None
        result.append(out)
    return result


def check_completeness(summary: list[dict], expected_n: int) -> list[str]:
    warnings = []
    variants = {s["variant"] for s in summary}
    present = {(s["map"], s["k"]): set() for s in summary}
    for s in summary:
        present[(s["map"], s["k"])].add(s["variant"])
        if s["n"] != expected_n:
            warnings.append(f"WARNING: {s['map']} k={s['k']} {s['variant']} has n={s['n']}, expected {expected_n}")
    for (m, k), vs in sorted(present.items()):
        for v in sorted(variants - vs):
            warnings.append(f"WARNING: {m} k={k} is missing variant {v}")
    return warnings


def fmt(value: float | None) -> str:
    return "-" if value is None else f"{value:.2f}"


def print_table(summary: list[dict]) -> None:
    print("map k variant n parsed succ_rate invalid timeouts crashes med_time_succ par2 med_attempts(parsed) "
          + " ".join(f"mean_{c}(parsed)" for c in COUNTERS) + " cycle_frac pushed_per_attempt")
    for s in summary:
        print(f"{s['map']} {s['k']} {s['variant']} {s['n']} {s['parsed']} {s['succ_rate']:.2f} {s['invalid']} "
              f"{s['timeouts']} {s['crashes']} {fmt(s['med_time_succ'])} {fmt(s['par2'])} "
              f"{fmt(s['med_attempts'])} " + " ".join(fmt(s[c]) for c in COUNTERS)
              + f" {fmt(s['cycle_frac'])} {fmt(s['pushed_per_attempt'])}")


def selftest() -> None:
    assert parse_name("Paris_1_256_600_psipp_c10_cl1_sf0_3.json") == \
        {"map": "Paris_1_256", "k": 600, "variant": "psipp_c10_cl1_sf0", "seed": 3}
    assert parse_name("empty-32-32_350_sipp_25.json")["variant"] == "sipp"
    assert parse_name("ost003d_800_psipp_cinf_cl0_sf1_olong1st_7.json") == \
        {"map": "ost003d", "k": 800, "variant": "psipp_cinf_cl0_sf1_olong1st", "seed": 7}
    assert parse_name("ost003d_800_sipp_oshort_7.json")["variant"] == "sipp_oshort"
    ok_line = ('{"map":"x.map","k":5,"success":1,"valid":1,"attempts":2,"time":1.5,"time_limit":60,'
               '"fail_no_path":1,"fail_cycle":0,"pushed_replanned":4}')
    fail_line = ('{"map":"x.map","k":5,"success":0,"valid":0,"attempts":9,"time":60,"time_limit":60,'
                 '"fail_no_path":7,"fail_cycle":2,"pushed_replanned":0}')
    rows = [make_row("x_5_sipp_1.json", ok_line, None),
            make_row("x_5_sipp_2.json", '{"map":"x.map",', TIMEOUT_RC),
            make_row("x_5_sipp_3.json", "", 139),
            make_row("x_5_sipp_4.json", fail_line, None)]
    summary = summarize(rows)
    assert len(summary) == 1
    s = summary[0]
    assert s["n"] == 4 and s["parsed"] == 2 and s["succ_rate"] == 0.25 and s["invalid"] == 0
    assert s["timeouts"] == 1 and s["crashes"] == 1
    assert s["med_time_succ"] == 1.5
    assert s["par2"] == (1.5 + 120 + 120 + 120) / 4
    assert s["med_attempts"] == 5.5 and s["fail_no_path"] == 4 and s["pushed_replanned"] == 2
    # pooled: failures = 1 + 9 = 10 (cycles 2), attempts = 2 + 9 = 11, pushed = 4
    assert s["cycle_frac"] == 0.2 and s["pushed_per_attempt"] == 4 / 11
    invalid_line = ok_line.replace('"valid":1', '"valid":0')
    inv = summarize([make_row("x_5_sipp_1.json", ok_line, None), make_row("x_5_sipp_2.json", invalid_line, None)])[0]
    assert inv["succ_rate"] == 0.5 and inv["invalid"] == 1 and inv["med_time_succ"] == 1.5
    clean = summarize([make_row("x_5_sipp_1.json", ok_line.replace('"fail_no_path":1', '"fail_no_path":0'), None)])[0]
    assert clean["cycle_frac"] is None
    assert check_completeness(summary, 4) == [] and len(check_completeness(summary, 25)) == 1
    print("OK")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("results_dir", nargs="?")
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--expected-n", type=int, default=25)
    parser.add_argument("--time-limit", type=float, default=60.0)
    args = parser.parse_args()
    if args.selftest:
        selftest()
        return
    assert args.results_dir, "results_dir required"
    summary = summarize(load(sorted(glob.glob(os.path.join(args.results_dir, "*.json")))), args.time_limit)
    for warning in check_completeness(summary, args.expected_n):
        print(warning, file=sys.stderr)
    print_table(summary)


if __name__ == "__main__":
    main()
