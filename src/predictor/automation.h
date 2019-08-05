/**
 * Implementation of predicting multihead deterministic finite-state automation.
 *
 */

#ifndef AUTOMATION_H_INCLUDED
#define AUTOMATION_H_INCLUDED

#include "head.h"
#include "dtypes.h"
#include "tseries.h"

#include <forward_list>
#include <array>
#include <utility>
#include <algorithm>
#include <cassert>
#include <type_traits>
#include <memory>
#include <typeinfo>
#include <limits>

namespace itp { 
    Prec_double_t krichevsky_predictor(Symbol_t sym, size_t sym_freq,
                                       size_t total_freq, size_t alphabet);

    class Automation_word {
    public:
        using Ext_symbol_t = unsigned long;
        static constexpr Ext_symbol_t beginning_delimiter = std::numeric_limits<Symbol_t>::max() + 1;

        void set_time_series(const Plain_tseries<Symbol_t> *ts);
        
        ssize_t size() const;

        const Ext_symbol_t operator [] (ssize_t n) const;

    private:
        const Plain_tseries<Symbol_t> *time_series;
    };
    
    /**
     * Common interface for all kinds of prediction automation.
     *
     */
    class Prediction_automation {
    public:

        /**
         * Estimates probability of the word.
         *
         * @param w Plain_tseries<Symbol_t> to evaluate probability.
         *
         * @return Evaluated probability of the word.
         */
        virtual Prec_double_t eval_probability(const Plain_tseries<Symbol_t> &w) = 0;
        virtual ~Prediction_automation() = default;

        virtual void set_min_symbol(Symbol_t);
        virtual void set_max_symbol(Symbol_t);
    };

    using Prediction_automation_ptr = std::shared_ptr<Prediction_automation>;

    template <size_t N>
    class Multihead_automation;

    /**
     * N - count of heads.
     */
    template <size_t N = 1>
    class Multihead_automation : public Prediction_automation {
    public:
        static const size_t heads_count = N;
        enum class Is_prediction_confident {yes, no};

        Multihead_automation(Symbol_t alphabet_min_symbol, Symbol_t alphabet_max_symbol);

        Prec_double_t eval_probability(const Plain_tseries<Symbol_t> &w) override final;

        /**
         * Assuming that heads of an automation are (somehow) numbered, allows
         * to get a head with specified number (for visualization, etc.)
         *
         * @param head_num Number of the head.
         *
         * @return Constant reference to the head with specified number.
         */
        inline const Head& h(size_t head_num) const;

        Symbol_t get_min_symbol() const;
        Symbol_t get_max_symbol() const;
        Symbol_t get_alphabet_range() const;

        void set_min_symbol(Symbol_t new_min_symbol) override;
        void set_max_symbol(Symbol_t new_max_symbol) override;
        
    protected:

        /**
         * Sets name to the head with specefied number.
         *
         * @param head_no Number of head.
         * @param name Name of the head.
         */
        void set_head_name(size_t head_no, const std::string &name);
        
        /**
         * Updates evaluated probability of the word.
         *
         * @param guessed_symbol Automation "thinks" that that symbol will be next.
         * @param confidence In several cases an automation cannot make sensing prediction,
         * to indicate such situation that parameter should be set to "no", otherwise "yes".
         */
        void guess(Symbol_t guessed_symbol, Is_prediction_confident confidence);

        template <typename H>
        bool move(const H &head_to_move);

        /**
         * Moves specified heads by one position to the right.
         *
         * @param head_to_move Head which should be moved.
         *
         * @return True, if movement were sucessful (symbol to the right of the head
         * was existed), and false otherwise.
         */
        template <typename H, typename... Heads>
        bool move(const H &head_to_move, Heads ...another_heads);

        bool is_rightmost(const Head &) const;

        /**
         * Initializes heads before prediction of a new word.
         *
         */
        virtual void init();

        /**
         * Run main prediction procedure.
         *
         */
        virtual void run() = 0;

        Automation_word a;                       /**< Time series to forecast (name choosed to be close
                                                            to the paper. */

        /**
         * Functions for debugging.
         */
        virtual void on_move_head(const Head &head_to_move);
        virtual void on_guess(Symbol_t guessed_symbol);
    private:
        std::array<Head, N> heads;
        size_t num_of_rightmost_head;

        Symbol_t alphabet_min_symbol;
        Symbol_t alphabet_max_symbol;

        Prec_double_t evaluated_probability;
        size_t confident_estimations_series_len;

        std::vector<size_t> letters_freq;
        std::vector<size_t> confident_guess_freq;
    };
}

template <size_t N>
itp::Multihead_automation<N>::Multihead_automation(Symbol_t alphabet_min_symbol,
                                                   Symbol_t alphabet_max_symbol)
    : alphabet_min_symbol {alphabet_min_symbol}, alphabet_max_symbol {alphabet_max_symbol},
      letters_freq(alphabet_max_symbol+1), confident_guess_freq(alphabet_max_symbol+1) {
          for (size_t i = 0; i < heads_count; ++i) {
              heads[i].id(i);
          }

          num_of_rightmost_head = 0;
      }

template <size_t N>
itp::Prec_double_t itp::Multihead_automation<N>::eval_probability(const Plain_tseries<Symbol_t> &w) {
    a.set_time_series(&w);

    init();
    run();

    return evaluated_probability;
}

