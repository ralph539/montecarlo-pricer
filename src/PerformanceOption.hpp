#pragma once

#include "Option.hpp"
#include "pnl/pnl_vector.h"

// Performance option on a weighted basket,
// payoff  1 + sum_{i=1}^{N} ( B_{t_i} / B_{t_{i-1}} - 1 )_+   with B = sum_d w_d S_d
// Only the positive period-over-period returns are collected. There is no
// strike, and the payoff is homogeneous of degree 0 in the path: scaling
// every price by the same factor leaves it unchanged, so the deltas are zero.
class PerformanceOption : public Option
{
    PnlVect *weights_;

public:
    PerformanceOption(double maturity, int nbTimeSteps, int size, const PnlVect *weights);
    ~PerformanceOption() override;

    double payoff(const PnlMat *path) const override;
};
