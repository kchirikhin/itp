#include "Head.h"

namespace itp
{

Head::Head(long init_pos, std::string init_name, size_t init_id)
	: position_on_tape_{init_pos}
	, head_name_{init_name}
	, head_id_{init_id}
{
}

void Head::Move()
{
	++position_on_tape_;
}

void Head::Move(long pos)
{
	position_on_tape_ = pos;
}

void Head::Name(const std::string& new_name)
{
	head_name_ = new_name;
}

std::string Head::Name() const
{
	return head_name_;
}

void Head::Id(size_t new_id)
{
	head_id_ = new_id;
}

size_t Head::Id() const
{
	return head_id_;
}

Head::operator long() const
{
	return position_on_tape_;
}

bool Head::operator==(const Head& other) const
{
	return position_on_tape_ == other.position_on_tape_;
}

bool Head::operator!=(const Head& other) const
{
	return !(*this == other);
}

} // namespace itp
