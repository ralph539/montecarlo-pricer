#include "BasketOption.hpp"
#include <algorithm>

BasketOption::BasketOption(double maturity, int nbTimeSteps, int size, double strike, const PnlVect *weights)
    : Option(maturity, nbTimeSteps, size), strike_(strike), weights_(pnl_vect_copy(weights))
{
}

BasketOption::~BasketOption()
{
    pnl_vect_free(&weights_);
}

double BasketOption::payoff(const PnlMat *path) const
{
    double basket = 0.0;
    for (int d = 0; d < size_; d++) {
        basket += GET(weights_, d) * MGET(path, nbTimeSteps_, d);
    }
    return std::max(basket - strike_, 0.0);
}
