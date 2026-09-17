#pragma once

#include <vector>
#include <iosfwd>
#include <nlohmann/json.hpp>

// One line of the hedging journal: what the pricer said at rebalancing date i,
// and what the replicating portfolio was worth.
struct Position
{
    int date;
    double price;
    double priceStdDev;
    std::vector<double> deltas;
    std::vector<double> deltasStdDev;
    double value;
};

void to_json(nlohmann::json &j, const Position &p);

struct Portfolio
{
    std::vector<Position> positions;
    double finalPayoff = 0.0;
    double finalPnL = 0.0;
};

std::ostream &operator<<(std::ostream &out, const Portfolio &portfolio);
