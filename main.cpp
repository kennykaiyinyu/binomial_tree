#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <mutex>
#include "binomial_tree.h"

int main() {
	using namespace binomial_tree;
	vector<Dividend*> dividends;

	// European Stock A put
	{
		auto optionValue = computeValue<PUT,EUROPEAN>(
			52.0,
			0.1,
			0.5,
			{{2.06, 3.5/12.0}},
			0.40,
			50,
			5100);
		cout << optionValue << endl;
	}
	// American Stock A put
	{
		auto optionValue = computeValue<PUT,AMERICAN>(
			52.0,
			0.1,
			0.5,
			{{2.06, 3.5/12.0}},
			0.40,
			50,
			5100);
		cout << optionValue << endl;
	}

	// convergency test for AAPL Call

	vector<int> step_sizes = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 30, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1200, 1400, 1600, 1800, 2000};
	{

		std::cout << std::left << std::setw(10) << "Steps (N)"
					  << std::setw(15) << "Price"
					  << std::setw(15) << "Latency (ms)" << std::endl;
		std::cout << std::string(40, '-') << std::endl;
		for (auto & step_size :step_sizes) {
			auto start = std::chrono::high_resolution_clock::now();
			auto optionValue = computeValue<CALL,EUROPEAN>(
				259.48,
				0.0362,
				0.3726,
				{{0.25, 0.0301}, {0.25, 0.2795}},
				0.265,
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
			auto optionValue = computeValue<CALL,AMERICAN>(
				259.48,
				0.0362,
				0.3726,
				{{0.25, 0.0301}, {0.25, 0.2795}},
				0.265,
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