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

	double computeValue(
		const CallPut& callPut,
		const OptionStyle& optionStyle,
		const double& initialUnderlyingPrice,
		const double& continuousRate,
		const double& timeToExpiry,
		const vector<Dividend> & dividends,
		const double& volatility,
		double strikePrice,
		long n /* steps */) {

			if (callPut != CALL && callPut != PUT)  {
				throw runtime_error("must be call or put!");
			}

			if (optionStyle != EUROPEAN && optionStyle != AMERICAN)  {
				throw runtime_error("must be European or American!");
			}

			double deltaT = timeToExpiry / (.0 + n);
			double u = exp(volatility * sqrt(deltaT));
			double d = 1 / u;
			double p = 1 / u;
			double a = exp(continuousRate * deltaT) ; // growth rate at each step (i.e. 1 / discount factor)
			double pu = (a - d) / (u - d);
			double pd = 1 - pu;

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
			for (auto & d : dividends) {
				double timeToExDiv = d.timeToExDiv;
				if (timeToExDiv < timeToExpiry) {
					double dividendAmount = d.amount;
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
				s[i] = initialUnderlyingPriceExcludedDividend * pow(u, n - 2.0 * i);

				const double underlyingPriceAdjustedByDividend = s[i] + discountedDividendAtTime[n];

				if (callPut == CALL) {
					o[i] = max(underlyingPriceAdjustedByDividend - strikePrice, .0);
				} else  {	// callPut == PUT
					o[i] = max(strikePrice - underlyingPriceAdjustedByDividend, .0);
				}
			}

			// backward process

			for (i = n - 1; i >= 0; i--)  {
				for (j = 0; j <= i; j++) {
					s[j] = (s[j] * pu + s[j + 1] * pd) / a;
					o[j] = (o[j] * pu + o[j + 1] * pd) / a;

					const double underlyingPriceAdjustedByDividend = s[j] + discountedDividendAtTime[i];

					if (optionStyle == AMERICAN) {
						double payoffWhenExerciseNow;
						if (callPut == CALL) {
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
int main(void) {
	using namespace binomial_tree;
	vector<Dividend*> dividends;

	{
		auto optionValue = computeValue(PUT,
			EUROPEAN,
			52.0,
			0.1,
			2.5,
			{{2.06, 3.5/12.0}},
			0.40,
			50,
			5100);
		cout << optionValue << endl;
		assert(std::abs(6.324 - std::floor(optionValue*1000.0 + 0.5)/1000.0) < 1e-6);
	}
	{
		auto optionValue = computeValue(PUT,
			AMERICAN,
			52.0,
			0.1,
			2.5,
			{{2.06, 3.5/12.0}},
			0.40,
			50,
			5100);
		cout << optionValue << endl;
		assert(std::abs(7.946 - std::floor(optionValue*1000.0 + 0.5)/1000.0) < 1e-6);
	}
	vector<int> step_sizes = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 30, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1200, 1400, 1600, 1800, 2000};
	{

		std::cout << std::left << std::setw(10) << "Steps (N)"
					  << std::setw(15) << "Price"
					  << std::setw(15) << "Latency (ms)" << std::endl;
		std::cout << std::string(40, '-') << std::endl;
		for (auto & step_size :step_sizes) {
			auto start = std::chrono::high_resolution_clock::now();
			auto optionValue = computeValue(CALL,
				EUROPEAN,
				259.48,
				0.0362,
				0.3726,
				{{0.25, 0.0301}, {0.25, 0.2795}},
				0.215,
				280,
				step_size);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> ms = end - start;
			std::cout << std::left << std::setw(10) << step_size
			  << std::setw(15) << std::fixed << std::setprecision(6) << optionValue
			  << std::setw(15) << ms.count() << std::endl;
		}
	}
	{

		std::cout << std::left << std::setw(10) << "Steps (N)"
					  << std::setw(15) << "Price"
					  << std::setw(15) << "Latency (ms)" << std::endl;
		std::cout << std::string(40, '-') << std::endl;
		for (auto & step_size :step_sizes) {
			auto start = std::chrono::high_resolution_clock::now();
			auto optionValue = computeValue(CALL,
				AMERICAN,
				259.48,
				0.0362,
				0.3726,
				{{0.25, 0.0301}, {0.25, 0.2795}},
				0.215,
				280,
				step_size);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> ms = end - start;
			std::cout << std::left << std::setw(10) << step_size
			  << std::setw(15) << std::fixed << std::setprecision(6) << optionValue
			  << std::setw(15) << ms.count() << std::endl;
		}
	}



	

}