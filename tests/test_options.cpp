// Payoffs on hand-built paths, every expected value computed by hand.

#include <cassert>
#include <cmath>
#include <iostream>

#include "BasketOption.hpp"
#include "AsianOption.hpp"
#include "PerformanceOption.hpp"

static const double EPS = 1e-10;

static PnlMat *path(int rows, int cols, const double *values)
{
    return pnl_mat_create_from_ptr(rows, cols, values);
}

int main()
{
    // Vanilla call: one asset, strike 100, N = 1. Only S_T matters.
    {
        PnlVect *w = pnl_vect_create_from_scalar(1, 1.0);
        BasketOption call(1.0, 1, 1, 100.0, w);
        double up[] = {100.0, 120.0};
        double down[] = {100.0, 80.0};
        PnlMat *p1 = path(2, 1, up);
        PnlMat *p2 = path(2, 1, down);
        assert(std::fabs(call.payoff(p1) - 20.0) < EPS);
        assert(std::fabs(call.payoff(p2) - 0.0) < EPS);
        pnl_mat_free(&p1); pnl_mat_free(&p2); pnl_vect_free(&w);
    }

    // Basket of two: the middle row is deliberately absurd and must be ignored.
    {
        double wv[] = {0.5, 0.5};
        PnlVect *w = pnl_vect_create_from_ptr(2, wv);
        BasketOption basket(1.0, 2, 2, 100.0, w);
        double v[] = {100, 100, 999, 999, 120, 140};
        PnlMat *p = path(3, 2, v);
        assert(std::fabs(basket.payoff(p) - 30.0) < EPS);   // 0.5*120 + 0.5*140 - 100
        pnl_mat_free(&p); pnl_vect_free(&w);
    }

    // Asian: average over the three fixings, spot included.
    {
        PnlVect *w = pnl_vect_create_from_scalar(1, 1.0);
        AsianOption asian(1.0, 2, 1, 100.0, w);
        double v1[] = {100.0, 110.0, 120.0};   // mean 110
        double v2[] = {100.0, 90.0, 60.0};     // mean 83.3
        PnlMat *p1 = path(3, 1, v1);
        PnlMat *p2 = path(3, 1, v2);
        assert(std::fabs(asian.payoff(p1) - 10.0) < EPS);
        assert(std::fabs(asian.payoff(p2) - 0.0) < EPS);
        pnl_mat_free(&p1); pnl_mat_free(&p2); pnl_vect_free(&w);
    }

    // Performance: only positive returns count; a flat path pays exactly 1.
    {
        PnlVect *w = pnl_vect_create_from_scalar(1, 1.0);
        PerformanceOption perf(1.0, 2, 1, w);
        double v1[] = {100.0, 110.0, 99.0};    // 1 + 0.1 + 0
        double v2[] = {100.0, 100.0, 100.0};
        PnlMat *p1 = path(3, 1, v1);
        PnlMat *p2 = path(3, 1, v2);
        assert(std::fabs(perf.payoff(p1) - 1.1) < EPS);
        assert(std::fabs(perf.payoff(p2) - 1.0) < EPS);
        pnl_mat_free(&p1); pnl_mat_free(&p2); pnl_vect_free(&w);
    }

    // Performance is scale-invariant: multiplying the whole path changes nothing.
    {
        double wv[] = {0.5, 0.5};
        PnlVect *w = pnl_vect_create_from_ptr(2, wv);
        PerformanceOption perf(2.0, 3, 2, w);
        double v[] = {100, 100, 120, 90, 95, 130, 140, 110};
        PnlMat *p = path(4, 2, v);
        double ref = perf.payoff(p);
        pnl_mat_mult_scalar(p, 3.7);
        assert(std::fabs(perf.payoff(p) - ref) < EPS);
        pnl_mat_free(&p); pnl_vect_free(&w);
    }

    std::cout << "test_options: ok" << std::endl;
    return 0;
}
