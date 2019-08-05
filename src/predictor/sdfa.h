/**
 * @file   sdfa.h
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Thu Apr 19 21:28:04 2018
 *
 * @brief  Predicting Sensing Multihead Deterministic Finite-State Automation implementation.
 *
 * The implemetation follows the paper:
 * Smith T. Prediction of infinite words with automation //Theory of Computing Systems. – 2018.
 * – Т. 62. – №. 3. – С. 653-681.
 * In the paper a word is assumed to be infinite, but the implementation, obviously,
 * works only with finite words. So when any head goues out of the word, the
 * automation stops.
 *
 */

#ifndef SDFA_H_INCLUDED
#define SDFA_H_INCLUDED

#include "automaton.h"
#include "head.h"

#include <array>
#include <algorithm>
#include <iostream>
#include <forward_list>
#include <fstream>

/**
 * Macro to break main infinte loop when move is not possible (head
 * to move reaches the end of the word).
 *
 */
#define EXIT_IF_IMPOSSIBLE(procedure_call) if(!(procedure_call)) return false;

namespace itp {
  bool less (long lhs, size_t rhs);

  /**
   * 10-head SDFA, completly as described in the paper.
   *
   */
  class Sensing_DFA : public Multihead_automation<10> {
  public:
      Sensing_DFA(Symbol_t min_symbol, Symbol_t max_symbol, Symbol_t default_symbol);

  private:
    /**
     * Run main "infinite" loop.
     *
     */
    void run() override;

      Symbol_t mean_symbol() const;

      void guess_if_rightmost(const Head &, Is_prediction_confident);
      void guess_if_rightmost(const Head &, Symbol_t, Is_prediction_confident);

    /**
     * All procedures based on pseudocode from the paper and very close to it.
     *
     */
    bool advance_one(size_t);
    bool advance_many(size_t);

    bool correction();
    bool matching();


    /**
     * Actually array of the heads is declared in the base class, but for
     * more similarity with the paper's code, make references to heads with
     * special names in the paper.
     *
     */
    const Head &h3a, &inner, &outer, &l, &r, &t;
    Symbol_t default_symbol;
  };
} // of itp

#endif // SDFA_H_INCLUDED
