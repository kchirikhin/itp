/**
 * Adapter for ttmath library (it's Big class's interface differs from standard double and such objects cannot be
 * replaced by boost::mpfr objects or standard doubles without modification of code).
 */
#ifndef ITP_BIGNUMS_H_INCLUDED_
#define ITP_BIGNUMS_H_INCLUDED_

#include <ttmath.h>

#include <exception>

namespace bignums
{

class ArithmeticCarryException : public std::runtime_error
{
public:
	explicit ArithmeticCarryException(const std::string& msg)
		: runtime_error(msg)
	{
	}

	template<size_t Exp1, size_t Man1, size_t Exp2, size_t Man2>
	ArithmeticCarryException(
		const std::string& msg,
		const ttmath::Big<Exp1, Man1>& lhs,
		const ttmath::Big<Exp2, Man2>& rhs)
		: runtime_error(
			std::string(msg + "\nNumbers: ") + lhs.ToString() + std::string(" and ") + rhs.ToString()
			+ std::string("."))
	{
	}
};

class InvalidBaseException : public std::runtime_error
{
public:
	explicit InvalidBaseException(const std::string& msg)
		: runtime_error(msg)
	{
	}

	template<size_t Exp, size_t Man>
	InvalidBaseException(const std::string& msg, const ttmath::Big<Exp, Man>& base)
		: runtime_error(std::string(msg + "\nBase: ") + base.ToString() + std::string("."))
	{
	}
};

template<size_t Exp, size_t Man>
class BigDouble
{
public:
	BigDouble()
		: base(0)
	{
		// DO NOTHING
	}

	template<typename T>
	BigDouble(T num)
		: base(num)
	{
		// DO NOTHING
	}

	BigDouble<Exp, Man>& operator+=(const BigDouble<Exp, Man>& other)
	{
		if (!base.Add(other.base))
		{
			// throw Arithmetic_carry_exception("Carry flag while adding two numbers.", base, other.base);
		}

		return *this;
	}

	BigDouble<Exp, Man>& operator-=(const BigDouble<Exp, Man>& other)
	{
		if (!base.Sub(other.base))
		{
			// throw Arithmetic_carry_exception("Carry flag while subtracting two numbers.", base, other.base);
		}

		return *this;
	}

	BigDouble<Exp, Man>& operator*=(const BigDouble<Exp, Man>& other)
	{
		if (!base.Mul(other.base))
		{
			// throw Arithmetic_carry_exception("Carry flag while multiplying two numbers.", base, other.base);
		}

		return *this;
	}

	BigDouble<Exp, Man>& operator/=(const BigDouble<Exp, Man>& other)
	{
		if (!base.Div(other.base))
		{
			// throw Arithmetic_carry_exception("Carry flag while dividing two numbers.", base, other.base);
		}

		return *this;
	}

	operator double() const { return base.ToDouble(); }

