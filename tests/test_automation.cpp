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
    expected_history.emplace_back("h2", 6);
    expected_history.emplace_back("h2", 7);
    expected_history.emplace_back("t", 5);
    expected_history.emplace_back("h2", 8);
    expected_history.emplace_back("h2", 9);
    expected_history.emplace_back("t", 6);

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
    expected_history.emplace_back("h2", 6);
    expected_history.emplace_back("h2", 7);
    expected_history.emplace_back("t", 5);
    expected_history.emplace_back("h2", 8);
    expected_history.emplace_back("h2", 9);
    expected_history.emplace_back("t", 6);
    expected_history.emplace_back("h2", 10);
    expected_history.emplace_back("l", 1);
    expected_history.emplace_back("r", 2);
    expected_history.emplace_back("outer", 2);
    expected_history.emplace_back("t", 7);
    expected_history.emplace_back("h2", 11);

    // h2 is fitted (after 8)
    expected_history.emplace_back("t", 8);
    expected_history.emplace_back("h2", 12);
    // h2 is done

    expected_history.emplace_back("l", 2);
    expected_history.emplace_back("r", 3);

    // move h3 until h3 = h2 
    expected_history.emplace_back("h3", 0);
    expected_history.emplace_back("h3", 1);
    expected_history.emplace_back("h3", 2);
    expected_history.emplace_back("h3", 3);
    expected_history.emplace_back("h3", 4);
    expected_history.emplace_back("h3", 5);
    expected_history.emplace_back("h3", 6);
    expected_history.emplace_back("h3", 7);
    expected_history.emplace_back("h3", 8);
    expected_history.emplace_back("h3", 9);
    expected_history.emplace_back("h3", 10);
    expected_history.emplace_back("h3", 11);
    expected_history.emplace_back("h3", 12);

    // AdvanceMany(3)
    expected_history.emplace_back("outer", 3);
    expected_history.emplace_back("t", 9);
    expected_history.emplace_back("t", 10);
    expected_history.emplace_back("t", 11);
    expected_history.emplace_back("t", 12);
    // todo: 2

    expected_history.emplace_back("h3", 13);
    expected_history.emplace_back("inner", 2);
    expected_history.emplace_back("inner", 3);
    expected_history.emplace_back("h3", 14);
    expected_history.emplace_back("t", 13);
    // 7

    expected_history.emplace_back("h3", 15);
    expected_history.emplace_back("h3", 16);
    expected_history.emplace_back("t", 14);
    expected_history.emplace_back("h3", 17);
    expected_history.emplace_back("l", 3);
    expected_history.emplace_back("r", 4);
    expected_history.emplace_back("outer", 4);
    expected_history.emplace_back("t", 15);

    // 8
    expected_history.emplace_back("h3", 18);
    expected_history.emplace_back("t", 16);
    expected_history.emplace_back("h3", 19);
    expected_history.emplace_back("l", 4);
    expected_history.emplace_back("r", 5);

    expected_history.emplace_back("h4", 0);
    expected_history.emplace_back("h4", 1);
    expected_history.emplace_back("h4", 2);
    expected_history.emplace_back("h4", 3);
    expected_history.emplace_back("h4", 4);
    expected_history.emplace_back("h4", 5);
    expected_history.emplace_back("h4", 6);
    expected_history.emplace_back("h4", 7);
    expected_history.emplace_back("h4", 8);
    expected_history.emplace_back("h4", 9);
    expected_history.emplace_back("h4", 10);
    expected_history.emplace_back("h4", 11);
    expected_history.emplace_back("h4", 12);
    expected_history.emplace_back("h4", 13);
    expected_history.emplace_back("h4", 14);
    expected_history.emplace_back("h4", 15);
    expected_history.emplace_back("h4", 16);
    expected_history.emplace_back("h4", 17);
    expected_history.emplace_back("h4", 18);
    expected_history.emplace_back("h4", 19);

    // AdvanceMany(4)
    expected_history.emplace_back("outer", 5);
    // AdvanceOne(4)
    expected_history.emplace_back("t", 17);
    expected_history.emplace_back("t", 18);
    expected_history.emplace_back("t", 19);
    // 2

    expected_history.emplace_back("h4", 20);
    expected_history.emplace_back("inner", 4);
    expected_history.emplace_back("inner", 5);
    expected_history.emplace_back("h4", 21);
    expected_history.emplace_back("t", 20);
    expected_history.emplace_back("h4", 22);

    // 4
    expected_history.emplace_back("l", 5);
    expected_history.emplace_back("r", 6);
    expected_history.emplace_back("outer", 6);
    expected_history.emplace_back("t", 21);

    // 8
    expected_history.emplace_back("h4", 23);
    expected_history.emplace_back("t", 22);
    expected_history.emplace_back("h4", 24);
    expected_history.emplace_back("t", 23);
    expected_history.emplace_back("h4", 25);
    expected_history.emplace_back("t", 24);
    expected_history.emplace_back("h4", 26);
    expected_history.emplace_back("l", 6);
    expected_history.emplace_back("r", 7);

    // Correction is done
    expected_history.emplace_back("h3a", 0);
    expected_history.emplace_back("h3a", 1);
    expected_history.emplace_back("h3a", 2);
    expected_history.emplace_back("h3a", 3);
    expected_history.emplace_back("h3a", 4);
    expected_history.emplace_back("h3a", 5);
    expected_history.emplace_back("h3a", 6);
    expected_history.emplace_back("h3a", 7);
    expected_history.emplace_back("h3a", 8);
    expected_history.emplace_back("h3a", 9);
    expected_history.emplace_back("h3a", 10);
    expected_history.emplace_back("h3a", 11);
    expected_history.emplace_back("h3a", 12);
    expected_history.emplace_back("h3a", 13);
    expected_history.emplace_back("h3a", 14);
    expected_history.emplace_back("h3a", 15);
    expected_history.emplace_back("h3a", 16);
    expected_history.emplace_back("h3a", 17);
    expected_history.emplace_back("h3a", 18);
    expected_history.emplace_back("h3a", 19);

    expected_history.emplace_back("r", 8);

    // correction(1)
    expected_history.emplace_back("h1", 4);
    expected_history.emplace_back("h1", 5);
    expected_history.emplace_back("h1", 6);
    expected_history.emplace_back("h1", 7);
    expected_history.emplace_back("h1", 8);
    expected_history.emplace_back("h1", 9);
    expected_history.emplace_back("h1", 10);
    expected_history.emplace_back("h1", 11);
    expected_history.emplace_back("h1", 12);
    expected_history.emplace_back("h1", 13);
    expected_history.emplace_back("h1", 14);
    expected_history.emplace_back("h1", 15);
    expected_history.emplace_back("h1", 16);
    expected_history.emplace_back("h1", 17);
    expected_history.emplace_back("h1", 18);
    expected_history.emplace_back("h1", 19);
    expected_history.emplace_back("h1", 20);
    expected_history.emplace_back("h1", 21);
    expected_history.emplace_back("h1", 22);
    expected_history.emplace_back("h1", 23);
    expected_history.emplace_back("h1", 24);
    expected_history.emplace_back("h1", 25);
    expected_history.emplace_back("h1", 26);

    // AdvanceOne(1)
    expected_history.emplace_back("t", 25);
    expected_history.emplace_back("t", 26);
    expected_history.emplace_back("h1", 27);
    expected_history.emplace_back("inner", 6);
    expected_history.emplace_back("inner", 7);
    expected_history.emplace_back("inner", 8);
    expected_history.emplace_back("h1", 28);
    expected_history.emplace_back("t", 27);

    expected_history.emplace_back("h1", 29);
    expected_history.emplace_back("l", 7);
    expected_history.emplace_back("r", 9);
    expected_history.emplace_back("outer", 7);
    expected_history.emplace_back("t", 28);

    // 8
    expected_history.emplace_back("h1", 30);
    expected_history.emplace_back("l", 8);
    expected_history.emplace_back("r", 10);
    expected_history.emplace_back("outer", 8);
    expected_history.emplace_back("t", 29);

    // 8
    expected_history.emplace_back("h1", 31);
    expected_history.emplace_back("t", 30);
    expected_history.emplace_back("h1", 32);
    expected_history.emplace_back("t", 31);

    // 10
    expected_history.emplace_back("h1", 33);
    expected_history.emplace_back("t", 32);
    expected_history.emplace_back("h1", 34);

    expected_history.emplace_back("h2", 13);
    expected_history.emplace_back("h2", 14);
    expected_history.emplace_back("h2", 15);
    expected_history.emplace_back("h2", 16);
    expected_history.emplace_back("h2", 17);
    expected_history.emplace_back("h2", 18);
    expected_history.emplace_back("h2", 19);
    expected_history.emplace_back("h2", 20);
    expected_history.emplace_back("h2", 21);
    expected_history.emplace_back("h2", 22);
    expected_history.emplace_back("h2", 23);
    expected_history.emplace_back("h2", 24);
    expected_history.emplace_back("h2", 25);
    expected_history.emplace_back("h2", 26);
    expected_history.emplace_back("h2", 27);
    expected_history.emplace_back("h2", 28);
    expected_history.emplace_back("h2", 29);
    expected_history.emplace_back("h2", 30);
    expected_history.emplace_back("h2", 31);
    expected_history.emplace_back("h2", 32);
    expected_history.emplace_back("h2", 33);
    expected_history.emplace_back("h2", 34);

    // AdvanceMany(2)
    expected_history.emplace_back("outer", 9);
    expected_history.emplace_back("outer", 10);
    expected_history.emplace_back("t", 33);
    expected_history.emplace_back("t", 34);
    expected_history.emplace_back("h2", 35);
    expected_history.emplace_back("inner", 9);
    expected_history.emplace_back("inner", 10);

    // 6
    expected_history.emplace_back("h2", 36);
    expected_history.emplace_back("t", 35);
    expected_history.emplace_back("h2", 37);
    expected_history.emplace_back("l", 9);
    expected_history.emplace_back("r", 11);
    expected_history.emplace_back("outer", 11);
    expected_history.emplace_back("t", 36);

    // 8
    expected_history.emplace_back("h2", 38);
    expected_history.emplace_back("l", 10);
    expected_history.emplace_back("r", 12);
    expected_history.emplace_back("outer", 12);
    expected_history.emplace_back("t", 37);
    expected_history.emplace_back("h2", 39);
    expected_history.emplace_back("t", 38);
    expected_history.emplace_back("h2", 40);
    expected_history.emplace_back("t", 39);

    // 10
    expected_history.emplace_back("h2", 41);
    expected_history.emplace_back("t", 40);
    expected_history.emplace_back("h2", 42);
    expected_history.emplace_back("t", 41);
    expected_history.emplace_back("h2", 43);
    expected_history.emplace_back("l", 11);
    expected_history.emplace_back("r", 13);

    // AdvanceOne(2)
    expected_history.emplace_back("t", 42);
    expected_history.emplace_back("t", 43);
    expected_history.emplace_back("h2", 44);
    expected_history.emplace_back("inner", 11);
    expected_history.emplace_back("inner", 12);
    expected_history.emplace_back("inner", 13);
    expected_history.emplace_back("h2", 45);
    expected_history.emplace_back("t", 44);

    // 8
    expected_history.emplace_back("h2", 46);
    expected_history.emplace_back("l", 12);
    expected_history.emplace_back("r", 14);
    expected_history.emplace_back("outer", 13);
    expected_history.emplace_back("t", 45);
    expected_history.emplace_back("h2", 47);
    expected_history.emplace_back("l", 13);
    expected_history.emplace_back("r", 15);
    expected_history.emplace_back("outer", 14);
    expected_history.emplace_back("t", 46);
    expected_history.emplace_back("h2", 48);

    // 9
    expected_history.emplace_back("t", 47);
    expected_history.emplace_back("h2", 49);
    expected_history.emplace_back("t", 48);
    expected_history.emplace_back("h2", 50);
    expected_history.emplace_back("t", 49);
    
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
