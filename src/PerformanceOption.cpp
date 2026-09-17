#include "PerformanceOption.hpp"

PerformanceOption::PerformanceOption(double maturity, int nbTimeSteps, int size, const PnlVect *weights)
    : Option(maturity, nbTimeSteps, size), weights_(pnl_vect_copy(weights))
{
}

PerformanceOption::~PerformanceOption()
{
    pnl_vect_free(&weights_);
}

double PerformanceOption::payoff(const PnlMat *path) const
{
    double previous = 0.0;
    for (int d = 0; d < size_; d++) {
        previous += GET(weights_, d) * MGET(path, 0, d);
    }

    double result = 1.0;
    for (int i = 1; i <= nbTimeSteps_; i++) {
        double current = 0.0;
        for (int d = 0; d < size_; d++) {
            current += GET(weights_, d) * MGET(path, i, d);
        }
        double ret = current / previous - 1.0;
        if (ret > 0.0) result += ret;
        previous = current;
    }
    return result;
}
