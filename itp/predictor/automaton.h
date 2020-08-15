/**
 * Implementation of the predicting multihead deterministic finite-state automaton.
 *
 */

#ifndef ITP_AUTOMATON_H_INCLUDED_
#define ITP_AUTOMATON_H_INCLUDED_

#include "head.h"
#include "dtypes.h"
#include "tseries.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <forward_list>
#include <limits>
#include <memory>
#include <utility>

namespace itp {

/**
 * Evaluates the probability of the given symbol of a word.
 * For more details, see
 * Krichevsky R. (1968) A relation between the plausibility of information about a source and encoding redundancy.
 *   Problems Inform. Transmission. Vol. 4 pp. 48-57.
 *
 * @param sym the symbol which probability is needed to evaluate;
 * @param sym_freq how many times sym was encountered in the word;
 * @param total_freq is the position of sym in the word;
 * @param alphabet_size is the number of all possible symbols which may be found in the word.
 */
HighPrecDouble KrichevskyPredictor(Symbol sym, size_t sym_freq, size_t total_freq, size_t alphabet_size);

class AutomatonWord {
 public:
  using Ext_symbol_t = unsigned long;
  static constexpr Ext_symbol_t kBeginningDelimiter = std::numeric_limits<Symbol>::max() + 1;

  AutomatonWord() = default;
  
  // Intentionally made implicit.
  AutomatonWord(const PlainTimeSeries<Symbol> &time_series);
  
  ssize_t size() const;

  // -1 is allowed! All other negative indexes are prohibited.
  // Made in such a way to be close to the paper.
  const Ext_symbol_t& operator [] (ssize_t n) const;

 private:
  PlainTimeSeries<Ext_symbol_t> time_series_;
};
    
/**
 * Common interface for all kinds of prediction automatons.
 *
 */
class PredictionAutomation {
 public:

  /**
   * Estimates probability of the word.
   *
   * @param w PlainTimeSeries<Symbol> to evaluate probability.
   *
   * @return Evaluated probability of the word.
   */
  virtual HighPrecDouble EvalProbability(const PlainTimeSeries<Symbol> &w) = 0;
  virtual ~PredictionAutomation() = default;

  /**
   * This functions are required because the size of an alphabet does kNot important for data
   * compression algorithms, but is required by the automaton. To kNot change the interfaces of data compression
   * algorithms, this data was extracted to the following methods.
   */
  virtual void SetMinSymbol(Symbol);
  virtual void SetMaxSymbol(Symbol);
};

using PredictionAutomationPtr = std::shared_ptr<PredictionAutomation>;

template <size_t N>
class MultiheadAutomaton;

/**
 * N - count of heads.
 */
template <size_t N = 1>
class MultiheadAutomaton : public PredictionAutomation {
 public:
	static_assert(sizeof(Symbol) <= sizeof(size_t), "");

  static const size_t heads_count = N;
  enum class IsPredictionConfident { kYes, kNo };

  MultiheadAutomaton(Symbol alphabet_min_symbol_, Symbol alphabet_max_symbol_);

  HighPrecDouble EvalProbability(const PlainTimeSeries<Symbol> &w) override final;

  /**
   * Assuming that heads of an automation are (somehow) numbered, allows
   * to get a head with specified number (for visualization, etc.)
   *
   * @param head_num Number of the head.
   *
   * @return Constant reference to the head with specified number.
   */
  inline const Head& h(size_t head_num) const;

  Symbol GetMinSymbol() const;
  Symbol GetMaxSymbol() const;
  size_t GetAlphabetRange() const;

  void SetMinSymbol(Symbol new_min_symbol) override;
  void SetMaxSymbol(Symbol new_max_symbol) override;
        
 protected:

  /**
   * Sets name to the head with specefied number.
   *
   * @param head_kNo Number of head.
   * @param name Name of the head.
   */
  void SetHeadName(size_t head_kNo, const std::string &name);
        
  /**
   * Updates evaluated probability of the word.
   *
   * @param guessed_symbol Automation "thinks" that that symbol will be next.
   * @param confidence In several cases an automation cankNot make sensing prediction,
   * to indicate such situation that parameter should be set to "kNo", otherwise "kYes".
   */
  void Guess(Symbol guessed_symbol, IsPredictionConfident confidence);

  template <typename H>
  bool Move(const H &head_to_move);

  /**
   * Moves specified heads by one position to the right.
   *
   * @param head_to_move Head which should be moved.
   *
   * @return True, if movement were sucessful (symbol to the right of the head
   * was existed), and false otherwise.
   */
  template <typename H, typename... Heads>
  bool Move(const H &head_to_move, Heads ...other_heads_to_move);

  bool IsRightmost(const Head &) const;

  /**
   * Initializes heads before prediction of a new word.
   *
   */
  virtual void Init();

  /**
   * Run main prediction procedure.
   *
   */
  virtual void Run() = 0;

  AutomatonWord a;                       /**< Time series to forecast (name choosed to be close
                                              to the paper. */

  /**
   * Functions for debugging.
   */
  virtual void OnMoveHead(const Head &head_to_move);
  virtual void OnGuess(Symbol guessed_symbol);
  
 private:
  std::array<Head, N> heads_;
  size_t num_of_rightmost_head_;

  Symbol alphabet_min_symbol_;
  Symbol alphabet_max_symbol_;

  HighPrecDouble evaluated_probability_;
  size_t confident_estimations_series_len_;

  std::vector<size_t> letters_freq_;
  std::vector<size_t> confident_guess_freq_;
};
} // itp

