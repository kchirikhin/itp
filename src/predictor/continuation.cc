#include "continuation.h"

#include <cassert>

namespace itp {
    bool increment(std::vector<Symbol> &sequence, size_t min, size_t max) {
        if (min > max) {
            throw std::invalid_argument(_S("In increment function: min should not be greater than max"
                                           "(provided values are ") +
                                        std::to_string(min) + _S(" and ") + std::to_string(max) + _S("."));
        }

        assert(max <= 10000);
        assert(sequence.size() > 0);

        for (size_t i = 0; i < sequence.size(); ++i) {
            if (sequence[i] < min) {
                throw std::invalid_argument(_S("In increment function: passed sequence has an element less"
                                               "than minimal value: ") +
                                            std::to_string(sequence[i]) + _S(" while min is: ") +
                                            std::to_string(min) + _S("."));
            }

            if (sequence[i] + 1 < max) {
                ++sequence[i];
                return true;
            }

            sequence[i] = min;
        }

        return false;
    }

    std::ostream & operator << (std::ostream &ost, const Continuation<Symbol> &cont) {
        for (size_t i = 0; i < cont.size(); ++i) {
            ost << (int)cont[i];
        }

        return ost;
    }

    bool operator < (const std::pair<Continuation<Symbol>, Double> &lhs,
                     const std::pair<Continuation<Symbol>, Double> &rhs) {
        return lhs.second < rhs.second;
    }
} // itp
