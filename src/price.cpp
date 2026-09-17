// price <scenario.json>
//
// Price and deltas of the option at t = 0, with the standard deviation of
// each Monte Carlo estimator, as JSON on stdout.

#include <iostream>
#include <iomanip>
#include <vector>
#include <nlohmann/json.hpp>

#include "Parameters.hpp"
#include "Option.hpp"
#include "BlackScholes.hpp"
#include "MonteCarlo.hpp"

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "usage: price <scenario.json>" << std::endl;
        return 1;
    }

    Parameters params = Parameters::load(argv[1]);
    auto option = params.makeOption();
    auto model = params.makeModel();
    MonteCarlo mc(params.samples, params.fdStep, option.get(), model.get());

    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng, 0);

    double price, priceStdDev;
    mc.price(nullptr, 0.0, rng, price, priceStdDev);

    PnlVect *deltas = pnl_vect_create(params.size);
    PnlVect *deltasStdDev = pnl_vect_create(params.size);
    mc.delta(nullptr, 0.0, rng, deltas, deltasStdDev);

    nlohmann::json out = {
        {"price", price},
        {"priceStdDev", priceStdDev},
        {"delta", std::vector<double>(deltas->array, deltas->array + deltas->size)},
        {"deltaStdDev", std::vector<double>(deltasStdDev->array, deltasStdDev->array + deltasStdDev->size)},
    };
    std::cout << std::setw(4) << out << std::endl;

    pnl_vect_free(&deltas);
    pnl_vect_free(&deltasStdDev);
    pnl_rng_free(&rng);
    return 0;
}
