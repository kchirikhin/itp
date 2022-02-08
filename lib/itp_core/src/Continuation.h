#ifndef CONTINUATION_H
#define CONTINUATION_H

#include "PrimitiveDataTypes.h"

#include <algorithm>
#include <string>
#include <vector>

namespace itp
{

template<typename T>
struct Forecast_point
{
	T point;

	// Confidence interval.
	T left_border;
	T right_border;
};

bool Increment(std::vector<Symbol>& sequence, size_t min, size_t max);

template<typename T>
class Continuation
{
public:
	explicit Continuation(T init_symbol = 0);
	Continuation(size_t alphabet, size_t size, T init_symbol = 0);
	Continuation(std::initializer_list<T> list);

	const T& operator[](size_t ind) const;

	Continuation<T> operator++(int);
	Continuation<T>& operator++();

	size_t size() const;

	bool is_init() const;
	bool overflow() const;

	size_t get_alphabet_size() const;

	bool operator<(const Continuation<T>& rhs) const;
	bool operator>(const Continuation<T>& rhs) const;
	bool operator==(const Continuation<T>& rhs) const;
	bool operator!=(const Continuation<T>& rhs) const;
	Continuation<T> operator/(T divisor) const;

	void push_back(const T& item);

	constexpr T* data() noexcept;
	constexpr const T* data() const noexcept;

	typename std::vector<T>::const_iterator cbegin() const;
	typename std::vector<T>::const_iterator cend() const;

private:
	std::vector<T> continuation_;
	size_t alphabet_size_;
	bool is_overflow_;
};

template<typename T>
class Continuations_generator
{
public:
	Continuations_generator(size_t alphabet, size_t size)
		: continuation_(alphabet, size)
	{
	}

	Continuation<T> operator()() { return continuation_++; }

private:
	Continuation<T> continuation_;
};

std::ostream& operator<<(std::ostream&, const Continuation<Symbol>&);
bool operator<(const std::pair<Continuation<Symbol>, Double>&, const std::pair<Continuation<Symbol>, Double>&);

} // namespace itp

template<typename T>
itp::Continuation<T>::Continuation(T init_symbol)
	: Continuation(init_symbol + 1, 1, init_symbol)
{
	// DO NOTHING
}

template<typename T>
itp::Continuation<T>::Continuation(size_t alphabet, size_t size, T init_symbol)
	: continuation_(size, init_symbol)
	, is_overflow_(false)
{
	alphabet_size_ = alphabet;
	if (alphabet_size_ <= init_symbol)
	{
		throw std::range_error(
			"In Continuation's constructor: init symbol out of the alphabet: "_s + std::to_string(init_symbol) + " and "
			+ std::to_string(alphabet_size_) + ".");
	}
}

template<typename T>
itp::Continuation<T>::Continuation(std::initializer_list<T> list)
{
	continuation_.resize(list.size());
	std::copy(begin(list), end(list), begin(continuation_));
	alphabet_size_ = *std::max_element(begin(list), end(list)) + 1;
	is_overflow_ = false;
}

template<typename T>
const T& itp::Continuation<T>::operator[](size_t ind) const
{
	if (continuation_.size() <= ind)
	{
		throw std::range_error("Index out of range");
	}

	return continuation_[ind];
}

template<typename T>
itp::Continuation<T> itp::Continuation<T>::operator++(int)
{
	if (is_overflow_)
	{
		return *this;
	}

	Continuation<T> prev(*this);
	is_overflow_ = !Increment(continuation_, 0, alphabet_size_);
	return prev;
}

template<typename T>
itp::Continuation<T>& itp::Continuation<T>::operator++()
{
	if (!is_overflow_)
	{
		is_overflow_ = !Increment(continuation_, 0, alphabet_size_);
	}

	return *this;
}

template<typename T>
size_t itp::Continuation<T>::size() const
{
	return continuation_.size();
}

template<typename T>
bool itp::Continuation<T>::is_init() const
{
	return std::all_of(begin(continuation_), end(continuation_), [](T item) { return item == 0; });
}

template<typename T>
bool itp::Continuation<T>::overflow() const
{
	return is_overflow_;
}

template<typename T>
size_t itp::Continuation<T>::get_alphabet_size() const
{
	return alphabet_size_;
}

template<typename T>
bool itp::Continuation<T>::operator<(const Continuation<T>& rhs) const
{
	if (continuation_.size() != rhs.size())
	{
		throw std::invalid_argument("In operator <: continuations must have the same length.");
	}

	for (size_t i = continuation_.size(); i > 0; --i)
	{
		if (continuation_[i - 1] < rhs[i - 1])
		{
			return true;
		}
		else if (continuation_[i - 1] > rhs[i - 1])
		{
			return false;
		}
	}

	return false;
}

template<typename T>
bool itp::Continuation<T>::operator>(const Continuation<T>& rhs) const
{
	if (continuation_.size() != rhs.size())
	{
		throw std::invalid_argument("In operator >: Continuations must have the same length.");
	}

	for (size_t i = continuation_.size(); i > 0; --i)
	{
		if (continuation_[i - 1] > rhs[i - 1])
		{
			return true;
		}
		else if (continuation_[i - 1] < rhs[i - 1])
		{
			return false;
		}
	}

	return false;
}

template<typename T>
bool itp::Continuation<T>::operator==(const Continuation<T>& rhs) const
{
	if (continuation_.size() != rhs.size())
	{
		return false;
	}

	for (size_t i = continuation_.size(); i > 0; --i)
	{
		if (continuation_[i - 1] != rhs[i - 1])
		{
			return false;
		}
	}

	return true;
}

template<typename T>
bool itp::Continuation<T>::operator!=(const Continuation<T>& rhs) const
{
	return !(*this == rhs);
}

template<typename T>
itp::Continuation<T> itp::Continuation<T>::operator/(T divisor) const
{
	Continuation<T> result(*this);
	std::transform(
		begin(continuation_),
		end(continuation_),
		begin(result.continuation_),
		[&divisor](T value) { return value / divisor; });
	result.alphabet_size_ /= divisor;
	result.is_overflow_ = false;

	return result;
}

template<typename T>
void itp::Continuation<T>::push_back(const T& item)
{
	if (item >= alphabet_size_)
	{
		throw std::range_error(
			"In Continuation::push_back: item '"_s + std::to_string(item) + "' is greater than alphabet size "
			+ std::to_string(alphabet_size_) + ".");
	}

	continuation_.push_back(item);
}

template<typename T>
constexpr T* itp::Continuation<T>::data() noexcept
{
	return continuation_.data();
}

template<typename T>
constexpr const T* itp::Continuation<T>::data() const noexcept
{
	return continuation_.data();
}

template<typename T>
typename std::vector<T>::const_iterator itp::Continuation<T>::cbegin() const
{
	return continuation_.cbegin();
}

template<typename T>
typename std::vector<T>::const_iterator itp::Continuation<T>::cend() const
{
	return continuation_.cend();
}

namespace std
{
template<>
struct hash<itp::Continuation<itp::Symbol>>
{
	size_t operator()(const itp::Continuation<itp::Symbol>& c) const
	{
		size_t h = 0;
		for (size_t i = 0; i < c.size(); ++i)
		{
			h = (2 * h + c[i]) % q;
		}

		return h;
	}

	static const size_t q = 32452843;
};

template<>
struct equal_to<itp::Continuation<itp::Symbol>>
{
	bool operator()(const itp::Continuation<itp::Symbol>& r, const itp::Continuation<itp::Symbol>& r2) const
	{
		return r == r2;
	}
};
} // namespace std

#endif // CONTINUATION_H
