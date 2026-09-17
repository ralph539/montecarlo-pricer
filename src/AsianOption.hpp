#pragma once

#include "Option.hpp"
#include "pnl/pnl_vector.h"

// Discrete Asian call on a weighted basket,
// payoff  ( 1/(N+1) sum_i sum_d w_d S_{t_i,d} - K )_+
// The average runs over all N+1 fixing dates, spot included.
class AsianOption : public Option
{
    double strike_;
    PnlVect *weights_;

public:
    AsianOption(double maturity, int nbTimeSteps, int size, double strike, const PnlVect *weights);
    ~AsianOption() override;

    double payoff(const PnlMat *path) const override;
};
