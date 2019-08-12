/**
 * Functions for sampling and desampling real time series.
 */

#ifndef SAMPLER_H
#define SAMPLER_H

#include "dtypes.h"

#include <vector>
#include <utility>
#include <cassert>
#include <memory>

namespace itp {
    class Sampler {
    public:
        Sampler() = default;
        explicit Sampler(double);

        void set_indent(double);
        double get_indent() const;

        virtual Sampler* clone();

        Preprocessed_tseries<Double, Symbol> sample(const Preprocessed_tseries<Double, Double> &, size_t);
        Preprocessed_tseries<Symbol, Symbol> normalize(const Preprocessed_tseries<Symbol, Symbol> &);

        template <typename T>
        Double desample(Symbol, const Preproc_info<T> &);

    private:
        double indent = 0.1;
    };

    using Sampler_ptr = std::shared_ptr<Sampler>;
} // of itp

template <typename T>
itp::Double itp::Sampler::desample(Symbol s, const Preproc_info<T> &info) {
    if (!info.is_sampled()) {
        return s;
    }
        
    assert(!info.get_desample_table().empty());
    assert(s < info.get_desample_table().size());

    return (info.get_desample_table())[s];
}

#endif // SAMPLER_H
