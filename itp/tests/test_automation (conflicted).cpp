#include "sdfa.h"

#include <gtest/gtest.h>

#include <vector>
#include <iostream>

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

    std::vector<itp::Head_position> expected_history;
    expected_history.emplace_back("r", 0);

    // Correction
    // AdvanceOne(1)
    expected_history.emplace_back("h1", 0);
    expected_history.emplace_back("inner", 0);
    expected_history.emplace_back("h1", 1);
    expected_history.emplace_back("t", 0);

    ASSERT_EQ(head_history.size(), expected_history.size());
    for (size_t i = 0; i < expected_history.size(); ++i) {
        EXPECT_EQ(head_history[i], expected_history[i]);
    }

    EXPECT_DOUBLE_EQ(res, 0.5*0.25);

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

    std::vector<itp::Head_position> expected_history;
    expected_history.emplace_back("r", 0);
    expected_history.emplace_back("h1", 0);
    expected_history.emplace_back("inner", 0);
    expected_history.emplace_back("h1", 1);
    expected_history.emplace_back("t", 0);
    expected_history.emplace_back("h1", 2);
    expected_history.emplace_back("l", 0);
    expected_history.emplace_back("r", 1);
    expected_history.emplace_back("outer", 0);
    expected_history.emplace_back("t", 1);
    expected_history.emplace_back("h1", 3);
    expected_history.emplace_back("h2", 0);
    expected_history.emplace_back("h2", 1);
    expected_history.emplace_back("h2", 2);
    expected_history.emplace_back("h2", 3);
    expected_history.emplace_back("outer", 1);
    expected_history.emplace_back("t", 2);
    expected_history.emplace_back("t", 3);
    expected_history.emplace_back("h2", 4);
    expected_history.emplace_back("inner", 1);

    ASSERT_EQ(head_history.size(), expected_history.size());
    for (size_t i = 0; i < expected_history.size(); ++i) {
        EXPECT_EQ(head_history[i], expected_history[i]);
    }

    EXPECT_DOUBLE_EQ(res, 0.5*0.25*0.5*0.625*0.3);

    auto guesses = a.get_guess_history();
    std::vector<itp::Symbol_t> expected_guesses {0, 0, 0, 0, 0};
    ASSERT_EQ(guesses.size(), expected_guesses.size());
    for (size_t i = 0; i < expected_guesses.size(); ++i) {
        EXPECT_EQ(guesses[i], expected_guesses[i]);
    }   
}

TEST(SdfaTest, PredictWordOfLength10) {
    itp::Automation_for_testing a {0, 1};
    itp::Plain_tseries<itp::Symbol_t> ts {0, 1, 0, 0, 1, 0};
    auto res = a.eval_probability(ts);
    auto head_history = a.get_head_history();

    std::vector<itp::Head_position> expected_history;
    expected_history.emplace_back("r", 0);
    expected_history.emplace_back("h1", 0);
    expected_history.emplace_back("inner", 0);
    expected_history.emplace_back("h1", 1);
    expected_history.emplace_back("t", 0);
    expected_history.emplace_back("h1", 2);
    expected_history.emplace_back("l", 0);
    expected_history.emplace_back("r", 1);
    expected_history.emplace_back("outer", 0);
    expected_history.emplace_back("t", 1);
    expected_history.emplace_back("h1", 3);
    expected_history.emplace_back("h2", 0);
    expected_history.emplace_back("h2", 1);
    expected_history.emplace_back("h2", 2);
    expected_history.emplace_back("h2", 3);
    expected_history.emplace_back("outer", 1);
    expected_history.emplace_back("t", 2);
    expected_history.emplace_back("t", 3);
    expected_history.emplace_back("h2", 4);
    expected_history.emplace_back("inner", 1);
    expected_history.emplace_back("h2", 5);
    expected_history.emplace_back("t", 4);

    ASSERT_EQ(head_history.size(), expected_history.size());
    for (size_t i = 0; i < expected_history.size(); ++i) {
        EXPECT_EQ(head_history[i], expected_history[i]);
    }

    EXPECT_DOUBLE_EQ(res, 0.5*0.25*0.5*0.625*0.3*0.5833333333333333);

    auto guesses = a.get_guess_history();
    std::vector<itp::Symbol_t> expected_guesses {0, 0, 0, 0, 0, 0};
    ASSERT_EQ(guesses.size(), expected_guesses.size());
    for (size_t i = 0; i < expected_guesses.size(); ++i) {
        EXPECT_EQ(guesses[i], expected_guesses[i]);
    }   
}