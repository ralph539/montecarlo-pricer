#include "MonteCarlo.hpp"
#include "Option.hpp"
#include "BlackScholes.hpp"

#include <algorithm>
#include <cmath>

MonteCarlo::MonteCarlo(int nbSamples, double fdStep, Option *option, BlackScholes *model)
    : nbSamples_(nbSamples), fdStep_(fdStep), option_(option), model_(model),
      path_(pnl_mat_create(option->nbTimeSteps() + 1, option->size())),
      pathUp_(pnl_mat_create(option->nbTimeSteps() + 1, option->size())),
      pathDown_(pnl_mat_create(option->nbTimeSteps() + 1, option->size()))
{
}

MonteCarlo::~MonteCarlo()
{
    pnl_mat_free(&path_);
    pnl_mat_free(&pathUp_);
    pnl_mat_free(&pathDown_);
}

void MonteCarlo::price(const PnlMat *past, double t, PnlRng *rng, double &price, double &stdDev)
{
    double sum = 0.0;
    double sumSquares = 0.0;

    for (int j = 0; j < nbSamples_; j++) {
        model_->asset(past, t, path_, rng);
        double payoff = option_->payoff(path_);
        sum += payoff;
        sumSquares += payoff * payoff;
    }

    double mean = sum / nbSamples_;
    double variance = std::max(sumSquares / nbSamples_ - mean * mean, 0.0);
    double discount = std::exp(-model_->rate() * (option_->maturity() - t));

    price = discount * mean;
    stdDev = discount * std::sqrt(variance / nbSamples_);
}

void MonteCarlo::delta(const PnlMat *past, double t, PnlRng *rng, PnlVect *deltas, PnlVect *stdDevs)
{
    int size = option_->size();
    PnlVect *sum = pnl_vect_create_from_zero(size);
    PnlVect *sumSquares = pnl_vect_create_from_zero(size);

    for (int j = 0; j < nbSamples_; j++) {
        model_->asset(past, t, path_, rng);
        for (int d = 0; d < size; d++) {
            pnl_mat_clone(pathUp_, path_);
            pnl_mat_clone(pathDown_, path_);
            model_->shiftAsset(pathUp_, d, fdStep_, t);
            model_->shiftAsset(pathDown_, d, -fdStep_, t);
            double diff = option_->payoff(pathUp_) - option_->payoff(pathDown_);
            LET(sum, d) += diff;
            LET(sumSquares, d) += diff * diff;
        }
    }

    double discount = std::exp(-model_->rate() * (option_->maturity() - t));
    for (int d = 0; d < size; d++) {
        double spot = (past == nullptr) ? MGET(path_, 0, d) : MGET(past, past->m - 1, d);
        double factor = discount / (2.0 * spot * fdStep_);
        double mean = GET(sum, d) / nbSamples_;
        double variance = std::max(GET(sumSquares, d) / nbSamples_ - mean * mean, 0.0);
        LET(deltas, d) = factor * mean;
        LET(stdDevs, d) = factor * std::sqrt(variance / nbSamples_);
    }

    pnl_vect_free(&sum);
    pnl_vect_free(&sumSquares);
}
