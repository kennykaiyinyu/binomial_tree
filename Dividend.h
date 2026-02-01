#pragma once

namespace dividends {
	class Dividend {
	public:
		Dividend(double amount, double timeToExDiv) : amount_(amount), timeToExDiv_(timeToExDiv) { }
		double getAmount() { return this->amount_; }
		double getTimeToExDiv() {return this->timeToExDiv_; }
	private:
		double amount_;
		double timeToExDiv_;
	};
};