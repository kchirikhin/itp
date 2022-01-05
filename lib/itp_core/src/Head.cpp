#include "Head.h"

namespace itp {
    Head::Head(long init_pos, std::string init_name, size_t init_id)
        : position_on_tape{init_pos}, head_name{init_name}, head_id{init_id} {}

    void Head::move() {
        ++position_on_tape;
    }

    void Head::move(long pos) {
        position_on_tape = pos;
    }

    void Head::name(const std::string &new_name) {
        head_name = new_name;
    }

    std::string Head::name() const {
        return head_name;
    }

    void Head::id(size_t new_id) {
        head_id = new_id;
    }

    size_t Head::id() const {
        return head_id;
    }

    Head::operator long() const {
        return position_on_tape;
    }

    bool Head::operator == (const Head &other) const {
        return position_on_tape == other.position_on_tape;
    }

    bool Head::operator != (const Head &other) const {
        return !(*this == other);
    }
} // itp
