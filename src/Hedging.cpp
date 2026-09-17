#include "Hedging.hpp"
#include <cmath>

static double dot(const PnlMat *deltas, const PnlMat *market, int i)
{
    double s = 0.0;
    for (int d = 0; d < market->n; d++) {
        s += MGET(deltas, i, d) * MGET(market, i, d);
    }
    return s;
}

void portfolioValues(const PnlMat *market, const PnlMat *deltas,
                     double premium, double rate, double maturity, int nbRebalancings,
                     PnlVect *values)
{
    double shares = dot(deltas, market, 0);
    double cash = premium - shares;
    LET(values, 0) = cash + shares;

    double accrual = std::exp(rate * maturity / nbRebalancings);
    for (int i = 1; i <= nbRebalancings; i++) {
        cash *= accrual;
        for (int d = 0; d < market->n; d++) {
            cash -= (MGET(deltas, i, d) - MGET(deltas, i - 1, d)) * MGET(market, i, d);
        }
        LET(values, i) = cash + dot(deltas, market, i);
    }
}

double profitAndLoss(const PnlMat *market, const PnlMat *deltas,
                     double premium, double rate, double maturity, int nbRebalancings,
                     double payoff)
{
    PnlVect *values = pnl_vect_create(nbRebalancings + 1);
    portfolioValues(market, deltas, premium, rate, maturity, nbRebalancings, values);
    double pnl = GET(values, nbRebalancings) - payoff;
    pnl_vect_free(&values);
    return pnl;
}

PnlMat *fixingDates(const PnlMat *market, int nbTimeSteps, int nbRebalancings)
{
    int step = nbRebalancings / nbTimeSteps;
    PnlMat *fixings = pnl_mat_create(nbTimeSteps + 1, market->n);
    for (int k = 0; k <= nbTimeSteps; k++) {
        for (int d = 0; d < market->n; d++) {
            MLET(fixings, k, d) = MGET(market, k * step, d);
        }
    }
    return fixings;
}

PnlMat *observedPast(const PnlMat *market, int i, int nbTimeSteps, int nbRebalancings)
{
    int step = nbRebalancings / nbTimeSteps;
    int index = i / step;
    bool onFixingDate = (i % step == 0);

    int rows = onFixingDate ? index + 1 : index + 2;
    PnlMat *past = pnl_mat_create(rows, market->n);
    for (int k = 0; k <= index; k++) {
        for (int d = 0; d < market->n; d++) {
            MLET(past, k, d) = MGET(market, k * step, d);
        }
    }
    if (!onFixingDate) {
        for (int d = 0; d < market->n; d++) {
            MLET(past, index + 1, d) = MGET(market, i, d);
        }
    }
    return past;
}
