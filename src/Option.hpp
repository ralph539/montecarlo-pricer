#pragma once

#include "pnl/pnl_matrix.h"

// A European option whose payoff depends on the underlying observed on a
// regular grid of fixing dates t_k = k T / N, k = 0..N.
//
// The only thing an option knows how to do is turn a simulated path into a
// payoff. It knows nothing about the model that produced the path, nor about
// the Monte Carlo engine that averages the payoffs.
class Option
{
protected:
    double maturity_;   // T
    int nbTimeSteps_;   // N  (the path has N+1 rows)
    int size_;          // D  (number of underlyings, one per column)

    Option(double maturity, int nbTimeSteps, int size);

public:
    virtual ~Option() = default;

    Option(const Option &) = delete;
    Option &operator=(const Option &) = delete;

    // path: (N+1) x D matrix, row i = the D spots at fixing date t_i.
    // Returns the undiscounted payoff of that trajectory.
    virtual double payoff(const PnlMat *path) const = 0;

    double maturity() const { return maturity_; }
    int nbTimeSteps() const { return nbTimeSteps_; }
    int size() const { return size_; }
};
