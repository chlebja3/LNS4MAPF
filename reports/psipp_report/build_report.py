"""Fill the tables of the PSIPP HTML report from the markdown outputs of paper_results.py and sota_results.py.

Usage: build_report.py <template.html> <paper report.md> <sota report.md> <out.html>
"""
import re
import sys

RP = "Ours (PSIPP + PSIPP repair)"
RPD = "Ours (PSIPP + direct PSIPP repair)"
RPDC = "Ours (PSIPP + direct PSIPP repair, cost 10)"
SOLVERS = [RP, RPD, RPDC, "Ours (PSIPP)", "Ours (SIPP, thesis)", "MAPF-LNS2", "MAPF-LNS", "BALANCE", "ADDRESS", "LaCAM3"]
OURS = (RP, RPD, RPDC, "Ours (PSIPP)")
SHORT = {RP: "PSIPP+rp", RPD: "PSIPP+rpd", RPDC: "PSIPP+rpd c10", "Ours (PSIPP)": "PSIPP", "Ours (SIPP, thesis)": "SIPP", "MAPF-LNS2": "LNS2", "MAPF-LNS": "LNS",
         "BALANCE": "BALANCE", "ADDRESS": "ADDRESS", "LaCAM3": "LaCAM3"}


def sections(text: str) -> dict[str, list[str]]:
    """Lines of the markdown under each '## limit' / '### table' heading pair, keyed '<limit>|<sub heading>'."""
    out, top, sub = {}, "", ""
    for line in text.split("\n"):
        if line.startswith("## "):
            top, sub = line[3:].strip(), ""
        elif line.startswith("### "):
            sub = line[4:].strip()
        out.setdefault(f"{top}|{sub}", []).append(line)
    return out


def table(lines: list[str]) -> list[dict[str, str]]:
    rows = [l for l in lines if l.startswith("|")]
    head = [c.strip() for c in rows[0].strip("|").split("|")]
    return [dict(zip(head, [c.strip() for c in r.strip("|").split("|")])) for r in rows[2:]]


def pct(cell: str) -> float:
    return float(re.search(r"\(([\d.]+)%", cell).group(1)) if "(" in cell else float(cell.rstrip("%"))


def bar(value: float, cls: str) -> str:
    return f'<span class="bar {cls}"><span style="width:{value:.1f}%"></span></span>'


def paper_map_table(rows: list[dict[str, str]]) -> str:
    out = ['<table class="data"><thead><tr><th>Map</th><th class="num">SIPP solved</th><th class="num">PSIPP solved</th>'
           '<th class="num">Only PSIPP</th><th class="num">Only SIPP</th><th class="num">Time PSIPP/SIPP</th>'
           '<th class="num">SoC PSIPP/SIPP</th></tr></thead><tbody>']
    for r in rows:
        s, p = pct(r["SIPP solved"]), pct(r["PSIPP solved"])
        t = float(r["time PSIPP/SIPP"].split()[0])
        soc = float(r["SoC PSIPP/SIPP"].split()[0])
        tcls = "good" if t < 0.95 else ("bad" if t > 1.02 else "")
        scls = "bad" if soc > 1.01 else ""
        out.append(f'<tr><td class="map">{r["map"]}</td><td class="num">{bar(s, "b-sipp")}{s:.1f}%</td>'
                   f'<td class="num">{bar(p, "b-psipp")}{p:.1f}%</td><td class="num">{r["only PSIPP"]}</td>'
                   f'<td class="num">{r["only SIPP"]}</td><td class="num {tcls}">{t:.2f}</td>'
                   f'<td class="num {scls}">{soc:.4f}</td></tr>')
    out.append("</tbody></table>")
    return "\n".join(out)


def sota_all_table(rows: list[dict[str, str]]) -> str:
    best = {"success": max(pct(r["success"]) for r in rows), "pi": min(float(r["primal integral"]) for r in rows),
            "gap": min(float(r["GAP common"]) for r in rows), "t": min(float(r["time first [s]"]) for r in rows)}
    out = ['<table class="data"><thead><tr><th>Solver</th><th class="num">Success</th>'
           '<th class="num">Mean time to first [s]</th><th class="num">Primal integral</th>'
           f'<th class="num">GAP to best (n={rows[0]["n common"]})</th></tr></thead><tbody>']
    for r in rows:
        s, pi, gap, t = pct(r["success"]), float(r["primal integral"]), float(r["GAP common"]), float(r["time first [s]"])
        mark = lambda ok: ' best' if ok else ''
        cls = ' class="ours"' if r["solver"] in OURS else ""
        out.append(f'<tr{cls}><td>{r["solver"]}</td><td class="num{mark(s == best["success"])}">{s:.1f}%</td>'
                   f'<td class="num{mark(t == best["t"])}">{t:.2f}</td>'
                   f'<td class="num{mark(pi == best["pi"])}">{pi:.3f}</td>'
                   f'<td class="num{mark(gap == best["gap"])}">{gap * 100:.2f}%</td></tr>')
    out.append("</tbody></table>")
    return "\n".join(out)


