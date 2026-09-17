#include "Portfolio.hpp"
#include <iomanip>
#include <ostream>

void to_json(nlohmann::json &j, const Position &p)
{
    j = {
        {"date", p.date},
        {"price", p.price},
        {"priceStdDev", p.priceStdDev},
        {"deltas", p.deltas},
        {"deltasStdDev", p.deltasStdDev},
        {"value", p.value},
    };
}

std::ostream &operator<<(std::ostream &out, const Portfolio &portfolio)
{
    nlohmann::json j = {
        {"portfolio", portfolio.positions},
        {"finalPayoff", portfolio.finalPayoff},
        {"finalPnL", portfolio.finalPnL},
    };
    out << std::setw(4) << j << std::endl;
    return out;
}
