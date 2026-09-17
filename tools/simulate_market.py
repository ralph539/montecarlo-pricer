#!/usr/bin/env python3
"""Simulate a market path for a scenario and write it next to the scenario file.

The path is a correlated geometric Brownian motion on the rebalancing grid,
under the real-world drift given in the scenario's "market" block (which the
pricer never reads). It is written as a plain (H+1) x D matrix, one
rebalancing date per row, that `hedge` loads with pnl_mat_create_from_file.

    python3 tools/simulate_market.py scenarios/call_1d
"""
import json
import sys
from pathlib import Path

import numpy as np


def broadcast(values, size):
    values = list(values)
    return values * size if len(values) == 1 else values


def simulate(params):
    D = params["model"]["size"]
    spot = np.array(broadcast(params["model"]["spot"], D))
    vol = np.array(broadcast(params["model"]["volatility"], D))
    rho = params["model"]["correlation"]
    T = params["option"]["maturity"]
    H = params["hedging"]["rebalancingDates"]
    drift = np.array(broadcast(params["market"]["drift"], D))
    rng = np.random.default_rng(params["market"]["seed"])

    corr = np.full((D, D), rho) + (1.0 - rho) * np.eye(D)
    L = np.linalg.cholesky(corr)
    dt = T / H

    path = np.empty((H + 1, D))
    path[0] = spot
    for i in range(1, H + 1):
        g = L @ rng.standard_normal(D)
        path[i] = path[i - 1] * np.exp((drift - 0.5 * vol**2) * dt + vol * np.sqrt(dt) * g)
    return path


def main(scenario_dir):
    scenario_dir = Path(scenario_dir)
    params = json.loads((scenario_dir / "params.json").read_text())
    path = simulate(params)
    out = scenario_dir / "market.txt"
    header = f"# {scenario_dir.name}: {path.shape[0]} dates x {path.shape[1]} assets"
    np.savetxt(out, path, fmt="%.6f", header=header, comments="")
    print(f"{out}: {path.shape[0]} x {path.shape[1]}, final spots {np.round(path[-1], 2)}")


if __name__ == "__main__":
    for d in sys.argv[1:]:
        main(d)
