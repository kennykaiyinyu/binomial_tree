#pragma once

#include <math.h>
#include <algorithm>    // std::max
#include <iostream>
#include <iomanip>      // std::setprecision
#include <vector>
#include <cassert>
#include <chrono>


using namespace std;

namespace dividends {
	struct Dividend {
		double amount;
		double timeToExDiv;
	};
}

using Dividend = dividends::Dividend;

namespace binomial_tree {

	static thread_local std::vector<double> memory_pool(1000000);

	enum CallPut {CALL, PUT};
	enum OptionStyle {EUROPEAN, AMERICAN};

	template <CallPut CP, OptionStyle OS>
	double computeValue(
		// const CallPut& callPut,
		// const OptionStyle& optionStyle,
		const double& initialUnderlyingPrice,
		const double& continuousRate,
		const double& timeToExpiry,
		const vector<Dividend> & dividends,
		const double& volatility,
		double strikePrice,
		long n /* steps */) {

			if (CP != CALL && CP != PUT)  {
				throw runtime_error("must be call or put!");
			}

			if (OS != EUROPEAN && OS != AMERICAN)  {
				throw runtime_error("must be European or American!");
			}

			const double deltaT = timeToExpiry / (.0 + n);
			const double u = exp(volatility * sqrt(deltaT));
			const double d = 1 / u;
			const double dSquare = d * d;
			const double p = 1 / u;
			const double a = exp(continuousRate * deltaT) ; // growth rate at each step (i.e. 1 / discount factor)
			const double pu = (a - d) / (u - d);
			const double pd = 1 - pu;
			const auto pu_div_a = pu / a;
			const auto pd_div_a = pd / a;

			if (memory_pool.size() < 3*n + 3) {
				throw runtime_error("memory pool must be at least 3 * n + 3");
			}

			double* const s = &memory_pool[0];	// The tree leaves of the underlying (with dividend excluded)
			double* const o = &memory_pool[n + 1];	// option values
			double* const discountedDividendAtTime = &memory_pool[2*n+2]; // the discountedDividendValueAtSteps[0, 1, 2, ..., n]

			//
			// Forward process
			// For simplicity, we just compute the stock prices at the leaves of the tree
			// I.e., no intermediate levels of the tree is kept.
			//
			//
			//
			//                                         s[0]
			//                       s[0]
			//    s[0]   ---->               ---->     s[1]
			//                       s[1]
			//                                         s[2]
			//

			for (long k = 0; k <= n; k++) {
				discountedDividendAtTime[k]  = .0;
			}
			for (auto & div : dividends) {
				double timeToExDiv = div.timeToExDiv;
				if (timeToExDiv < timeToExpiry) {
					double dividendAmount = div.amount;
					long lastStepBeforeExDiv = static_cast<long>(timeToExDiv / deltaT);
					for (long k = 0; k <= lastStepBeforeExDiv; k++) {
						discountedDividendAtTime[k] += dividendAmount * exp(-continuousRate *( timeToExDiv - k * deltaT));
					}
				}
			}

			const double initialUnderlyingPriceExcludedDividend = initialUnderlyingPrice - discountedDividendAtTime[0];

			long i, j;

			// at expiry (time t == n * deltaT), calcaulte the option at the tree-leaves
			for (i = 0; i <= n; i++) {
				if (i == 0) {
					s[i] = initialUnderlyingPriceExcludedDividend * pow(u, n);
				} else {
					s[i] = s[i-1] * dSquare;
				}

				const double underlyingPriceAdjustedByDividend = s[i] + discountedDividendAtTime[n];

				if (CP == CALL) {
					o[i] = max(underlyingPriceAdjustedByDividend - strikePrice, .0);
				} else  {	// callPut == PUT
					o[i] = max(strikePrice - underlyingPriceAdjustedByDividend, .0);
				}
			}

			// backward process

			for (i = n - 1; i >= 0; i--)  {
				for (j = 0; j <= i; j++) {
					/*
					 * (a) Instead of doing s[j] := (s[j] * pu + s[j + 1] * pd) / a
					 *		we can have optimization where:
					 *
					 *		Denote S_{i, j} to be price of stock at step i and index j.
					 *
					 *		note that
					 *			S_{i+1, j} = S_{i, j} * u
					 *		=>	S_{i, j} = S_{i, j} / u = S_{i, j}
					 *
					 * (b) Instead of doing o[j] := (o[j] * pu + o[j + 1] * pd) / a
					 *		we can precalculate the numbers below:
					 *			disc_pu = pu / a ; disc_pd = pd / a ;
					 */
					s[j] = s[j] * d;		// (a)
					o[j] = o[j] * pu_div_a + o[j + 1] * pd_div_a;		// (b)

					const double underlyingPriceAdjustedByDividend = s[j] + discountedDividendAtTime[i];

					if (OS == AMERICAN) {
						double payoffWhenExerciseNow;
						if (CP == CALL) {
							 payoffWhenExerciseNow = max(underlyingPriceAdjustedByDividend - strikePrice, .0);
						} else {
							payoffWhenExerciseNow = max(strikePrice - underlyingPriceAdjustedByDividend, .0);
						}
						o[j] = max(o[j], payoffWhenExerciseNow);
					}
				}

			}

			double optionValue = o[0];
			return optionValue;

		}




}