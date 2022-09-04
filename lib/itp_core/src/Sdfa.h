/**
 * @file   Sdfa.h
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Thu Apr 19 21:28:04 2018
 *
 * @brief  Predicting Sensing Multihead Deterministic Finite-State Automaton implementation.
 *
 * The implemetation follows the paper:
 * Smith T. Prediction of infinite words with automata //Theory of Computing Systems. – 2018.
 * – Т. 62. – №. 3. – С. 653-681.
 * In the paper a word is assumed to be infinite, but the implementation, obviously,
 * works only with finite words. So when any head goues out of the word, the
 * automaton stops.
 */

#ifndef SDFA_H_INCLUDED
#define SDFA_H_INCLUDED

#include "Automaton.h"
#include "Head.h"

#include <algorithm>
#include <array>
#include <forward_list>
#include <fstream>
#include <iostream>

/**
 * Macro to break main infinte loop when move is not possible (head
 * to move reaches the end of the word).
 */
#define EXIT_IF_IMPOSSIBLE(procedure_call) \
	if (!(procedure_call)) \
		return false;

namespace itp
{

bool Less(long lhs, size_t rhs);

/**
 * 10-head SDFA, completely as described in the paper.
 */
class SensingDFA : public MultiheadAutomaton<10>
{
public:
	SensingDFA(Symbol min_symbol, Symbol max_symbol);

private:
	/**
	 * Run main "infinite" loop.
	 */
	void Run() override;

	Symbol MeanSymbol() const;

	void GuessIfRightmost(const Head&, IsPredictionConfident);
	void GuessIfRightmost(const Head&, Symbol, IsPredictionConfident);

	/**
	 * All procedures based on pseudocode from the paper and very close to it.
	 */
	bool AdvanceOne(size_t);
	bool AdvanceMany(size_t);

	bool Correction();
	bool Matching();

	/**
	 * Actually array of the heads is declared in the base class, but for
	 * more similarity with the paper's code, make references to heads with
	 * special names in the paper.
	 */
	const Head &h3a_, &inner_, &outer_, &l_, &r_, &t_;
};

} // namespace itp

#endif // SDFA_H_INCLUDED
