#pragma once

#include "pnl/pnl_matrix.h"
#include "pnl/pnl_vector.h"
#include "pnl/pnl_random.h"

// Multidimensional Black-Scholes model with a single correlation parameter:
//
//   S_{t,d} = S_{0,d} exp( (r - sigma_d^2/2) t + sigma_d (L W_t)_d )
//
// where W is a standard D-dimensional Brownian motion and L L' = Gamma,
// Gamma_ij = rho for i != j and 1 on the diagonal.
//
// The model simulates trajectories on the option's fixing grid t_k = k T / N.
// It knows nothing about options or payoffs.
class BlackScholes
{
    int size_;               // D
    double rate_;            // r
    double maturity_;        // T
    int nbTimeSteps_;        // N
    double timeStep_;        // T / N
    PnlVect *spot_;          // S_0, size D
    PnlVect *volatility_;    // sigma, size D
    PnlMat *cholesky_;       // L, D x D lower triangular
    PnlVect *gaussian_;      // work vector, one draw of D independent N(0,1)
    PnlVect *correlated_;    // work vector, L * gaussian_

    // Index of the last fixing date <= t, robust to floating-point noise.
    int lastFixingIndex(double t) const;

public:
    BlackScholes(int size, double rate, double maturity, int nbTimeSteps,
                 const PnlVect *spot, const PnlVect *volatility, double correlation);
    ~BlackScholes();

    BlackScholes(const BlackScholes &) = delete;
    BlackScholes &operator=(const BlackScholes &) = delete;

    // Fill `path` ((N+1) x D) with one trajectory on the fixing grid.
    //
    // At t = 0 pass past = nullptr: every row after the spot is simulated.
    // At t > 0, `past` holds the fixing dates already observed, with the
    // current spot S_t as its last row. Those rows are copied into `path`
    // and the remaining rows are simulated from S_t, the first step spanning
    // t_{i+1} - t and the following ones T/N.
    void asset(const PnlMat *past, double t, PnlMat *path, PnlRng *rng);

    // Multiply asset d by (1 + h) on every row that depends on the spot at
    // date t, i.e. the simulated rows. Rows that are observed fixings are
    // left untouched.
    void shiftAsset(PnlMat *path, int d, double h, double t) const;

    double rate() const { return rate_; }
    double maturity() const { return maturity_; }
    int nbTimeSteps() const { return nbTimeSteps_; }
    int size() const { return size_; }
};
