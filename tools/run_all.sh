#!/usr/bin/env bash
# Price and hedge every scenario, writing the JSON outputs and timings to results/.
#
#   tools/run_all.sh [build_dir]

set -euo pipefail
cd "$(dirname "$0")/.."
BUILD=${1:-build}

mkdir -p results
: > results/timings.csv
echo "scenario,price_seconds,hedge_seconds" >> results/timings.csv

for dir in scenarios/*/; do
    name=$(basename "$dir")
    echo "== $name"

    start=$(date +%s.%N)
    "$BUILD/price" "$dir/params.json" > "results/${name}_price.json"
    t_price=$(echo "$(date +%s.%N) - $start" | bc)

    start=$(date +%s.%N)
    "$BUILD/hedge" "$dir/params.json" "$dir/market.txt" > "results/${name}_hedge.json"
    t_hedge=$(echo "$(date +%s.%N) - $start" | bc)

    printf "%s,%.2f,%.2f\n" "$name" "$t_price" "$t_hedge" >> results/timings.csv
    printf "   price %.2fs   hedge %.2fs\n" "$t_price" "$t_hedge"
done
