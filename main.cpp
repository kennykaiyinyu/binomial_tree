#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include "binomial_tree.h"
#include "Dividend.h"

int main() {
    using namespace binomial_tree;
    using dividends::Dividend;

    // Parameters for testing
    CallPut cp = PUT;
    OptionStyle style = AMERICAN;
    double spot = 235.50;
    double rate = 0.0042;
    double expiry = 0.5;
    double vol = .25;
    double strike = 235.00;

    Dividend d1{0.25, 0.25};
    std::vector<Dividend*> divs;
    divs.push_back(&d1);

    // Steps to test for convergence
    std::vector<long> test_steps = {100, 500, 1000, 5000, 10000, 20000};

    std::cout << std::left << std::setw(10) << "Steps (N)"
              << std::setw(15) << "Price"
              << std::setw(15) << "Latency (ms)" << std::endl;
    std::cout << std::string(40, '-') << std::endl;

    for (long n : test_steps) {
        auto start = std::chrono::high_resolution_clock::now();

        double price = computeValue(cp, style, spot, rate, expiry, &divs, vol, strike, n);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms = end - start;

        std::cout << std::left << std::setw(10) << n
                  << std::setw(15) << std::fixed << std::setprecision(6) << price
                  << std::setw(15) << ms.count() << std::endl;
    }

    return 0;
}