	template<size_t E, size_t M>
	friend BigDouble<E, M> pow(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend BigDouble<E, M> log(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend BigDouble<E, M> ceil(const BigDouble<E, M>&);

	template<size_t E, size_t M>
	friend BigDouble<E, M> operator+(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend BigDouble<E, M> operator-(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend BigDouble<E, M> operator*(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend BigDouble<E, M> operator/(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend bool operator<(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend bool operator<=(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend bool operator>(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend bool operator>=(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend bool operator==(const BigDouble<E, M>&, const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend BigDouble<E, M> abs(const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend BigDouble<E, M> operator-(const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend std::string to_string(const BigDouble<E, M>&);
	template<size_t E, size_t M>
	friend std::ostream& operator<<(std::ostream&, const BigDouble<E, M>&);

private:
	ttmath::Big<Exp, Man> base;
};

template<size_t Exp, size_t Man>
BigDouble<Exp, Man> operator+(const BigDouble<Exp, Man>& lhs, const BigDouble<Exp, Man>& rhs)
{
	BigDouble<Exp, Man> result = lhs;
	result += rhs;

	return result;
}

template<size_t Exp, size_t Man>
BigDouble<Exp, Man> operator-(const BigDouble<Exp, Man>& lhs, const BigDouble<Exp, Man>& rhs)
{
	BigDouble<Exp, Man> result = lhs;
	result -= rhs;

	return result;
}

template<size_t Exp, size_t Man>
BigDouble<Exp, Man> operator*(const BigDouble<Exp, Man>& lhs, const BigDouble<Exp, Man>& rhs)
{
	BigDouble<Exp, Man> result = lhs;
	result *= rhs;

	return result;
}

template<size_t Exp, size_t Man>
BigDouble<Exp, Man> operator/(const BigDouble<Exp, Man>& lhs, const BigDouble<Exp, Man>& rhs)
{
	BigDouble<Exp, Man> result = lhs;
	result /= rhs;

	return result;
}

template<size_t Exp, size_t Man>
bool operator<(const BigDouble<Exp, Man>& lhs, const BigDouble<Exp, Man>& rhs)
{
	return lhs.base < rhs.base;
}

template<size_t Exp, size_t Man>
bool operator<=(const BigDouble<Exp, Man>& lhs, const BigDouble<Exp, Man>& rhs)
{
	return lhs.base <= rhs.base;
}

template<size_t Exp, size_t Man>
bool operator>(const BigDouble<Exp, Man>& lhs, const BigDouble<Exp, Man>& rhs)
{
	return lhs.base > rhs.base;
}

template<size_t Exp, size_t Man>
bool operator>=(const BigDouble<Exp, Man>& lhs, const BigDouble<Exp, Man>& rhs)
{
	return lhs.base >= rhs.base;
}

template<size_t Exp, size_t Man>
bool operator==(const BigDouble<Exp, Man>& lhs, const BigDouble<Exp, Man>& rhs)
{
	return lhs.base == rhs.base;
}

template<size_t Exp, size_t Man>
BigDouble<Exp, Man> pow(const BigDouble<Exp, Man>& base, const BigDouble<Exp, Man>& power)
{
	auto tmp = base;

	auto err = tmp.base.PowFrac(power.base);
	if (err == 1)
	{
		throw ArithmeticCarryException(
			"Carry flag while pow the first number to the second number.",
			base.base,
			power.base);
	}
	else if (err == 2)
	{
		throw InvalidBaseException("Invalid base in pow.", base.base);
	}

	return tmp;
}

template<size_t Exp, size_t Man>
BigDouble<Exp, Man> log(const BigDouble<Exp, Man>& base, const BigDouble<Exp, Man>& x)
{
	ttmath::ErrorCode err;
	return ttmath::Log(x.base, base.base, &err);

	/*auto tmp = base;
	  ttmath::ErrorCode err;
	  auto res = ttmath::Log(bbase, xx, &err);
	  if (err == ttmath::err_overflow) {
	  throw Arithmetic_carry_exception("Overflow while computing lograrithm"
	  "of the second number with base of the first number.",
	  bbase.base, xx.base);
	  } else if (err == 2) {
	  throw Invalid_base_exception("Invalid base in pow.", bbase.base);
	  }

	  return tmp;*/
}

template<size_t Exp, size_t Man>
BigDouble<Exp, Man> log2(const BigDouble<Exp, Man>& x)
{
	BigDouble<Exp, Man> base = 2.;
	return log(base, x);
}

template<size_t Exp, size_t Man>
BigDouble<Exp, Man> abs(const BigDouble<Exp, Man>& num)
{
	auto tmp = num;
	tmp.base.Abs();

	return tmp;
}

template<size_t Exp, size_t Man>
BigDouble<Exp, Man> ceil(const BigDouble<Exp, Man>& num)
{
	return ttmath::Ceil(num.base);
}

template<size_t Exp, size_t Man>
BigDouble<Exp, Man> operator-(const BigDouble<Exp, Man>& num)
{
	auto tmp = num;
	tmp.base = -tmp.base;

	return tmp;
}

template<size_t Exp, size_t Man>
std::string to_string(const BigDouble<Exp, Man>& num)
{
	return num.base.ToString();
}

template<size_t E, size_t M>
std::ostream& operator<<(std::ostream& ost, const BigDouble<E, M>& num)
{
	ost << num.base;
	return ost;
}
} // namespace bignums

#endif // ITP_BIGNUMS_H_INCLUDED_