template <size_t N>
const itp::Head & itp::Multihead_automation<N>::h(size_t head_num) const {
    return heads[head_num];
}

template <size_t N>
void itp::Multihead_automation<N>::set_head_name(size_t head_no, const std::string &name) {
    assert(head_no < N);
    heads[head_no].name(name);
}

template <size_t N>
void itp::Multihead_automation<N>::guess(Symbol_t guessed_symbol,
                                         Is_prediction_confident confidence) {
    size_t total_freq = 0;
    Prec_double_t sym_probability;
    if (h(num_of_rightmost_head) < a.size() - 1) {
        on_guess(guessed_symbol);
        auto observed_symbol = a[h(num_of_rightmost_head) + 1];
        switch(confidence) {
        case Is_prediction_confident::yes:
            ++confident_estimations_series_len;
            total_freq = confident_estimations_series_len;
            confident_guess_freq[guessed_symbol] = confident_estimations_series_len;
            sym_probability = krichevsky_predictor(observed_symbol, confident_guess_freq[observed_symbol],
                                                        total_freq, get_alphabet_range());
            // std::cout << (int)guessed_symbol << ' ' << sym_probability << std::endl;
            evaluated_probability *= krichevsky_predictor(observed_symbol,
                                                          confident_guess_freq[observed_symbol],
                                                          total_freq,
                                                          get_alphabet_range());
            confident_guess_freq[guessed_symbol] = 0;
            break;
        case Is_prediction_confident::no:
            confident_estimations_series_len = 0;
            auto position_in_word = h(num_of_rightmost_head);
            total_freq = position_in_word + 1;
            // observed_symbol = std::distance(std::begin(letters_freq), std::max_element(std::begin(letters_freq), std::end(letters_freq)));
            sym_probability = krichevsky_predictor(observed_symbol, letters_freq[observed_symbol],
                                                   total_freq, get_alphabet_range());
            // std::cout << (int)guessed_symbol << ' ' << sym_probability << std::endl;
            evaluated_probability *= krichevsky_predictor(observed_symbol, letters_freq[observed_symbol],
                                                          total_freq, get_alphabet_range());
        }
    }
}

template <size_t N>
template <typename H>
bool itp::Multihead_automation<N>::move(const H &head_to_move) {
    static_assert(std::is_same<H, Head>::value, "Incorrect head type in move.");

    /// Subclasses are allowed to get only const references to heads, but in order
    /// to move a head const reference is not sutable. So every head holds it's
    /// own position in the array of heads in that class. That class is obligated
    /// to maintain such invariant.
    if (heads[head_to_move.id()] + 1 == a.size()) {
        return false;
    }

    heads[head_to_move.id()].move();
    on_move_head(heads[head_to_move.id()]);

    if (heads[num_of_rightmost_head] < heads[head_to_move.id()]) {
        num_of_rightmost_head = head_to_move.id();
    }

    if (head_to_move.id() == num_of_rightmost_head) {
        auto new_observed_symbol = a[h(num_of_rightmost_head)];
        ++letters_freq[new_observed_symbol];
    }

    // std::cout << head_to_move.name() << ' ' << (long)head_to_move << ' '
    //           << num_of_rightmost_head << std::endl;

    return true;
}

template <size_t N>
template <typename H, typename... Heads>
bool itp::Multihead_automation<N>::move(const H &head_to_move, Heads... another_heads) {
    static_assert(std::is_same<H, Head>::value, "Incorrect head type in move.");
    return move(head_to_move) && move(another_heads...);
}

template <size_t N>
bool itp::Multihead_automation<N>::is_rightmost(const Head &h) const {
    return h == heads[num_of_rightmost_head];
}

template <size_t N>
void itp::Multihead_automation<N>::init() {
    for (auto &head : heads) {
        head.move(-1);
    }

    evaluated_probability = 1;
    confident_estimations_series_len = 0;
    std::fill(begin(letters_freq), end(letters_freq), 0);
    std::fill(begin(confident_guess_freq), end(confident_guess_freq), 0);
}

template <size_t N>
void itp::Multihead_automation<N>::on_move_head(const Head &head_to_move) {
    // DO NOTHING
}

template <size_t N>
void itp::Multihead_automation<N>::on_guess(Symbol_t guessed_symbol) {
    // DO NOTHING
}

template <size_t N>
itp::Symbol_t itp::Multihead_automation<N>::get_min_symbol() const {
    return alphabet_min_symbol;
}

template <size_t N>
itp::Symbol_t itp::Multihead_automation<N>::get_max_symbol() const {
    return alphabet_max_symbol;
}

template <size_t N>
itp::Symbol_t itp::Multihead_automation<N>::get_alphabet_range() const {
    return alphabet_max_symbol - alphabet_min_symbol + 1;
}

template <size_t N>
void itp::Multihead_automation<N>::set_min_symbol(Symbol_t new_min_symbol) {
    alphabet_min_symbol = new_min_symbol;
}

template <size_t N>
void itp::Multihead_automation<N>::set_max_symbol(Symbol_t new_max_symbol) {
    alphabet_max_symbol = new_max_symbol;
}
 
#endif // AUTOMATION_H_INCLUDED
