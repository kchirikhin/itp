#include "../src/Sdfa.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <numeric>
#include <string_view>
#include <vector>

namespace itp
{

struct HeadPosition
{
  HeadPosition(std::string name, long position)
      : name{std::move(name)}, position{position}
  {
    // DO NOTHING
  }
        
  std::string name;
  long position;
};

bool operator == (const HeadPosition &lhs, const HeadPosition &rhs)
{
  return (lhs.name == rhs.name) && (lhs.position == rhs.position);
}

std::ostream& operator << (std::ostream &ost, const HeadPosition &head)
{
  return ost << head.name << ' ' << head.position;
}
    
class AutomatonForTesting : public itp::Sensing_DFA
{
public:
  AutomatonForTesting(const Symbol alphabet_min_symbol, const Symbol alphabet_max_symbol);
        
  std::vector<HeadPosition> get_head_history() const;
  std::vector<Symbol> get_guess_history() const;
        
 protected:
  void OnMoveHead(const Head &head_to_move) override;
  void OnGuess(const Symbol guessed_symbol) override;
        
 private:
  std::vector<HeadPosition> head_history_;
  std::vector<Symbol> guess_history_;
};
} // of itp

itp::AutomatonForTesting::AutomatonForTesting(const Symbol alphabet_min_symbol,
                                              const Symbol alphabet_max_symbol)
    : Sensing_DFA{alphabet_min_symbol, alphabet_max_symbol}
{
  // DO NOTHING
}

std::vector<itp::HeadPosition> itp::AutomatonForTesting::get_head_history() const
{
  return head_history_;
}

std::vector<itp::Symbol> itp::AutomatonForTesting::get_guess_history() const
{
  return guess_history_;
}

void itp::AutomatonForTesting::OnMoveHead(const Head &head_to_move)
{
  head_history_.emplace_back(head_to_move.name(), static_cast<long>(head_to_move));
}

void itp::AutomatonForTesting::OnGuess(const Symbol guessed_symbol)
{
  guess_history_.push_back(guessed_symbol);
}

class WordPredictionTest
{
public:
  explicit WordPredictionTest(const itp::PlainTimeSeries<itp::Symbol> &test_word,
                              const itp::Symbol min_alphabet_sym = 0, const itp::Symbol max_alphabet_sym = 1)
      : automaton_{min_alphabet_sym, max_alphabet_sym}
  {
    evaluated_probability_ = automaton_.EvalProbability(test_word);
  }

  template <typename... Probability>
  void AssertProbabilityIsProductOf(Probability... probabilities) const
  {
    itp::HighPrecDouble set_of_probabilities[] = { static_cast<itp::HighPrecDouble>(probabilities)... };
    itp::HighPrecDouble expected_probability =
        std::accumulate(std::begin(set_of_probabilities), std::end(set_of_probabilities), 1.,
                        [](auto &num1, auto &num2) { return num1 * num2; });
    EXPECT_DOUBLE_EQ(static_cast<itp::Double>(evaluated_probability_),
                     static_cast<itp::Double>(expected_probability));
  }

  void AssertHistoryOfHeadsMovementsIs(const std::vector<itp::HeadPosition> &expected_history_of_movements)
  {
    auto actual_history_of_movements = automaton_.get_head_history();
    EXPECT_THAT(actual_history_of_movements, testing::ContainerEq(expected_history_of_movements));
  }

  template <typename... Guess>
  void AssertHistoryOfGussesIs(Guess... expected_guesses)
  {
    itp::Symbol expected_history_of_guesses[] = { static_cast<itp::Symbol>(expected_guesses)... };
    auto actual_history_of_guesses = automaton_.get_guess_history();
    EXPECT_THAT(actual_history_of_guesses, testing::ElementsAreArray(expected_history_of_guesses));
  }

private:
  itp::AutomatonForTesting automaton_;
  itp::HighPrecDouble evaluated_probability_;
};

TEST(SdfaTest, PredictWordOfLength2)
{
  itp::PlainTimeSeries<itp::Symbol> testing_ts{0, 1};
  WordPredictionTest test(testing_ts);

  test.AssertProbabilityIsProductOf(0.5, 0.25);
  test.AssertHistoryOfHeadsMovementsIs({{"r", 0}, {"h1", 0}, {"inner", 0}, {"h1", 1}, {"t", 0}});
  test.AssertHistoryOfGussesIs(0, 0);
}

