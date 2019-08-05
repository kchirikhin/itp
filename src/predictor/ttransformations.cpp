#include "ttransformations.h"

namespace itp {
    std::vector<Double_t> Weights_generator::generate(size_t n) const {
        return std::vector<Double_t>(n, 1. / static_cast<Double_t>(n));
    }

    std::vector<Double_t> Countable_weights_generator::generate(size_t n) const {
        assert(0 < n);

        std::vector<Double_t> result(n);
        for (size_t i = 0; i < n - 1; ++i) {
            result[i] = 1. / (i + 1.) - 1. / (i + 2.);
        }
        result[n - 1] = 1. / n;

        return result;
    }
} // itp
