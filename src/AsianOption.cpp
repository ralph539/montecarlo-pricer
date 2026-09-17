#include "AsianOption.hpp"
#include <algorithm>

AsianOption::AsianOption(double maturity, int nbTimeSteps, int size, double strike, const PnlVect *weights)
    : Option(maturity, nbTimeSteps, size), strike_(strike), weights_(pnl_vect_copy(weights))
{
}

AsianOption::~AsianOption()
{
    pnl_vect_free(&weights_);
}

double AsianOption::payoff(const PnlMat *path) const
{
    double sum = 0.0;
    for (int i = 0; i <= nbTimeSteps_; i++) {
        for (int d = 0; d < size_; d++) {
            sum += GET(weights_, d) * MGET(path, i, d);
        }
    }
    return std::max(sum / (nbTimeSteps_ + 1) - strike_, 0.0);
}