TEST(SdfaTest, PredictWordOfLength5)
{
  itp::PlainTimeSeries<itp::Symbol> testing_ts{0, 1, 0, 0, 1};
  WordPredictionTest test(testing_ts);

  test.AssertProbabilityIsProductOf(0.5, 0.25, 0.5, 0.625, 0.3);
  test.AssertHistoryOfHeadsMovementsIs({
      {"r", 0}, {"h1", 0}, {"inner", 0}, {"h1", 1}, {"t", 0}, {"h1", 2}, {"l", 0},
      {"r", 1}, {"outer", 0}, {"t", 1}, {"h1", 3}, {"h2", 0}, {"h2", 1}, {"h2", 2},
      {"h2", 3}, {"outer", 1}, {"t", 2}, {"t", 3}, {"h2", 4}, {"inner", 1}
    });
  test.AssertHistoryOfGussesIs(0, 0, 0, 0, 0);
}

TEST(SdfaTest, PredictWordOfLength10)
{
  itp::PlainTimeSeries<itp::Symbol> testing_ts{0, 1, 0, 0, 1, 0, 0, 0, 1, 0};
  WordPredictionTest test(testing_ts);

  test.AssertProbabilityIsProductOf(0.5, 0.25, 0.5, 0.625, 0.3, 3.5/6, 4.5/7, 5.5/8, 2.5/9, 0.65);
  test.AssertHistoryOfHeadsMovementsIs({
      {"r", 0}, {"h1", 0}, {"inner", 0}, {"h1", 1}, {"t", 0}, {"h1", 2}, {"l", 0},
      {"r", 1}, {"outer", 0}, {"t", 1}, {"h1", 3}, {"h2", 0}, {"h2", 1}, {"h2", 2},
      {"h2", 3}, {"outer", 1}, {"t", 2}, {"t", 3}, {"h2", 4}, {"inner", 1}, {"h2", 5},
      {"t", 4}, {"h2", 6}, {"h2", 7}, {"t", 5}, {"h2", 8}, {"h2", 9}, {"t", 6}
    });
  test.AssertHistoryOfGussesIs(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

TEST(SdfaTest, PredictWordOfLength20)
{
  itp::PlainTimeSeries<itp::Symbol> testing_ts {0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
  WordPredictionTest test(testing_ts);

  test.AssertProbabilityIsProductOf(0.5, 0.25, 0.5, 0.625, 0.3, 3.5/6, 4.5/7, 5.5/8, 2.5/9, 0.65,
                                    7.5/11, 8.5/12, 0.25, 0.25, 10.5/15, 11.5/16, 12.5/17, 13.5/18,
                                    14.5/19, 0.25, 15.5/21, 16.5/22, 17.5/23, 18.5/24, 0.75, 2.5/3,
                                    0.5/4, 21.5/28, 22.5/29, 23.5/30, 24.5/31, 25.5/32, 0.75, 2.5/3,
                                    0.5/4, 28.5/36, 29.5/37, 30.5/38, 31.5/39, 32.5/40, 0.75, 2.5/3, 
                                    3.5/4, 0.5/5, 36.5/45, 37.5/46, 38.5/47, 39.5/48, 40.5/49, 0.75, 2.5/3);
  test.AssertHistoryOfHeadsMovementsIs({
      {"r", 0}, {"h1", 0}, {"inner", 0}, {"h1", 1}, {"t", 0}, {"h1", 2}, {"l", 0},
      {"r", 1}, {"outer", 0}, {"t", 1}, {"h1", 3}, {"h2", 0}, {"h2", 1}, {"h2", 2},
      {"h2", 3}, {"outer", 1}, {"t", 2}, {"t", 3}, {"h2", 4}, {"inner", 1}, {"h2", 5},
      {"t", 4}, {"h2", 6}, {"h2", 7}, {"t", 5}, {"h2", 8}, {"h2", 9}, {"t", 6},
      {"h2", 10}, {"l", 1}, {"r", 2}, {"outer", 2}, {"t", 7}, {"h2", 11}, {"t", 8},
      {"h2", 12}, {"l", 2}, {"r", 3}, {"h3", 0}, {"h3", 1}, {"h3", 2}, {"h3", 3}, {"h3", 4},
      {"h3", 5}, {"h3", 6}, {"h3", 7}, {"h3", 8}, {"h3", 9}, {"h3", 10}, {"h3", 11}, {"h3", 12},

      // AdvanceMany(3)
      {"outer", 3}, {"t", 9}, {"t", 10}, {"t", 11}, {"t", 12}, {"h3", 13}, {"inner", 2},
      {"inner", 3}, {"h3", 14}, {"t", 13}, {"h3", 15}, {"h3", 16}, {"t", 14}, {"h3", 17},
      {"l", 3}, {"r", 4}, {"outer", 4}, {"t", 15}, {"h3", 18}, {"t", 16}, {"h3", 19}, {"l", 4},
      {"r", 5}, {"h4", 0}, {"h4", 1}, {"h4", 2}, {"h4", 3}, {"h4", 4}, {"h4", 5}, {"h4", 6},
      {"h4", 7}, {"h4", 8}, {"h4", 9}, {"h4", 10}, {"h4", 11}, {"h4", 12}, {"h4", 13},
      {"h4", 14}, {"h4", 15}, {"h4", 16}, {"h4", 17}, {"h4", 18}, {"h4", 19},

      // AdvanceMany(4)
      {"outer", 5}, {"t", 17}, {"t", 18}, {"t", 19}, {"h4", 20}, {"inner", 4}, {"inner", 5},
      {"h4", 21}, {"t", 20}, {"h4", 22}, {"l", 5}, {"r", 6}, {"outer", 6}, {"t", 21}, {"h4", 23},
      {"t", 22}, {"h4", 24}, {"t", 23}, {"h4", 25}, {"t", 24}, {"h4", 26}, {"l", 6}, {"r", 7},
      {"h3a", 0}, {"h3a", 1}, {"h3a", 2}, {"h3a", 3}, {"h3a", 4}, {"h3a", 5}, {"h3a", 6}, {"h3a", 7},
      {"h3a", 8}, {"h3a", 9}, {"h3a", 10}, {"h3a", 11}, {"h3a", 12}, {"h3a", 13}, {"h3a", 14},
      {"h3a", 15}, {"h3a", 16}, {"h3a", 17}, {"h3a", 18}, {"h3a", 19}, {"r", 8},

      // correction(1)
      {"h1", 4}, {"h1", 5}, {"h1", 6}, {"h1", 7}, {"h1", 8}, {"h1", 9}, {"h1", 10}, {"h1", 11},
      {"h1", 12}, {"h1", 13}, {"h1", 14}, {"h1", 15}, {"h1", 16}, {"h1", 17}, {"h1", 18}, {"h1", 19},
      {"h1", 20}, {"h1", 21}, {"h1", 22}, {"h1", 23}, {"h1", 24}, {"h1", 25}, {"h1", 26},

      // AdvanceOne(1)
      {"t", 25}, {"t", 26}, {"h1", 27}, {"inner", 6}, {"inner", 7}, {"inner", 8}, {"h1", 28}, {"t", 27},
      {"h1", 29}, {"l", 7}, {"r", 9}, {"outer", 7}, {"t", 28}, {"h1", 30}, {"l", 8}, {"r", 10}, {"outer", 8},
      {"t", 29}, {"h1", 31}, {"t", 30}, {"h1", 32}, {"t", 31}, {"h1", 33}, {"t", 32}, {"h1", 34}, {"h2", 13},
      {"h2", 14}, {"h2", 15}, {"h2", 16}, {"h2", 17}, {"h2", 18}, {"h2", 19}, {"h2", 20}, {"h2", 21},
      {"h2", 22}, {"h2", 23}, {"h2", 24}, {"h2", 25}, {"h2", 26}, {"h2", 27}, {"h2", 28}, {"h2", 29},
      {"h2", 30}, {"h2", 31}, {"h2", 32}, {"h2", 33}, {"h2", 34},

      // AdvanceMany(2)
      {"outer", 9}, {"outer", 10}, {"t", 33}, {"t", 34}, {"h2", 35}, {"inner", 9}, {"inner", 10}, {"h2", 36},
      {"t", 35}, {"h2", 37}, {"l", 9}, {"r", 11}, {"outer", 11}, {"t", 36}, {"h2", 38}, {"l", 10}, {"r", 12},
      {"outer", 12}, {"t", 37}, {"h2", 39}, {"t", 38}, {"h2", 40}, {"t", 39}, {"h2", 41}, {"t", 40},
      {"h2", 42}, {"t", 41}, {"h2", 43}, {"l", 11}, {"r", 13},

      // AdvanceOne(2)
      {"t", 42}, {"t", 43}, {"h2", 44}, {"inner", 11}, {"inner", 12}, {"inner", 13}, {"h2", 45}, {"t", 44},
      {"h2", 46}, {"l", 12}, {"r", 14}, {"outer", 13}, {"t", 45}, {"h2", 47}, {"l", 13}, {"r", 15}, {"outer", 14},
      {"t", 46}, {"h2", 48}, {"t", 47}, {"h2", 49}, {"t", 48}, {"h2", 50}, {"t", 49}
    });
  test.AssertHistoryOfGussesIs(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                               );
}
