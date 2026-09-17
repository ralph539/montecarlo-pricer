#include "Parameters.hpp"
#include "BasketOption.hpp"
#include "AsianOption.hpp"
#include "PerformanceOption.hpp"
#include "BlackScholes.hpp"

#include <fstream>
#include <stdexcept>

static std::vector<double> broadcast(std::vector<double> v, int size, const std::string &name)
{
    if (v.size() == 1 && size > 1) {
        v.assign(size, v[0]);
    }
    if (static_cast<int>(v.size()) != size) {
        throw std::invalid_argument(name + ": expected " + std::to_string(size) + " values, got " + std::to_string(v.size()));
    }
    return v;
}

Parameters Parameters::load(const std::string &path)
{
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("cannot open " + path);
    }
    return parse(nlohmann::json::parse(in));
}

Parameters Parameters::parse(const nlohmann::json &j)
{
    Parameters p;
    const auto &option = j.at("option");
    const auto &model = j.at("model");
    const auto &mc = j.at("monteCarlo");

    p.optionType = option.at("type").get<std::string>();
    p.maturity = option.at("maturity").get<double>();
    p.fixingDates = option.at("fixingDates").get<int>();
    if (option.contains("strike")) {
        p.strike = option.at("strike").get<double>();
    }

    p.size = model.at("size").get<int>();
    p.weights = broadcast(option.at("weights").get<std::vector<double>>(), p.size, "weights");
    p.spot = broadcast(model.at("spot").get<std::vector<double>>(), p.size, "spot");
    p.volatility = broadcast(model.at("volatility").get<std::vector<double>>(), p.size, "volatility");
    p.correlation = model.at("correlation").get<double>();
    p.rate = model.at("rate").get<double>();

    p.samples = mc.at("samples").get<int>();
    p.fdStep = mc.at("fdStep").get<double>();

    if (j.contains("hedging")) {
        p.rebalancingDates = j.at("hedging").at("rebalancingDates").get<int>();
    }
    return p;
}

PnlVect *toPnlVect(const std::vector<double> &v)
{
    return pnl_vect_create_from_ptr(static_cast<int>(v.size()), v.data());
}

std::unique_ptr<Option> Parameters::makeOption() const
{
    PnlVect *w = toPnlVect(weights);
    std::unique_ptr<Option> option;
    if (optionType == "basket") {
        option.reset(new BasketOption(maturity, fixingDates, size, strike, w));
    } else if (optionType == "asian") {
        option.reset(new AsianOption(maturity, fixingDates, size, strike, w));
    } else if (optionType == "performance") {
        option.reset(new PerformanceOption(maturity, fixingDates, size, w));
    }
    pnl_vect_free(&w);
    if (!option) {
        throw std::invalid_argument("unknown option type: " + optionType);
    }
    return option;
}

std::unique_ptr<BlackScholes> Parameters::makeModel() const
{
    PnlVect *s = toPnlVect(spot);
    PnlVect *v = toPnlVect(volatility);
    std::unique_ptr<BlackScholes> model(new BlackScholes(size, rate, maturity, fixingDates, s, v, correlation));
    pnl_vect_free(&s);
    pnl_vect_free(&v);
    return model;
}
