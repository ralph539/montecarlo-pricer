// The self-financing recursion on cases small enough to do by hand, and the
// grid helpers that extract fixing dates and the observed past from a market.

#include <cassert>
#include <cmath>
#include <iostream>

#include "Hedging.hpp"

static const double EPS = 1e-12;

int main()
{
    // One asset, one rebalancing, zero rate.
    // cash0 = 10 - 0.5*100 = -40; nothing changes; value1 = -40 + 0.5*110 = 15
    {
        PnlMat *market = pnl_mat_create(2, 1);
        MLET(market, 0, 0) = 100.0;
        MLET(market, 1, 0) = 110.0;
        PnlMat *deltas = pnl_mat_create_from_scalar(2, 1, 0.5);
        PnlVect *values = pnl_vect_create(2);
        portfolioValues(market, deltas, 10.0, 0.0, 1.0, 1, values);
        assert(std::fabs(GET(values, 0) - 10.0) < EPS);
        assert(std::fabs(GET(values, 1) - 15.0) < EPS);
        pnl_mat_free(&market); pnl_mat_free(&deltas); pnl_vect_free(&values);
    }

    // Rate 5 %, delta moves from 0.6 to 0.8, share at 120 on day 1.
    // cash0 = 10 - 60 = -50 ; cash1 = -50 e^0.05 - 0.2*120 ; value1 = cash1 + 0.8*120
    {
        PnlMat *market = pnl_mat_create(2, 1);
        MLET(market, 0, 0) = 100.0;
        MLET(market, 1, 0) = 120.0;
        PnlMat *deltas = pnl_mat_create(2, 1);
        MLET(deltas, 0, 0) = 0.6;
        MLET(deltas, 1, 0) = 0.8;
        PnlVect *values = pnl_vect_create(2);
        portfolioValues(market, deltas, 10.0, 0.05, 1.0, 1, values);
        double expected = -50.0 * std::exp(0.05) - 24.0 + 96.0;
        assert(std::fabs(GET(values, 0) - 10.0) < EPS);
        assert(std::fabs(GET(values, 1) - expected) < EPS);

        // P&L = value_H - payoff
        double pnl = profitAndLoss(market, deltas, 10.0, 0.05, 1.0, 1, 20.0);
        assert(std::fabs(pnl - (expected - 20.0)) < EPS);
        pnl_mat_free(&market); pnl_mat_free(&deltas); pnl_vect_free(&values);
    }

    // Grid helpers: H = 12 rebalancings, N = 3 fixings, so one fixing every 4 rows.
    {
        PnlMat *market = pnl_mat_create(13, 1);
        for (int i = 0; i <= 12; i++) MLET(market, i, 0) = 100.0 + i;

        PnlMat *fix = fixingDates(market, 3, 12);
        assert(fix->m == 4);
        assert(MGET(fix, 0, 0) == 100.0 && MGET(fix, 1, 0) == 104.0 && MGET(fix, 2, 0) == 108.0 && MGET(fix, 3, 0) == 112.0);
        pnl_mat_free(&fix);

        PnlMat *p0 = observedPast(market, 0, 3, 12);    // day 0: just the spot
        PnlMat *p5 = observedPast(market, 5, 3, 12);    // day 5: fixings 0,4 then today
        PnlMat *p8 = observedPast(market, 8, 3, 12);    // day 8: on a fixing, no duplicate
        assert(p0->m == 1 && MGET(p0, 0, 0) == 100.0);
        assert(p5->m == 3 && MGET(p5, 1, 0) == 104.0 && MGET(p5, 2, 0) == 105.0);
        assert(p8->m == 3 && MGET(p8, 2, 0) == 108.0);
        pnl_mat_free(&p0); pnl_mat_free(&p5); pnl_mat_free(&p8);
        pnl_mat_free(&market);
    }

    std::cout << "test_hedging: ok" << std::endl;
    return 0;
}
