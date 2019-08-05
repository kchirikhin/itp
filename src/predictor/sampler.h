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

        Preprocessed_tseries<Double_t, Symbol_t> sample(const Preprocessed_tseries<Double_t, Double_t> &, size_t);
        Preprocessed_tseries<Symbol_t, Symbol_t> normalize(const Preprocessed_tseries<Symbol_t, Symbol_t> &);

        template <typename T>
        Double_t desample(Symbol_t, const Preproc_info<T> &);

    private:
        double indent = 0.1;
    };

    using Sampler_ptr = std::shared_ptr<Sampler>;
} // of itp

template <typename T>
itp::Double_t itp::Sampler::desample(Symbol_t s, const Preproc_info<T> &info) {
    if (!info.is_sampled()) {
        return s;
    }
        
    assert(!info.get_desample_table().empty());
    assert(s < info.get_desample_table().size());

    return (info.get_desample_table())[s];
}

#endif // SAMPLER_H
