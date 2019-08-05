#include "sdfa.h"

namespace itp {
    bool less (long lhs, size_t rhs) {
        if ((lhs < 0) || (static_cast<size_t>(lhs) < rhs)) {
            return true;
        }

        return false;
    }

    Sensing_DFA::Sensing_DFA(Symbol_t min_symbol, Symbol_t max_symbol, Symbol_t default_symbol)
        : Multihead_automation<10> {min_symbol, max_symbol}, h3a{h(0)}, inner{h(5)}, outer{h(6)}, l{h(7)},
        r{h(8)}, t{h(9)}, default_symbol {default_symbol} {
            set_head_name(0, "h3a");
            set_head_name(1, "h1");
            set_head_name(2, "h2");
            set_head_name(3, "h3");
            set_head_name(4, "h4");
            set_head_name(5, "inner");
            set_head_name(6, "outer");
            set_head_name(7, "l");
            set_head_name(8, "r");
            set_head_name(9, "t");
        }

    void Sensing_DFA::run() {
        while (less(h(4), a.size())) {
            guess_if_rightmost(r, Is_prediction_confident::no);
            if (!move(r) || !correction() || !matching()) {
                return;
            }
        }
    }

    Symbol_t Sensing_DFA::mean_symbol() const {
        return (get_max_symbol() + get_min_symbol()) / 2;
    }

    void Sensing_DFA::guess_if_rightmost(const Head &h, Is_prediction_confident confidence) {
        guess_if_rightmost(h, a[h], confidence);
    }

    void Sensing_DFA::guess_if_rightmost(const Head &h, Symbol_t predicted_symbol,
                                         Is_prediction_confident confidence) {
        if (is_rightmost(h)) {
            if (confidence == Is_prediction_confident::yes) {
                guess(predicted_symbol, confidence);
            } else {
                guess(mean_symbol(), confidence);
            }
        }
    }

    bool Sensing_DFA::advance_one(size_t i) {
        while (t != h(i)) {
            move(t);
        }

        guess_if_rightmost(h(i), Is_prediction_confident::no);
        EXIT_IF_IMPOSSIBLE(move(h(i)));

        while (inner != r) {
            move(inner);
        }

        while (l != inner) {
            if (a[t] == a[h(i)]) {
                EXIT_IF_IMPOSSIBLE(move(l, r, outer));
            } else {
                while (inner != r) {
                    move(inner);
                }

                guess_if_rightmost(h(i), Is_prediction_confident::no);
                EXIT_IF_IMPOSSIBLE(move(h(i)))
                    }

            EXIT_IF_IMPOSSIBLE(move(t));
            guess_if_rightmost(h(i), Is_prediction_confident::no);
            EXIT_IF_IMPOSSIBLE(move(h(i)));
        }

        while (a[t] == a[h(i)]) {
            EXIT_IF_IMPOSSIBLE(move(t));
            guess_if_rightmost(h(i), a[t], Is_prediction_confident::yes);
            EXIT_IF_IMPOSSIBLE(move(h(i)));
        }

        return true;
    }

    bool Sensing_DFA::advance_many(size_t i) {
        while (outer != r) {
            move(outer);
        }
        while (l != outer) {
            EXIT_IF_IMPOSSIBLE(advance_one(i) && move(l, r));
        }

        return true;
    }

    bool Sensing_DFA::correction() {
        while (h(1) != h(4)) {
            move(h(1));
        }
        EXIT_IF_IMPOSSIBLE(advance_one(1));

        while (h(2) != h(1)) {
            move(h(2));
        }
        EXIT_IF_IMPOSSIBLE(advance_many(2));

        while (h(3) != h(2)) {
            move(h(3));
        }
        EXIT_IF_IMPOSSIBLE(advance_many(3));

        while (h(4) != h(3)) {
            move(h(4));
        }
        EXIT_IF_IMPOSSIBLE(advance_many(4));

        return true;
    }

    bool Sensing_DFA::matching() {
        while (less(h(4), a.size())) {
            while (h3a != h(3)) {
                move(h3a);
            }

            while ((a[h(1)] == a[h(2)]) &&
                   (a[h(2)] == a[h(3)]) &&
                   (a[h(3)] == a[h(4)])) {
                EXIT_IF_IMPOSSIBLE(move(h(1), h(2), h3a, h(3)));
                guess_if_rightmost(h(4), a[h(2)], Is_prediction_confident::yes);
                EXIT_IF_IMPOSSIBLE(move(h(4)));
            }

            if (a[h(2)] != a[h(4)]) {
                break;
            }

            while ((a[h(2)] == a[h(3)])
                   && (a[h(3)] == a[h(4)])) {
                EXIT_IF_IMPOSSIBLE(move(h(2), h(3)));
                guess_if_rightmost(h(4), a[h(3)], Is_prediction_confident::yes);
                EXIT_IF_IMPOSSIBLE(move(h(4)));
            }

            if (a[h(3)] != a[h(4)]) {
                break;
            }

            while ((a[h3a] == a[h(3)]) &&
                   (a[h(3)] == a[h(4)])) {
                EXIT_IF_IMPOSSIBLE(move(h3a, h(3)));
                guess_if_rightmost(h(4), a[h3a], Is_prediction_confident::yes);
                EXIT_IF_IMPOSSIBLE(move(h(4)));
            }

            if (a[h3a] != a[h(4)]) {
                break;
            }

            while ((h3a != h(3))) {
                EXIT_IF_IMPOSSIBLE(move(h3a));
                guess_if_rightmost(h(4), a[h3a], Is_prediction_confident::yes);
                EXIT_IF_IMPOSSIBLE(move(h(4)));
            }
        }

        return true;
    }
} // itp
