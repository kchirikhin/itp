#include "ttransformations.h"

namespace itp {
    std::vector<Double> Weights_generator::generate(size_t n) const {
        return std::vector<Double>(n, 1. / static_cast<Double>(n));
    }

    std::vector<Double> Countable_weights_generator::generate(size_t n) const {
        assert(0 < n);

        std::vector<Double> result(n);
        for (size_t i = 0; i < n - 1; ++i) {
            result[i] = 1. / (i + 1.) - 1. / (i + 2.);
        }
        result[n - 1] = 1. / n;

        return result;
    }
} // itp
