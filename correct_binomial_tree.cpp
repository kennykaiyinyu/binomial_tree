#include <math.h>
#include <algorithm>    // std::max
#include <iostream>
#include <iomanip>      // std::setprecision
#include <vector>
#include <cassert>


using namespace std;

namespace dividends {
	class Dividend {
	public:
		Dividend(double _amount, double _timeToExDiv) : amount(_amount), timeToExDiv(_timeToExDiv) { }
		double getAmount() { return this->amount; }
		double getTimeToExDiv() {return this->timeToExDiv; }
	private:
		double amount;
		double timeToExDiv;
	};
}

using Dividend = dividends::Dividend;

namespace binomial_tree {

	enum CallPut {CALL, PUT};
	enum OptionStyle {EUROPEAN, AMERICAN};

	double computeValue(
		const CallPut& callPut, 
		const OptionStyle& optionStyle, 
		const double& initialUnderlyingPrice, 
		const double& continuousRate, 
		const double& timeToExpiry, 
		vector<Dividend*> *const dividends, 
		const double& volatility,
		double strikePrice,
		long n /* steps */) {

			if (callPut != CALL && callPut != PUT)  {
				throw "must be call or put!";
			}

			if (optionStyle != EUROPEAN && optionStyle != AMERICAN)  {
				throw "must be European or American!";
			}

			


			double deltaT = timeToExpiry / (.0 + n);
			double u = exp(volatility * sqrt(deltaT));
			double d = 1 / u;
			double p = 1 / u;
			double a = exp(continuousRate * deltaT) ; // growth rate at each step (i.e. 1 / discount factor)
			double pu = (a - d) / (u - d);
			double pd = 1 - pu;
			

			
			
			double* const s = new double[n + 1];	// The tree leaves of the underlying (with dividend excluded)
			double* const o = new double[n + 1];	// option values 
			
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


			double* const discountedDividendAtTime = new double[n + 1];	// the discountedDividendValueAtSteps[0, 1, 2, ..., n]
	
			for (long k = 0; k <= n; k++) {
				discountedDividendAtTime[k]  = .0;
			}

			

			if (dividends != 0) {

				for (std::vector<Dividend*>::iterator itr = dividends->begin() ; itr != dividends->end(); itr++) {
				
					Dividend* d = *itr;
					double timeToExDiv = d->getTimeToExDiv();
					if (timeToExDiv < timeToExpiry) {
						double dividendAmount = d->getAmount();
						long lastStepBeforeExDiv = (long) (timeToExDiv / deltaT);
						for (long k = 0; k <= lastStepBeforeExDiv; k++) {
							discountedDividendAtTime[k] += dividendAmount * exp(-continuousRate *( timeToExDiv - k * deltaT));
						}

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
			delete[] s;
			delete[] o;
			delete[] discountedDividendAtTime;

			return optionValue;

		}



	
}
int main(void) {
	using namespace binomial_tree ;

	CallPut callPut = PUT;
	OptionStyle os = AMERICAN;
	double initS = 52.0;
	double r = .10;
	double timeToExpiry = 2.5;
	Dividend d1(2.06, 3.5/12.0);
	//Dividend dividends[] = { d1 };
	vector<Dividend*> dividends;
	dividends.push_back(&d1);
	
	double vol = .40;
	double strike = 50.0;
	long n = (long) (timeToExpiry * 2040);

	assert(n == 5100);
	auto optionValue = std::floor(binomial_tree::computeValue(callPut, os, initS, r,timeToExpiry, &dividends, vol, strike, n) * 1000 + .5)/1000.0;
	cout << optionValue << endl;
	assert(std::abs(7.946 - optionValue) < 1e-6);

	cout << setprecision(4) << n <<"," << optionValue << endl;
	

}