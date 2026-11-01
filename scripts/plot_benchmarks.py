#!/usr/bin/env python3
"""Create charts from benchmark_summary.csv; requires matplotlib."""
import csv
import os
import sys
from collections import defaultdict

import matplotlib.pyplot as plt

source = sys.argv[1] if len(sys.argv) > 1 else "benchmark_results/benchmark_summary.csv"
outdir = sys.argv[2] if len(sys.argv) > 2 else os.path.dirname(source) or "."
os.makedirs(outdir, exist_ok=True)

groups = defaultdict(list)
with open(source, newline="", encoding="utf-8-sig") as handle:
    for row in csv.DictReader(handle):
        groups[row["dataset"]].append(row)

if not groups:
    raise SystemExit("No benchmark rows found. Run scripts/benchmark.ps1 first.")

for dataset, rows in groups.items():
    labels = [r["query"] for r in rows]
    medians = [float(r["median_ms"]) for r in rows]
    p95s = [float(r["p95_ms"]) for r in rows]

    x = range(len(labels))
    width = 0.38
    fig, ax = plt.subplots(figsize=(max(8, len(labels) * 1.6), 5))
    ax.bar([i - width / 2 for i in x], medians, width, label="Median")
    ax.bar([i + width / 2 for i in x], p95s, width, label="p95")
    ax.set_title(f"IndexIt search timings – {dataset}")
    ax.set_ylabel("Wall-clock time (ms)")
    ax.set_xticks(list(x))
    ax.set_xticklabels(labels, rotation=20, ha="right")
    ax.legend()
    fig.tight_layout()
    path = os.path.join(outdir, f"benchmark_{dataset}.png")
    fig.savefig(path, dpi=160)
    plt.close(fig)
    print(path)
