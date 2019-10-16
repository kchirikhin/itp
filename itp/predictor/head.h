/**
 * @file   head.h
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Thu Apr 12 20:21:44 2018
 *
 * @brief Defenition of the Head class for Multihead Deterministic Finite State
 * Automation.
 *
 *
 */

#ifndef HEAD_H_INCLUDED
#define HEAD_H_INCLUDED

#include <iostream>

namespace itp {

  const long INIT_HEAD_POS = -1;
  /**
   * A single head of Multihead DFA.
   *
   */
  class Head {
  public:
    Head() = default;

    Head(long, std::string = "", size_t = 0);

    /**
     * Moves head to the next position from the right on the tape.
     *
     */
    void move();
    void move(long pos);

    /**
     * Set human-readable name of the head for visualization.
     *
     */
    void name(const std::string &);

    /**
     * Read name of the head.
     *
     *
     * @return Human-friendly name of the head.
     */
    std::string name() const;

    void id(size_t);

    /**
     * Returns predefined numerical indetifier of the head.
     *
     *
     * @return Numerical identifier.
     */
    size_t id() const;

    /**
     * Casts head to the unsigned integer to enable using of heads as
     * indeces in words/arrays.
     *
     *
     * @return Current position of the head from the beggining of the tape.
     */
    operator long() const;

    /**
     * Compares two heads by positions only.
     *
     * @param other Right-sight head.
     *
     * @return True, if heads have a same positions, and false otherwise.
     */
    bool operator == (const Head &other) const;
    bool operator != (const Head &other) const;

    // To enable indexing of word by head and hide implementation of the head from
    // the user.
    friend class Word;
  private:
    long position_on_tape = 0;  /**< Signed integer to allow position before the first letter. */
    std::string head_name;
    size_t head_id;
  };
} // of itp

#endif // HEAD_H_INCLUDED
