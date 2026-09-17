#pragma once

#include "Option.hpp"
#include "pnl/pnl_vector.h"

// Call on a weighted basket, payoff  ( sum_d w_d S_{T,d} - K )_+
// Only the last fixing date matters. K <= 0 turns it into a put-like payoff.
class BasketOption : public Option
{
    double strike_;
    PnlVect *weights_;

public:
    BasketOption(double maturity, int nbTimeSteps, int size, double strike, const PnlVect *weights);
    ~BasketOption() override;

    double payoff(const PnlMat *path) const override;
};