def h2h_table(rows: list[dict[str, str]], name: str) -> str:
    out = [f'<table class="data"><thead><tr><th>{name} vs</th><th class="num">{name} better</th><th class="num">Equal</th>'
           f'<th class="num">{name} worse</th><th class="num">Both unsolved</th><th>Share of decided</th></tr></thead><tbody>']
    for r in rows:
        b, w = int(r["better"]), int(r["worse"])
        share = 100 * b / (b + w)
        out.append(f'<tr><td>{r["vs"]}</td><td class="num">{b}</td><td class="num">{r["equal"]}</td>'
                   f'<td class="num">{w}</td><td class="num">{r["both unsolved"]}</td>'
                   f'<td><span class="split"><span style="width:{share:.1f}%"></span></span> {share:.0f}%</td></tr>')
    out.append("</tbody></table>")
    return "\n".join(out)


def sota_map_table(rows: list[dict[str, str]], metric: str) -> str:
    """Pivot map x solver of success rate or primal integral; the best value of each map is marked."""
    maps = list(dict.fromkeys(r["map"] for r in rows))
    val = {(r["map"], r["solver"]): (pct(r["success"]) if metric == "success" else float(r["primal integral"]))
           for r in rows}
    out = ['<table class="data pivot"><thead><tr><th>Map</th>'
           + "".join(f'<th class="num">{SHORT[s]}</th>' for s in SOLVERS) + "</tr></thead><tbody>"]
    for m in maps:
        vals = [val[(m, s)] for s in SOLVERS]
        best = max(vals) if metric == "success" else min(vals)
        cells = []
        for s, v in zip(SOLVERS, vals):
            txt = f"{v:.0f}%" if metric == "success" else f"{v:.3f}"
            cls = "num" + (" best" if v == best else "") + (" ourscol" if s in OURS else "")
            heat = v / 100 if metric == "success" else 1 - v
            cells.append(f'<td class="{cls}" style="--h:{heat:.3f}">{txt}</td>')
        out.append(f'<tr><td class="map">{m}</td>{"".join(cells)}</tr>')
    out.append("</tbody></table>")
    return "\n".join(out)


def seeds_table(rows: list[dict[str, str]]) -> str:
    out = ['<table class="data"><thead><tr><th>Solver</th><th class="num">Success, mean over seeds</th>'
           '<th class="num">std</th><th class="num">min–max</th><th class="num">Primal integral, mean</th>'
           '<th class="num">std</th><th class="num">min–max</th></tr></thead><tbody>']
    for r in rows:
        cls = ' class="ours"' if r["solver"] in OURS else ""
        out.append(f'<tr{cls}><td>{r["solver"]}</td><td class="num">{r["success mean"]}</td>'
                   f'<td class="num">{r["success std"]}</td><td class="num">{r["success min-max"].replace("-", "–")}</td>'
                   f'<td class="num">{r["primal integral mean"]}</td><td class="num">{r["primal integral std"]}</td>'
                   f'<td class="num">{r["primal integral min-max"].replace("-", "–")}</td></tr>')
    out.append("</tbody></table>")
    return "\n".join(out)


def main() -> None:
    template_path, paper_path, sota_path, out_path = sys.argv[1:5]
    with open(template_path) as f:
        html = f.read()
    with open(paper_path) as f:
        paper = sections(f.read())
    with open(sota_path) as f:
        sota = sections(f.read())
    for lim in ["60", "10"]:
        key = f"Time limit {lim} s"
        html = html.replace(f"{{{{PAPER_MAP_{lim}}}}}", paper_map_table(table(paper[f"{key}|Per map"])))
        html = html.replace(f"{{{{SOTA_ALL_{lim}}}}}", sota_all_table(table(sota[f"{key}|All instances"])))
        for tag, ours in [("RP", RP), ("RPD", RPD), ("RPDC", RPDC), ("", "Ours (PSIPP)")]:
            h2h = table(sota[f"{key}|{ours} head to head (final sum of costs, no solution is the worst)"])
            html = html.replace(f"{{{{SOTA_H2H{tag}_{lim}}}}}", h2h_table(h2h, SHORT[ours]))
        html = html.replace(f"{{{{SOTA_SEEDS_{lim}}}}}", seeds_table(table(sota[f"{key}|Seeds"])))
        map_rows = table(sota[f"{key}|Per map"])
        html = html.replace(f"{{{{SOTA_MAP_SUCCESS_{lim}}}}}", sota_map_table(map_rows, "success"))
        html = html.replace(f"{{{{SOTA_MAP_PI_{lim}}}}}", sota_map_table(map_rows, "pi"))
    assert "{{" not in html, "unfilled placeholder"
    with open(out_path, "w") as f:
        f.write(html)


if __name__ == "__main__":
    main()
