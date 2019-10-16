#include "sdfa.h"

namespace itp {
bool less (long lhs, size_t rhs) {
  if ((lhs < 0) || (static_cast<size_t>(lhs) < rhs)) {
    return true;
  }

  return false;
}

Sensing_DFA::Sensing_DFA(Symbol min_symbol, Symbol max_symbol, Symbol default_symbol)
    : MultiheadAutomaton<10> {min_symbol, max_symbol}, h3a{h(0)}, inner{h(5)}, outer{h(6)}, l{h(7)},
  r{h(8)}, t{h(9)}, default_symbol {default_symbol} {
    SetHeadName(0, "h3a");
    SetHeadName(1, "h1");
    SetHeadName(2, "h2");
    SetHeadName(3, "h3");
    SetHeadName(4, "h4");
    SetHeadName(5, "inner");
    SetHeadName(6, "outer");
    SetHeadName(7, "l");
    SetHeadName(8, "r");
    SetHeadName(9, "t");
  }

void Sensing_DFA::Run() {
  while (less(h(4), a.size())) {
    guess_if_rightmost(r, IsPredictionConfident::kNo);
    if (!Move(r) || !correction() || !matching()) {
      return;
    }
  }
}

Symbol Sensing_DFA::mean_symbol() const {
  return (GetMaxSymbol() + GetMinSymbol()) / 2;
}

void Sensing_DFA::guess_if_rightmost(const Head &h, IsPredictionConfident confidence) {
  guess_if_rightmost(h, a[h], confidence);
}

void Sensing_DFA::guess_if_rightmost(const Head &h, Symbol predicted_symbol,
                                     IsPredictionConfident confidence) {
  if (IsRightmost(h)) {
    if (confidence == IsPredictionConfident::kYes) {
      Guess(predicted_symbol, confidence);
    } else {
      Guess(mean_symbol(), confidence);
    }
  }
}

bool Sensing_DFA::advance_one(size_t i) {
  while (t != h(i)) {
    Move(t);
  }

  guess_if_rightmost(h(i), IsPredictionConfident::kNo);
  EXIT_IF_IMPOSSIBLE(Move(h(i)));

  while (inner != r) {
    Move(inner);
  }

  while (l != inner) {
    if (a[t] == a[h(i)]) {
      EXIT_IF_IMPOSSIBLE(Move(l, r, outer));
    } else {
      while (inner != r) {
        Move(inner);
      }

      guess_if_rightmost(h(i), IsPredictionConfident::kNo);
      EXIT_IF_IMPOSSIBLE(Move(h(i)))
    }

    EXIT_IF_IMPOSSIBLE(Move(t));
    guess_if_rightmost(h(i), IsPredictionConfident::kNo);
    EXIT_IF_IMPOSSIBLE(Move(h(i)));
  }

  while (a[t] == a[h(i)]) {
    EXIT_IF_IMPOSSIBLE(Move(t));
    guess_if_rightmost(h(i), a[t], IsPredictionConfident::kYes);
    EXIT_IF_IMPOSSIBLE(Move(h(i)));
  }

  return true;
}

bool Sensing_DFA::advance_many(size_t i) {
  while (outer != r) {
    Move(outer);
  }
  while (l != outer) {
    EXIT_IF_IMPOSSIBLE(advance_one(i) && Move(l, r));
  }

  return true;
}

bool Sensing_DFA::correction() {
  while (h(1) != h(4)) {
    Move(h(1));
  }
  EXIT_IF_IMPOSSIBLE(advance_one(1));

  while (h(2) != h(1)) {
    Move(h(2));
  }
  EXIT_IF_IMPOSSIBLE(advance_many(2));

  while (h(3) != h(2)) {
    Move(h(3));
  }
  EXIT_IF_IMPOSSIBLE(advance_many(3));

  while (h(4) != h(3)) {
    Move(h(4));
  }
  EXIT_IF_IMPOSSIBLE(advance_many(4));

  return true;
}

bool Sensing_DFA::matching() {
  while (less(h(4), a.size())) {
    while (h3a != h(3)) {
      Move(h3a);
    }

    while ((a[h(1)] == a[h(2)]) &&
           (a[h(2)] == a[h(3)]) &&
           (a[h(3)] == a[h(4)])) {
      EXIT_IF_IMPOSSIBLE(Move(h(1), h(2), h3a, h(3)));
      guess_if_rightmost(h(4), a[h(2)], IsPredictionConfident::kYes);
      EXIT_IF_IMPOSSIBLE(Move(h(4)));
    }

    if (a[h(2)] != a[h(4)]) {
      break;
    }

    while ((a[h(2)] == a[h(3)])
           && (a[h(3)] == a[h(4)])) {
      EXIT_IF_IMPOSSIBLE(Move(h(2), h(3)));
      guess_if_rightmost(h(4), a[h(3)], IsPredictionConfident::kYes);
      EXIT_IF_IMPOSSIBLE(Move(h(4)));
    }

    if (a[h(3)] != a[h(4)]) {
      break;
    }

    while ((a[h3a] == a[h(3)]) &&
           (a[h(3)] == a[h(4)])) {
      EXIT_IF_IMPOSSIBLE(Move(h3a, h(3)));
      guess_if_rightmost(h(4), a[h3a], IsPredictionConfident::kYes);
      EXIT_IF_IMPOSSIBLE(Move(h(4)));
    }

    if (a[h3a] != a[h(4)]) {
      break;
    }

    while ((h3a != h(3))) {
      EXIT_IF_IMPOSSIBLE(Move(h3a));
      guess_if_rightmost(h(4), a[h3a], IsPredictionConfident::kYes);
      EXIT_IF_IMPOSSIBLE(Move(h(4)));
    }
  }

  return true;
}
} // itp
