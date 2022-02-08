#ifndef ITP_PREPROC_INFO_H_INCLUDED_
#define ITP_PREPROC_INFO_H_INCLUDED_

#include "PrimitiveDataTypes.h"

#include <cassert>
#include <limits>
#include <stack>

namespace itp
{

/**
 * Class contains all information about preliminary actions with a time series. This information
 * allows to apply inverse transformations.
 */
template<typename T>
class PreprocInfo
{
public:
	size_t AppliedDiffCount() const;
	void PushLastDiffValue(const T&);
	T PopLastDiffValue();

	void SetSamplingAlphabet(size_t);
	size_t GetSamplingAlphabet() const;

	void SetDesampleTable(const std::vector<T>&);
	const std::vector<T>& GetDesampleTable() const;
	void ClearDesampleTable();
	bool IsSampled() const;

	void SetDesampleIndent(Double);
	Double GetDesampleIndent() const;

	void CopyPreprocessingInfoFrom(const PreprocInfo<T>&);

private:
	std::stack<T> last_values_;

	size_t alphabet_;

	std::vector<T> desample_table_;
	Double desample_indent_;
	bool sampled_ = false;
};

} // namespace itp

template<typename T>
size_t itp::PreprocInfo<T>::AppliedDiffCount() const
{
	return std::size(last_values_);
}

template<typename T>
void itp::PreprocInfo<T>::PushLastDiffValue(const T& value)
{
	last_values_.push(value);
}

template<typename T>
T itp::PreprocInfo<T>::PopLastDiffValue()
{
	T res;
	if (0 < std::size(last_values_))
	{
		res = last_values_.top();
		last_values_.pop();
	}

	return res;
}

template<typename T>
void itp::PreprocInfo<T>::SetSamplingAlphabet(size_t new_alphabet)
{
	alphabet_ = new_alphabet;
}

template<typename T>
size_t itp::PreprocInfo<T>::GetSamplingAlphabet() const
{
	return alphabet_;
}

template<typename T>
void itp::PreprocInfo<T>::SetDesampleTable(const std::vector<T>& new_table)
{
	desample_table_ = new_table;
	sampled_ = true;
}

template<typename T>
const std::vector<T>& itp::PreprocInfo<T>::GetDesampleTable() const
{
	return desample_table_;
}

template<typename T>
void itp::PreprocInfo<T>::ClearDesampleTable()
{
	desample_table_.clear();
	sampled_ = false;
}

template<typename T>
bool itp::PreprocInfo<T>::IsSampled() const
{
	return sampled_;
}

template<typename T>
void itp::PreprocInfo<T>::SetDesampleIndent(Double new_indent)
{
	desample_indent_ = new_indent;
}

template<typename T>
itp::Double itp::PreprocInfo<T>::GetDesampleIndent() const
{
	return desample_indent_;
}

template<typename T>
void itp::PreprocInfo<T>::CopyPreprocessingInfoFrom(const PreprocInfo<T>& src)
{
	last_values_ = src.last_values_;
	alphabet_ = src.alphabet_;
	desample_table_ = src.desample_table_;
	desample_indent_ = src.desample_indent_;
	sampled_ = src.sampled_;
}

#endif // of ITP_PREPROC_INFO_H_INCLUDED_
