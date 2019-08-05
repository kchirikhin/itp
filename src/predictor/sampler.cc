#include "sampler.h"

#include <algorithm>
#include <cmath>
#include <functional>

namespace itp {
    Sampler::Sampler(double indent)
        : indent {indent} {}

    void Sampler::set_indent(double new_indent) {
        indent = new_indent;
    }

    double Sampler::get_indent() const {
        return indent;
    }

    Sampler* Sampler::clone() {
        return new Sampler(indent);
    }

    Preprocessed_tseries<Double_t, Symbol_t> Sampler::sample(const Preprocessed_tseries<Double_t, Double_t> &points, size_t N) {
        Double_t min {*std::min_element(points.cbegin(), points.cend())};
        Double_t max {*std::max_element(points.cbegin(), points.cend())};
        auto width = fabs(max - min);
        min -= (width * indent);
        max += (width * indent);
        auto delta = (max - min) / N;

        Plain_tseries<Symbol_t> sampled_ts(points.size());
        for (size_t i = 0; i < points.size(); ++i) {
            sampled_ts[i] = static_cast<Plain_tseries<Symbol_t>::value_type>(floor((points[i]- min) / delta));

            // Это событие обязательно произойдёт для максимального члена
            // временного ряда.
            if (sampled_ts[i] > N - 1) {
                sampled_ts[i] = static_cast<Symbol_t>(N - 1);
            }
        }

        Preprocessed_tseries<Double_t, Symbol_t> result(sampled_ts);
        result.copy_preprocessing_info_from(points);

        std::vector<Double_t> desample_table(N);
        for (size_t i = 0; i < N; ++i) {
            desample_table[i] = min + i * delta + delta / 2;
        }

        result.set_desample_table(desample_table);
        result.set_desample_indent(indent);
        result.set_sampling_alphabet(N);

        return result;
    }

    Preprocessed_tseries<Symbol_t, Symbol_t>
    Sampler::normalize(const Preprocessed_tseries<Symbol_t, Symbol_t> &points) {
        using namespace std::placeholders;

        auto min_point = *std::min_element(points.cbegin(), points.cend());
        auto max_point = *std::max_element(points.cbegin(), points.cend());
        Plain_tseries<Symbol_t> normalized_points(points.size());
        std::transform(points.cbegin(), points.cend(), begin(normalized_points),
                       std::bind(std::minus<Symbol_t>(), _1,
                                 min_point));
        assert(*std::min_element(begin(normalized_points),
                                 end(normalized_points)) == 0);
        std::vector<Symbol_t> desample_table(max_point - min_point + 1);
        for (size_t i = 0; i < desample_table.size(); ++i) {
            desample_table[i] = i + min_point;
        }

        Preprocessed_tseries<Symbol_t, Symbol_t> result(normalized_points);
        result.copy_preprocessing_info_from(points);
        result.set_desample_table(desample_table);
        result.set_desample_indent(indent);
        result.set_sampling_alphabet(max_point - min_point + 1);

        return result;
    }
} // itp
