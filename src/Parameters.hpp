#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

#include "pnl/pnl_vector.h"

class Option;
class BlackScholes;

// A scenario file. Vector fields (spot, volatility, weights) accept either one
// value per underlying or a single value that applies to all of them.
//
// {
//   "option":     { "type": "basket", "strike": 100, "maturity": 1.0,
//                   "fixingDates": 1, "weights": [0.5, 0.5] },
//   "model":      { "spot": [100], "volatility": [0.2], "correlation": 0.3, "rate": 0.03 },
//   "monteCarlo": { "samples": 50000, "fdStep": 0.1 },
//   "hedging":    { "rebalancingDates": 252 }
// }
struct Parameters
{
    std::string optionType;
    double strike = 0.0;
    double maturity;
    int fixingDates;
    std::vector<double> weights;

    int size;
    std::vector<double> spot;
    std::vector<double> volatility;
    double correlation;
    double rate;

    int samples;
    double fdStep;

    int rebalancingDates = 0;

    static Parameters load(const std::string &path);
    static Parameters parse(const nlohmann::json &j);

    std::unique_ptr<Option> makeOption() const;
    std::unique_ptr<BlackScholes> makeModel() const;
};

// A PnlVect built from a std::vector. The caller frees it.
PnlVect *toPnlVect(const std::vector<double> &v);
