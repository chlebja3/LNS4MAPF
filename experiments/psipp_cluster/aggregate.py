#!/usr/bin/env python3
"""Aggregate benchmark result files. Usage: aggregate.py <results_dir> | aggregate.py --selftest"""
import glob
import json
import os
import re
import statistics
import sys

NAME_RE = re.compile(r"^(?P<map>.+)_(?P<k>\d+)_(?P<variant>sipp|psipp_c[^_]+_cl[01]_sf[01])_(?P<seed>\d+)\.json$")
COUNTERS = ["fail_no_path", "fail_cycle", "pushed_replanned"]


def parse_name(name: str) -> dict:
    match = NAME_RE.match(name)
    assert match, f"unparsable file name {name}"
    return {"map": match["map"], "k": int(match["k"]), "variant": match["variant"], "seed": int(match["seed"])}


def make_row(name: str, text: str) -> dict:
    row = parse_name(name)
    try:
        data = json.loads(text)
    except json.JSONDecodeError:  # a run killed by timeout leaves a truncated file
        data = {}
    if "success" in data:
        row.update({key: value for key, value in data.items() if key not in row})
    else:
        row["success"] = 0
    return row


def load(paths: list[str]) -> list[dict]:
    rows = []
    for path in paths:
        with open(path) as f:
            rows.append(make_row(os.path.basename(path), f.read()))
    return rows


def median(values: list[float]) -> float | None:
    return statistics.median(values) if values else None


def mean(values: list[float]) -> float | None:
    return statistics.mean(values) if values else None


def summarize(rows: list[dict]) -> list[dict]:
    groups = {}
    for row in rows:
        groups.setdefault((row["map"], row["k"], row["variant"]), []).append(row)
    result = []
    for (m, k, variant), rs in sorted(groups.items()):
        out = {"map": m, "k": k, "variant": variant, "n": len(rs),
               "success_rate": sum(r["success"] for r in rs) / len(rs),
               "invalid": sum(1 for r in rs if r["success"] and not r.get("valid", 0)),
               "median_time": median([r["time"] for r in rs if "time" in r]),
               "median_attempts": median([r["attempts"] for r in rs if "attempts" in r])}
        for c in COUNTERS:
            out[c] = mean([r[c] for r in rs if c in r])
        result.append(out)
    return result


def fmt(value) -> str:
    return "-" if value is None else f"{value:.2f}"


def print_table(summary: list[dict]) -> None:
    print("map k variant n success med_time med_attempts invalid " + " ".join(COUNTERS))
    for s in summary:
        print(f"{s['map']} {s['k']} {s['variant']} {s['n']} {s['success_rate']:.2f} {fmt(s['median_time'])} "
              f"{fmt(s['median_attempts'])} {s['invalid']} " + " ".join(fmt(s[c]) for c in COUNTERS))


def selftest() -> None:
    assert parse_name("Paris_1_256_600_psipp_c10_cl1_sf0_3.json") == \
        {"map": "Paris_1_256", "k": 600, "variant": "psipp_c10_cl1_sf0", "seed": 3}
    assert parse_name("empty-32-32_350_sipp_25.json")["variant"] == "sipp"
    ok_line = ('{"map":"x.map","k":5,"success":1,"valid":1,"attempts":2,"time":1.5,'
               '"fail_no_path":1,"fail_cycle":0,"pushed_replanned":4}')
    rows = [make_row("x_5_sipp_1.json", ok_line), make_row("x_5_sipp_2.json", '{"map":"x.map",')]
    summary = summarize(rows)
    assert len(summary) == 1
    assert summary[0]["n"] == 2 and summary[0]["success_rate"] == 0.5 and summary[0]["invalid"] == 0
    assert summary[0]["median_time"] == 1.5 and summary[0]["pushed_replanned"] == 4
    print("OK")


def main() -> None:
    if sys.argv[1:] == ["--selftest"]:
        selftest()
    else:
        assert len(sys.argv) == 2, __doc__
        print_table(summarize(load(sorted(glob.glob(os.path.join(sys.argv[1], "*.json"))))))


if __name__ == "__main__":
    main()
