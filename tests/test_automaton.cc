#include "sdfa.h"

#include <gtest/gtest.h>

#include <vector>
#include <iostream>

#define _D(x) itp::Prec_double_t(x)

namespace itp {

struct Head_position {
  Head_position(const std::string &n, long p)
      : name(n), position(p) {}
        
  std::string name;
  long position;
};

bool operator == (const Head_position &lhs, const Head_position &rhs) {
  return (lhs.name == rhs.name) && (lhs.position == rhs.position);
}

std::ostream & operator << (std::ostream &ost, const Head_position &head) {
  ost << head.name << ' ' << head.position;
  return ost;
}
    
class Automation_for_testing : public itp::Sensing_DFA {
 public:
  Automation_for_testing(Symbol_t alphabet_min_symbol, Symbol_t alphabet_max_symbol);
        
  std::vector<Head_position> get_head_history() const;
  std::vector<Symbol_t> get_guess_history() const;
        
 protected:
  void on_move_head(const Head &head_to_move) override;
  void on_guess(Symbol_t guessed_symbol) override;
        
 private:
  std::vector<Head_position> head_history;
  std::vector<Symbol_t> guess_history;
};
} // of itp

itp::Automation_for_testing::Automation_for_testing(Symbol_t alphabet_min_symbol,
                                                    Symbol_t alphabet_max_symbol)
    : Sensing_DFA {alphabet_min_symbol, alphabet_max_symbol, 0} {}

std::vector<itp::Head_position> itp::Automation_for_testing::get_head_history() const {
  return head_history;
}

std::vector<itp::Symbol_t> itp::Automation_for_testing::get_guess_history() const {
  return guess_history;
}

void itp::Automation_for_testing::on_move_head(const Head &head_to_move) {
  head_history.emplace_back(head_to_move.name(), (long)head_to_move);
}

void itp::Automation_for_testing::on_guess(Symbol_t guessed_symbol) {
  guess_history.push_back(guessed_symbol);
}

TEST(SdfaTest, PredictWordOfLength2) {
  itp::Automation_for_testing a {0, 1};
  itp::Plain_tseries<itp::Symbol_t> ts {0, 1};
  auto res = a.eval_probability(ts);
  auto head_history = a.get_head_history();

  std::vector<itp::Head_position> expected_history {
    {"r", 0},

    // Correction
    // AdvanceOne(1)
    {"h1", 0}, {"inner", 0}, {"h1", 1}, {"t", 0}
  };


  ASSERT_EQ(head_history.size(), expected_history.size());
  for (size_t i = 0; i < expected_history.size(); ++i) {
    EXPECT_EQ(head_history[i], expected_history[i]);
  }

  EXPECT_DOUBLE_EQ(static_cast<itp::Double_t>(res), 0.5*0.25);

  auto guesses = a.get_guess_history();
  std::vector<itp::Symbol_t> expected_guesses {0, 0};
  ASSERT_EQ(guesses.size(), expected_guesses.size());
  for (size_t i = 0; i < expected_guesses.size(); ++i) {
    EXPECT_EQ(guesses[i], expected_guesses[i]);
  }
}

TEST(SdfaTest, PredictWordOfLength5) {
  itp::Automation_for_testing a {0, 1};
  itp::Plain_tseries<itp::Symbol_t> ts {0, 1, 0, 0, 1};
  auto res = a.eval_probability(ts);
  auto head_history = a.get_head_history();

  std::vector<itp::Head_position> expected_history {
    {"r", 0}, {"h1", 0}, {"inner", 0}, {"h1", 1}, {"t", 0}, {"h1", 2}, {"l", 0},
    {"r", 1}, {"outer", 0}, {"t", 1}, {"h1", 3}, {"h2", 0}, {"h2", 1}, {"h2", 2},
    {"h2", 3}, {"outer", 1}, {"t", 2}, {"t", 3}, {"h2", 4}, {"inner", 1}
  };

  ASSERT_EQ(head_history.size(), expected_history.size());
  for (size_t i = 0; i < expected_history.size(); ++i) {
    EXPECT_EQ(head_history[i], expected_history[i]);
  }

  EXPECT_DOUBLE_EQ(static_cast<itp::Double_t>(res), 0.5*0.25*0.5*0.625*0.3);

  auto guesses = a.get_guess_history();
  std::vector<itp::Symbol_t> expected_guesses {0, 0, 0, 0, 0};
  ASSERT_EQ(guesses.size(), expected_guesses.size());
  for (size_t i = 0; i < expected_guesses.size(); ++i) {
    EXPECT_EQ(guesses[i], expected_guesses[i]);
  }   
}

