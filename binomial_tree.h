#pragma once
#include <cmath>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include "Dividend.h"
using dividends::Dividend;
namespace binomial_tree {

    // Static pool avoids the overhead of repeated allocations during backtesting
    static thread_local std::vector<double> memory_pool;

    enum CallPut { CALL, PUT };
    enum OptionStyle { EUROPEAN, AMERICAN };

    inline double computeValue(
        const CallPut& callPut,
        const OptionStyle& optionStyle,
        const double& initialUnderlyingPrice,
        const double& continuousRate,
        const double& timeToExpiry,
        std::vector<Dividend*>* const dividends,
        const double& volatility,
        double strikePrice,
        long n) {

        // Dynamically grow the pool if N increases, but reuse memory otherwise.
        size_t requiredSize = static_cast<size_t>(n + 1) * 3;
        if (memory_pool.size() < requiredSize) {
            memory_pool.resize(requiredSize);
        }

        const double deltaT = timeToExpiry / static_cast<double>(n);
        const double u = std::exp(volatility * std::sqrt(deltaT));
        const double d = 1.0 / u;
        const double a = std::exp(continuousRate * deltaT);
        const double invA = 1.0 / a;
        const double pu = (a - d) / (u - d);
        const double pd = 1.0 - pu;

        // Pointers into our pre-allocated 'Arena'
        double* const s = &memory_pool[0];
        double* const o = &memory_pool[n + 1];
        double* const discDivArray = &memory_pool[2 * (n + 1)];

        for (long k = 0; k <= n; k++) discDivArray[k] = 0.0;

        // Handling the 'Escrowed Dividend' model
        if (dividends != nullptr) {
            for (auto* div : *dividends) {
                double tEx = div->getTimeToExDiv();
                if (tEx < timeToExpiry) {
                    double amt = div->getAmount();
                    long lastStep = static_cast<long>(tEx / deltaT);
                    for (long k = 0; k <= lastStep; k++) {
                        discDivArray[k] += amt * std::exp(-continuousRate * (tEx - k * deltaT));
                    }
                }
            }
        }

        // Price on the dividend-adjusted spot
        const double s0_adj = initialUnderlyingPrice - discDivArray[0];

        // Terminal step
        for (long i = 0; i <= n; i++) {
            s[i] = s0_adj * std::pow(u, n - 2.0 * i);
            double s_total = s[i] + discDivArray[n];
            o[i] = (callPut == CALL) ? std::max(s_total - strikePrice, 0.0)
                                     : std::max(strikePrice - s_total, 0.0);
        }

        // Backward Induction
        for (long i = n - 1; i >= 0; i--) {
            const double currentDiv = discDivArray[i];
            for (long j = 0; j <= i; j++) {
                o[j] = (o[j] * pu + o[j + 1] * pd) * invA;
                s[j] = (s[j] * pu + s[j + 1] * pd) * invA;

                if (optionStyle == AMERICAN) {
                    double s_total = s[j] + currentDiv;
                    double earlyPayoff = (callPut == CALL) ? (s_total - strikePrice) : (strikePrice - s_total);
                    if (earlyPayoff > o[j]) o[j] = earlyPayoff;
                }
            }
        }

        return o[0];
    }
}