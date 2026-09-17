#!/usr/bin/env python3
"""Parameter sweeps on the vanilla call, for the convergence and bias plots.

Reprices scenarios/call_1d with a range of sample counts and a range of
finite-difference steps, and writes results/sweep_samples.csv and
results/sweep_fdstep.csv.

    python3 tools/sweep.py [build_dir]
"""
import csv
import json
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / (sys.argv[1] if len(sys.argv) > 1 else "build")
BASE = json.loads((ROOT / "scenarios/call_1d/params.json").read_text())


def price(samples, fd_step):
    params = json.loads(json.dumps(BASE))
    params["monteCarlo"]["samples"] = samples
    params["monteCarlo"]["fdStep"] = fd_step
    with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False) as f:
        json.dump(params, f)
        tmp = f.name
    start = time.perf_counter()
    out = subprocess.run([str(BUILD / "price"), tmp], capture_output=True, text=True, check=True).stdout
    elapsed = time.perf_counter() - start
    Path(tmp).unlink()
    r = json.loads(out)
    return r["price"], r["priceStdDev"], r["delta"][0], r["deltaStdDev"][0], elapsed


def write(name, header, rows):
    out = ROOT / "results" / name
    with out.open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow(header)
        w.writerows(rows)
    print(f"{out}: {len(rows)} rows")


if __name__ == "__main__":
    rows = []
    for M in [500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000]:
        p, sd, d, dsd, t = price(M, 0.01)
        rows.append([M, p, sd, d, dsd, round(t, 3)])
        print(f"  M={M:>7}  price {p:.4f} +/- {sd:.4f}   {t:.2f}s")
    write("sweep_samples.csv", ["samples", "price", "priceStdDev", "delta", "deltaStdDev", "seconds"], rows)

    rows = []
    for h in [0.5, 0.3, 0.2, 0.1, 0.05, 0.02, 0.01, 0.005]:
        p, sd, d, dsd, t = price(200000, h)
        rows.append([h, d, dsd])
        print(f"  h={h:<6} delta {d:.5f} +/- {dsd:.5f}")
    write("sweep_fdstep.csv", ["fdStep", "delta", "deltaStdDev"], rows)
