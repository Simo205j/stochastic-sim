#include "covid19_helper.hpp"

#include <iostream>

namespace
{

    void print_result(const covid19_example::PeakHospitalizedResult &result)
    {
        std::cout << result.population_name
                  << " (N = "
                  << result.population_size
                  << "): peak hospitalized = "
                  << result.peak_hospitalized
                  << '\n';
    }

} // namespace

int main()
{
    const auto nnj = covid19_example::estimate_peak_hospitalized("NNJ", 589'755, 42U);
    const auto ndk = covid19_example::estimate_peak_hospitalized("NDK", 5'822'763, 42U);

    print_result(nnj);
    print_result(ndk);
}