#include "BlackScholes.hpp"
#include <cmath>

static const double EPS = 1e-10;

BlackScholes::BlackScholes(int size, double rate, double maturity, int nbTimeSteps,
                           const PnlVect *spot, const PnlVect *volatility, double correlation)
    : size_(size), rate_(rate), maturity_(maturity), nbTimeSteps_(nbTimeSteps),
      timeStep_(maturity / nbTimeSteps),
      spot_(pnl_vect_copy(spot)), volatility_(pnl_vect_copy(volatility)),
      cholesky_(pnl_mat_create(size, size)),
      gaussian_(pnl_vect_create(size)), correlated_(pnl_vect_create(size))
{
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            MLET(cholesky_, i, j) = (i == j) ? 1.0 : correlation;
        }
    }
    pnl_mat_chol(cholesky_);
}

BlackScholes::~BlackScholes()
{
    pnl_vect_free(&spot_);
    pnl_vect_free(&volatility_);
    pnl_mat_free(&cholesky_);
    pnl_vect_free(&gaussian_);
    pnl_vect_free(&correlated_);
}

int BlackScholes::lastFixingIndex(double t) const
{
    double k = t / timeStep_;
    int nearest = static_cast<int>(std::lround(k));
    if (std::fabs(k - nearest) < EPS) return nearest;
    return static_cast<int>(std::floor(k));
}

void BlackScholes::asset(const PnlMat *past, double t, PnlMat *path, PnlRng *rng)
{
    int index = lastFixingIndex(t);

    if (past == nullptr) {
        pnl_mat_set_row(path, spot_, 0);
    } else {
        for (int i = 0; i <= index; i++) {
            for (int d = 0; d < size_; d++) {
                MLET(path, i, d) = MGET(past, i, d);
            }
        }
    }

    // The first simulated step starts from the current spot and lasts until
    // the next fixing date; the following steps are full grid steps.
    double firstStep = (index + 1) * timeStep_ - t;

    for (int k = index; k < nbTimeSteps_; k++) {
        double dt = (k == index) ? firstStep : timeStep_;
        double sqrtDt = std::sqrt(dt);

        pnl_vect_rng_normal(gaussian_, size_, rng);
        pnl_mat_mult_vect_inplace(correlated_, cholesky_, gaussian_);

        for (int d = 0; d < size_; d++) {
            double sigma = GET(volatility_, d);
            double start = (k == index)
                ? ((past == nullptr) ? GET(spot_, d) : MGET(past, past->m - 1, d))
                : MGET(path, k, d);
            double exponent = (rate_ - 0.5 * sigma * sigma) * dt + sigma * sqrtDt * GET(correlated_, d);
            MLET(path, k + 1, d) = start * std::exp(exponent);
        }
    }
}

void BlackScholes::shiftAsset(PnlMat *path, int d, double h, double t) const
{
    int index = lastFixingIndex(t);
    bool onFixingDate = std::fabs(t - index * timeStep_) < EPS;
    int first = onFixingDate ? index : index + 1;
    for (int i = first; i <= nbTimeSteps_; i++) {
        MLET(path, i, d) *= (1.0 + h);
    }
}
