#pragma once

#include "pnl/pnl_matrix.h"
#include "pnl/pnl_vector.h"
#include "pnl/pnl_random.h"

class Option;
class BlackScholes;

// Monte Carlo pricer. Holds an option and a model but owns neither.
//
// price():  e^{-r(T-t)} * mean of M payoffs, with the standard deviation of
//           that estimator.
// delta():  central finite differences. Each of the M simulated trajectories
//           is reused for all D underlyings: the path is copied, the column
//           of asset d is shifted by +h and -h, and the two payoffs are
//           differenced. The same Brownian draw on both sides is what keeps
//           the estimator's variance small.
class MonteCarlo
{
    int nbSamples_;      // M
    double fdStep_;      // h
    Option *option_;
    BlackScholes *model_;

    PnlMat *path_;       // work matrices, allocated once
    PnlMat *pathUp_;
    PnlMat *pathDown_;

public:
    MonteCarlo(int nbSamples, double fdStep, Option *option, BlackScholes *model);
    ~MonteCarlo();

    MonteCarlo(const MonteCarlo &) = delete;
    MonteCarlo &operator=(const MonteCarlo &) = delete;

    // Price at date t given the observed past (nullptr at t = 0).
    void price(const PnlMat *past, double t, PnlRng *rng, double &price, double &stdDev);

    // Deltas at date t, one per underlying, with their standard deviations.
    void delta(const PnlMat *past, double t, PnlRng *rng, PnlVect *deltas, PnlVect *stdDevs);
};
