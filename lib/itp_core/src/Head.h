/**
 * @file   head.h
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Thu Apr 12 20:21:44 2018
 *
 * @brief Defenition of the Head class for Multihead Deterministic Finite State
 * Automaton.
 */

#ifndef HEAD_H_INCLUDED
#define HEAD_H_INCLUDED

#include <iostream>

namespace itp
{

const long INIT_HEAD_POS = -1;

/**
 * A single head of Multihead DFA.
 */
class Head
{
public:
	Head() = default;

	Head(long, std::string = "", size_t = 0);

	/**
	 * Moves head to the next position from the right on the tape.
	 */
	void Move();
	void Move(long pos);

	/**
	 * Set human-readable name of the head for visualization.
	 */
	void Name(const std::string&);

	/**
	 * Read name of the head.
	 *
	 * \return Human-friendly name of the head.
	 */
	std::string Name() const;

	void Id(size_t);

	/**
	 * Returns predefined numerical identifier of the head.
	 *
	 * \return Numerical identifier.
	 */
	size_t Id() const;

	/**
	 * Casts head to the unsigned integer to enable using of heads as
	 * indexes in words/arrays.
	 *
	 * \return Current position of the head from the beginning of the tape.
	 */
	operator long() const;

	/**
	 * Compares two heads by positions only.
	 *
	 * \param[in] other Right-sight head.
	 *
	 * \return True, if heads have a same positions, and false otherwise.
	 */
	bool operator==(const Head& other) const;
	bool operator!=(const Head& other) const;

	// To enable indexing of word by head and hide implementation of the head from
	// the user.
	friend class Word;

private:
	long position_on_tape_ = 0; /**< Signed integer to allow position before the first letter. */
	std::string head_name_;
	size_t head_id_;
};
} // namespace itp

#endif // HEAD_H_INCLUDED