template <size_t N>
itp::MultiheadAutomaton<N>::MultiheadAutomaton(Symbol alphabet_min_symbol_,
                                               Symbol alphabet_max_symbol_)
    : alphabet_min_symbol_ {alphabet_min_symbol_}, alphabet_max_symbol_ {alphabet_max_symbol_},
      letters_freq_(alphabet_max_symbol_+1), confident_guess_freq_(alphabet_max_symbol_+1) {
        for (size_t i = 0; i < heads_count; ++i) {
          heads_[i].id(i);
        }

        num_of_rightmost_head_ = 0;
      }

template <size_t N>
itp::HighPrecDouble itp::MultiheadAutomaton<N>::EvalProbability(const PlainTimeSeries<Symbol> &w) {
  a = w;

  Init();
  Run();

  return evaluated_probability_;
}

template <size_t N>
const itp::Head & itp::MultiheadAutomaton<N>::h(size_t head_num) const {
  return heads_[head_num];
}

template <size_t N>
void itp::MultiheadAutomaton<N>::SetHeadName(size_t head_kNo, const std::string &name) {
  assert(head_kNo < N);
  heads_[head_kNo].name(name);
}

template <size_t N>
void itp::MultiheadAutomaton<N>::Guess(Symbol guessed_symbol,
                                         IsPredictionConfident confidence) {
  size_t total_freq = 0;
  HighPrecDouble sym_probability;
  if (h(num_of_rightmost_head_) < a.size() - 1) {
    OnGuess(guessed_symbol);
    auto observed_symbol = a[h(num_of_rightmost_head_) + 1];
    switch(confidence) {
      case IsPredictionConfident::kYes:
        ++confident_estimations_series_len_;
        total_freq = confident_estimations_series_len_;
        confident_guess_freq_[guessed_symbol] = confident_estimations_series_len_;
        sym_probability = KrichevskyPredictor(observed_symbol, confident_guess_freq_[observed_symbol],
                                              total_freq, GetAlphabetRange());
        evaluated_probability_ *= sym_probability;
        confident_guess_freq_[guessed_symbol] = 0;
        break;
      case IsPredictionConfident::kNo:
        confident_estimations_series_len_ = 0;
        auto position_in_word = h(num_of_rightmost_head_);
        total_freq = position_in_word + 1;
        sym_probability = KrichevskyPredictor(observed_symbol, letters_freq_[observed_symbol],
                                              total_freq, GetAlphabetRange());
        evaluated_probability_ *= sym_probability;
    }
  }
}

template <size_t N>
template <typename H>
bool itp::MultiheadAutomaton<N>::Move(const H &head_to_move) {
  static_assert(std::is_same<H, Head>::value, "Incorrect head type in move.");

  /// Subclasses are allowed to get only const references to heads_, but in order
  /// to move a head const reference is kNot sutable. So every head holds it's
  /// own position in the array of heads_ in that class. That class is obligated
  /// to maintain such invariant.
  if (heads_[head_to_move.id()] + 1 == a.size()) {
    return false;
  }

  heads_[head_to_move.id()].move();
  OnMoveHead(heads_[head_to_move.id()]);

  if (heads_[num_of_rightmost_head_] < heads_[head_to_move.id()]) {
    num_of_rightmost_head_ = head_to_move.id();
  }

  if (head_to_move.id() == num_of_rightmost_head_) {
    auto new_observed_symbol = a[h(num_of_rightmost_head_)];
    ++letters_freq_[new_observed_symbol];
  }

  return true;
}

template <size_t N>
template <typename H, typename... Heads>
bool itp::MultiheadAutomaton<N>::Move(const H &head_to_move, Heads... other_heads_to_move) {
  static_assert(std::is_same<H, Head>::value, "Incorrect head type in move.");
  return Move(head_to_move) && Move(other_heads_to_move...);
}

template <size_t N>
bool itp::MultiheadAutomaton<N>::IsRightmost(const Head &h) const {
  return h == heads_[num_of_rightmost_head_];
}

template <size_t N>
void itp::MultiheadAutomaton<N>::Init() {
  for (auto &head : heads_) {
    head.move(-1);
  }

  evaluated_probability_ = 1;
  confident_estimations_series_len_ = 0;
  std::fill(begin(letters_freq_), end(letters_freq_), 0);
  std::fill(begin(confident_guess_freq_), end(confident_guess_freq_), 0);
}

template <size_t N>
void itp::MultiheadAutomaton<N>::OnMoveHead(const Head &) {
  // DO NOTHING
}

template <size_t N>
void itp::MultiheadAutomaton<N>::OnGuess(Symbol) {
  // DO NOTHING
}

template <size_t N>
itp::Symbol itp::MultiheadAutomaton<N>::GetMinSymbol() const {
  return alphabet_min_symbol_;
}

template <size_t N>
itp::Symbol itp::MultiheadAutomaton<N>::GetMaxSymbol() const {
  return alphabet_max_symbol_;
}

template <size_t N>
size_t itp::MultiheadAutomaton<N>::GetAlphabetRange() const {
  return static_cast<size_t>(alphabet_max_symbol_) - static_cast<size_t>(alphabet_min_symbol_) + 1;
}

template <size_t N>
void itp::MultiheadAutomaton<N>::SetMinSymbol(Symbol new_min_symbol) {
  alphabet_min_symbol_ = new_min_symbol;
}

template <size_t N>
void itp::MultiheadAutomaton<N>::SetMaxSymbol(Symbol new_max_symbol) {
  alphabet_max_symbol_ = new_max_symbol;
}
 
#endif // ITP_AUTOMATON_H_INCLUDED_
