// The Monte Carlo pricer against the Black-Scholes closed form, and the
// 1/sqrt(M) behaviour of its standard deviation.

#include <cassert>
#include <cmath>
#include <iostream>

#include "BasketOption.hpp"
#include "BlackScholes.hpp"
#include "MonteCarlo.hpp"
#include "pnl/pnl_finance.h"

int main()
{
    double S0 = 100.0, K = 100.0, T = 1.0, r = 0.05, sigma = 0.2;

    PnlVect *w = pnl_vect_create_from_scalar(1, 1.0);
    PnlVect *spot = pnl_vect_create_from_scalar(1, S0);
    PnlVect *vol = pnl_vect_create_from_scalar(1, sigma);
    BasketOption call(T, 1, 1, K, w);
    BlackScholes model(1, r, T, 1, spot, vol, 0.0);

    double reference = pnl_bs_call(S0, K, T, r, 0.0, sigma);

    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng, 0);

    // Price within 3 standard deviations of the closed form.
    {
        MonteCarlo mc(50000, 0.1, &call, &model);
        double price, sd;
        mc.price(nullptr, 0.0, rng, price, sd);
        std::cout << "closed form " << reference << "   monte carlo " << price << " +/- " << sd << std::endl;
        assert(sd > 0.0);
        assert(std::fabs(price - reference) < 3.0 * sd);

    }

    // Delta against N(d1). The central difference has an O(h^2) truncation
    // bias: at h = 10 % it is visible above the Monte Carlo noise, at h = 1 %
    // it is not.
    {
        double d1 = (std::log(S0 / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
        double deltaRef = pnl_cdfnor(d1);
        PnlVect *delta = pnl_vect_create(1);
        PnlVect *deltaSd = pnl_vect_create(1);

        MonteCarlo coarse(50000, 0.1, &call, &model);
        coarse.delta(nullptr, 0.0, rng, delta, deltaSd);
        double biasCoarse = GET(delta, 0) - deltaRef;
        std::cout << "delta N(d1) " << deltaRef << "   h=0.10: " << GET(delta, 0) << " +/- " << GET(deltaSd, 0) << std::endl;
        assert(std::fabs(biasCoarse) < 0.02);

        MonteCarlo fine(50000, 0.01, &call, &model);
        fine.delta(nullptr, 0.0, rng, delta, deltaSd);
        double biasFine = GET(delta, 0) - deltaRef;
        std::cout << "                       h=0.01: " << GET(delta, 0) << " +/- " << GET(deltaSd, 0) << std::endl;
        assert(std::fabs(biasFine) < 3.0 * GET(deltaSd, 0));
        assert(std::fabs(biasFine) < std::fabs(biasCoarse));

        pnl_vect_free(&delta);
        pnl_vect_free(&deltaSd);
    }

    // Standard deviation shrinks like 1/sqrt(M): x100 samples, ~/10 std dev.
    {
        MonteCarlo small(500, 0.1, &call, &model);
        MonteCarlo large(50000, 0.1, &call, &model);
        double p1, sd1, p2, sd2;
        small.price(nullptr, 0.0, rng, p1, sd1);
        large.price(nullptr, 0.0, rng, p2, sd2);
        double ratio = sd1 / sd2;
        std::cout << "std dev ratio M=500 / M=50000: " << ratio << " (expected ~10)" << std::endl;
        assert(ratio > 7.0 && ratio < 14.0);
    }

    pnl_rng_free(&rng);
    pnl_vect_free(&w);
    pnl_vect_free(&spot);
    pnl_vect_free(&vol);
    std::cout << "test_montecarlo: ok" << std::endl;
    return 0;
}
