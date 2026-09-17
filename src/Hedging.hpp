#pragma once

#include "pnl/pnl_matrix.h"
#include "pnl/pnl_vector.h"

// Self-financing replication along a market path.
//
//   V_0 = p_0 - delta_0 . S_0
//   V_i = V_{i-1} e^{r T / H} - (delta_i - delta_{i-1}) . S_i        i = 1..H
//   value_i = V_i + delta_i . S_i
//
// V is the cash account: it earns the risk-free rate between two rebalancing
// dates and pays for every change in the share holdings. The premium p_0 is
// spent once, on day 0; after that the portfolio finances itself.
//
// market and deltas are (H+1) x D matrices, one row per rebalancing date.
void portfolioValues(const PnlMat *market, const PnlMat *deltas,
                     double premium, double rate, double maturity, int nbRebalancings,
                     PnlVect *values);

// Hedging error at maturity:  value_H - payoff.
double profitAndLoss(const PnlMat *market, const PnlMat *deltas,
                     double premium, double rate, double maturity, int nbRebalancings,
                     double payoff);

// Rows of `market` that fall on the option's fixing grid (every H/N rows),
// as an (N+1) x D matrix the option's payoff can consume.
PnlMat *fixingDates(const PnlMat *market, int nbTimeSteps, int nbRebalancings);

// What is known at rebalancing date i: the fixing dates already observed,
// plus the current spot as last row when i is not itself a fixing date.
PnlMat *observedPast(const PnlMat *market, int i, int nbTimeSteps, int nbRebalancings);
