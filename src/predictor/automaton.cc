#include "automation.h"

namespace itp {
    void Automation_word::set_time_series(const Plain_tseries<Symbol_t> *ts) {
        time_series = ts;
    }

    ssize_t Automation_word::size() const {
        assert(time_series != nullptr);
        return time_series->size();
    }

    const Automation_word::Ext_symbol_t Automation_word::operator [] (ssize_t n) const {
        assert(time_series != nullptr);

        if (0 <= n) {
            return (*time_series)[n];
        }

        assert(n == -1);
        return beginning_delimiter;
    }

    void Prediction_automation::set_min_symbol(Symbol_t) {
        // DO NOTHING
    }

    void Prediction_automation::set_max_symbol(Symbol_t) {
        // DO NOTHING
    }

    Prec_double_t krichevsky_predictor(Symbol_t sym, size_t sym_freq,
                                       size_t total_freq, size_t alphabet) {
        return (Prec_double_t(sym_freq) + 1./2) / (Prec_double_t(total_freq) +
                                                   alphabet/2.);
    }
} // itp
