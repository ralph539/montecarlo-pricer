// hedge <scenario.json> <market.txt>
//
// Replay a market path day by day. At every rebalancing date the option is
// repriced and re-hedged given everything observed so far; the self-financing
// portfolio is then rolled forward on the real prices. Prints the full hedging
// journal and the final P&L as JSON on stdout.

#include <iostream>
#include <vector>

#include "Parameters.hpp"
#include "Option.hpp"
#include "BlackScholes.hpp"
#include "MonteCarlo.hpp"
#include "Hedging.hpp"
#include "Portfolio.hpp"

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "usage: hedge <scenario.json> <market.txt>" << std::endl;
        return 1;
    }

    Parameters params = Parameters::load(argv[1]);
    PnlMat *market = pnl_mat_create_from_file(argv[2]);

    int H = params.rebalancingDates;
    int N = params.fixingDates;
    int D = params.size;
    if (market->m != H + 1 || market->n != D || H % N != 0) {
        std::cerr << "market file does not match the scenario: expected "
                  << H + 1 << " x " << D << " with H multiple of N" << std::endl;
        pnl_mat_free(&market);
        return 1;
    }

    auto option = params.makeOption();
    auto model = params.makeModel();
    MonteCarlo mc(params.samples, params.fdStep, option.get(), model.get());

    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng, 0);

    // Pass 1: price and deltas at every rebalancing date.
    PnlVect *prices = pnl_vect_create(H + 1);
    PnlVect *priceStdDevs = pnl_vect_create(H + 1);
    PnlMat *deltas = pnl_mat_create(H + 1, D);
    PnlMat *deltaStdDevs = pnl_mat_create(H + 1, D);
    PnlVect *d = pnl_vect_create(D);
    PnlVect *dsd = pnl_vect_create(D);

    for (int i = 0; i <= H; i++) {
        double t = i * params.maturity / H;
        PnlMat *past = observedPast(market, i, N, H);
        double p, sd;
        mc.price(past, t, rng, p, sd);
        mc.delta(past, t, rng, d, dsd);
        LET(prices, i) = p;
        LET(priceStdDevs, i) = sd;
        pnl_mat_set_row(deltas, d, i);
        pnl_mat_set_row(deltaStdDevs, dsd, i);
        pnl_mat_free(&past);
    }

    // Pass 2: roll the self-financing portfolio on the real prices.
    PnlVect *values = pnl_vect_create(H + 1);
    portfolioValues(market, deltas, GET(prices, 0), params.rate, params.maturity, H, values);

    PnlMat *fixings = fixingDates(market, N, H);
    double payoff = option->payoff(fixings);
    pnl_mat_free(&fixings);

    Portfolio portfolio;
    portfolio.positions.reserve(H + 1);
    for (int i = 0; i <= H; i++) {
        Position pos;
        pos.date = i;
        pos.price = GET(prices, i);
        pos.priceStdDev = GET(priceStdDevs, i);
        pos.deltas.assign(deltas->array + i * D, deltas->array + (i + 1) * D);
        pos.deltasStdDev.assign(deltaStdDevs->array + i * D, deltaStdDevs->array + (i + 1) * D);
        pos.value = GET(values, i);
        portfolio.positions.push_back(std::move(pos));
    }
    portfolio.finalPayoff = payoff;
    portfolio.finalPnL = GET(values, H) - payoff;

    std::cout << portfolio;

    pnl_vect_free(&values);
    pnl_vect_free(&prices);
    pnl_vect_free(&priceStdDevs);
    pnl_mat_free(&deltas);
    pnl_mat_free(&deltaStdDevs);
    pnl_vect_free(&d);
    pnl_vect_free(&dsd);
    pnl_rng_free(&rng);
    pnl_mat_free(&market);
    return 0;
}