TEST(SdfaTest, PredictWordOfLength10) {
  itp::Automation_for_testing a {0, 1};
  itp::Plain_tseries<itp::Symbol_t> ts {0, 1, 0, 0, 1, 0, 0, 0, 1, 0};
  auto res = a.eval_probability(ts);
  auto head_history = a.get_head_history();

  std::vector<itp::Head_position> expected_history {
    {"r", 0}, {"h1", 0}, {"inner", 0}, {"h1", 1}, {"t", 0}, {"h1", 2}, {"l", 0}, 
    {"r", 1}, {"outer", 0}, {"t", 1}, {"h1", 3}, {"h2", 0}, {"h2", 1}, {"h2", 2},
    {"h2", 3}, {"outer", 1}, {"t", 2}, {"t", 3}, {"h2", 4}, {"inner", 1}, {"h2", 5},
    {"t", 4}, {"h2", 6}, {"h2", 7}, {"t", 5}, {"h2", 8}, {"h2", 9}, {"t", 6}
  };

  ASSERT_EQ(head_history.size(), expected_history.size());
  for (size_t i = 0; i < expected_history.size(); ++i) {
    EXPECT_EQ(head_history[i], expected_history[i]);
  }

  EXPECT_DOUBLE_EQ(static_cast<itp::Double_t>(res), 0.5*0.25*0.5*0.625*0.3*3.5/6*4.5/7*5.5/8*2.5/9*0.65);

  auto guesses = a.get_guess_history();
  std::vector<itp::Symbol_t> expected_guesses { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  ASSERT_EQ(guesses.size(), expected_guesses.size());
  for (size_t i = 0; i < expected_guesses.size(); ++i) {
    EXPECT_EQ(guesses[i], expected_guesses[i]);
  }
}

TEST(SdfaTest, PredictWordOfLength20) {
  itp::Automation_for_testing a {0, 1};
  itp::Plain_tseries<itp::Symbol_t> ts { 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 };
  auto res = a.eval_probability(ts);
  auto head_history = a.get_head_history();

  std::vector<itp::Head_position> expected_history {
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
  };
    
  ASSERT_EQ(head_history.size(), expected_history.size());
  for (size_t i = 0; i < expected_history.size(); ++i) {
    EXPECT_EQ(head_history[i], expected_history[i]);
  }

  EXPECT_DOUBLE_EQ(static_cast<itp::Double_t>(res), _D(0.5)*_D(0.25)*_D(0.5)*_D(0.625)*_D(0.3)*_D(3.5/6)*_D(4.5/7)*_D(5.5/8)*_D(2.5/9)*_D(0.65)*_D(7.5/11)*_D(8.5/12)*_D(0.25)*_D(0.25)*_D(10.5/15)*_D(11.5/16)*_D(12.5/17)*_D(13.5/18)*_D(14.5/19)*_D(0.25)*_D(15.5/21)*_D(16.5/22)*_D(17.5/23)*_D(18.5/24)*_D(0.75)*_D(2.5/3)*_D(0.5/4)*_D(21.5/28)*_D(22.5/29)*_D(23.5/30)*_D(24.5/31)*_D(25.5/32)*_D(0.75)*_D(2.5/3)*_D(0.5/4)*_D(28.5/36)*_D(29.5/37)*_D(30.5/38)*_D(31.5/39)*_D(32.5/40)*_D(0.75)*_D(2.5/3)*_D(3.5/4)*_D(0.5/5)*_D(36.5/45)*_D(37.5)/46*_D(38.5)/47*_D(39.5)/48*_D(40.5)/49*_D(0.75)*_D(2.5)/3);

  auto guesses = a.get_guess_history();
  std::vector<itp::Symbol_t> expected_guesses { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  ASSERT_EQ(guesses.size(), expected_guesses.size());
  for (size_t i = 0; i < expected_guesses.size(); ++i) {
    EXPECT_EQ(guesses[i], expected_guesses[i]);
  }
}
