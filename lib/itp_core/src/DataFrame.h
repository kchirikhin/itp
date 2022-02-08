#ifndef ITP_DFRAME_H_INCLUDED_
#define ITP_DFRAME_H_INCLUDED_

#include <algorithm>
#include <backward/hashtable.h>
#include <iterator>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

template<typename Value, bool is_const = false>
class DataFrameIterator
{
public:
	using value_type = Value;
	using difference_type = size_t;
	using pointer = typename std::conditional<is_const, const Value*, Value*>::type;
	using reference = typename std::conditional<is_const, const Value&, Value&>::type;
	using iterator_category = std::bidirectional_iterator_tag;
	using container_ptr_type = typename std::
		conditional<is_const, const std::vector<std::vector<Value>>*, std::vector<std::vector<Value>>*>::type;

	DataFrameIterator() = default;
	explicit DataFrameIterator(container_ptr_type, size_t = 0, size_t = 0);

	reference operator*() const;

	DataFrameIterator& operator++();
	DataFrameIterator& operator++(int);

	DataFrameIterator& operator--();
	DataFrameIterator& operator--(int);

	bool operator==(const DataFrameIterator&) const;
	bool operator!=(const DataFrameIterator&) const;

private:
	container_ptr_type data_ = nullptr;

	size_t curr_row_pos_ = 0;
	size_t curr_col_pos_ = 0;
};

template<typename Value, bool is_const>
DataFrameIterator<Value, is_const>::DataFrameIterator(container_ptr_type data_ptr, size_t init_row, size_t init_col)
	: data_(data_ptr)
	, curr_row_pos_(init_row)
	, curr_col_pos_(init_col)
{
}

template<typename Value, bool is_const>
typename DataFrameIterator<Value, is_const>::reference DataFrameIterator<Value, is_const>::operator*() const
{
	return (*data_)[curr_row_pos_][curr_col_pos_];
}

template<typename Value, bool is_const>
DataFrameIterator<Value, is_const>& DataFrameIterator<Value, is_const>::operator++()
{
	if (curr_col_pos_ < (*data_)[curr_row_pos_].size() - 1)
	{
		++curr_col_pos_;
	}
	else
	{
		++curr_row_pos_;
		curr_col_pos_ = 0;
	}

	return *this;
}

template<typename Value, bool is_const>
DataFrameIterator<Value, is_const>& DataFrameIterator<Value, is_const>::operator++(int)
{
	auto tmp = *this;
	this->operator++();
	return tmp;
}

template<typename Value, bool is_const>
DataFrameIterator<Value, is_const>& DataFrameIterator<Value, is_const>::operator--()
{
	if (curr_col_pos_ > 0)
	{
		--curr_col_pos_;
	}
	else
	{
		--curr_row_pos_;
		curr_col_pos_ = (*data_)[curr_row_pos_].size() - 1;
	}

	return *this;
}

template<typename Value, bool is_const>
DataFrameIterator<Value, is_const>& DataFrameIterator<Value, is_const>::operator--(int)
{
	auto tmp = *this;
	this->operator++();
	return tmp;
}

template<typename Value, bool is_const>
bool DataFrameIterator<Value, is_const>::operator==(const DataFrameIterator& other) const
{
	return ((curr_row_pos_ == other.curr_row_pos_) && (curr_col_pos_ == other.curr_col_pos_) && (data_ == other.data_));
}

template<typename Value, bool is_const>
bool DataFrameIterator<Value, is_const>::operator!=(const DataFrameIterator& other) const
{
	return !(*this == other);
}

template<typename Index, typename Factor, typename Value>
class DataFrame
{
public:
	using IndexType = Index;
	using FactorType = Factor;
	using ValueType = Value;
	using iterator = DataFrameIterator<Value, false>;
	using const_iterator = DataFrameIterator<Value, true>;

	DataFrame() = default;
	DataFrame(std::initializer_list<IndexType>, std::initializer_list<FactorType> = {});

	template<typename IndexGenerator>
	DataFrame(IndexGenerator, size_t);

	template<typename IndexGenerator, typename FactorsGenerator>
	DataFrame(IndexGenerator, size_t, FactorsGenerator, size_t);

	template<typename ForwardIterator>
	DataFrame(ForwardIterator, ForwardIterator);

	template<typename ForwardIterator1, typename ForwardIterator2>
	DataFrame(ForwardIterator1, ForwardIterator1, ForwardIterator2, ForwardIterator2);

	void AddIndex(const IndexType&);

	template<typename Generator>
	void AddIndex(Generator, size_t);

	template<typename ForwardIterator>
	void AddIndex(ForwardIterator, ForwardIterator);

	void AddFactor(const FactorType&);

	template<typename Generator>
	void AddFactor(Generator, size_t);

	template<typename ForwardIterator>
	void AddFactor(ForwardIterator, ForwardIterator);

	size_t IndexSize() const;
	size_t FactorsSize() const;

	void Join(const DataFrame&);

	std::vector<IndexType> GetIndex() const;
	std::vector<FactorType> GetFactors() const;

	void ReplaceIndexName(const std::string& prev_name, const std::string& new_name);
	void ReplaceFactorName(const std::string& prev_name, const std::string& new_name);

	ValueType& operator()(const IndexType&, const FactorType&);
	const ValueType& operator()(const IndexType&, const FactorType&) const;

	iterator begin();
	const_iterator begin() const;

	iterator end();
	const_iterator end() const;

private:
	std::vector<std::vector<ValueType>> data_;

	std::map<IndexType, size_t> ind_to_row_;
	std::map<FactorType, size_t> fac_to_column_;

	std::vector<IndexType> indexes_;
	std::vector<FactorType> factors_;
};

template<typename Index, typename Factor, typename Value>
DataFrame<Index, Factor, Value>::DataFrame(
	std::initializer_list<IndexType> init_index,
	std::initializer_list<FactorType> init_factors)
{
	AddIndex(std::begin(init_index), std::end(init_index));
	AddFactor(std::begin(init_factors), std::end(init_factors));
}

template<typename Index, typename Factor, typename Value>
template<typename IndexGenerator>
DataFrame<Index, Factor, Value>::DataFrame(IndexGenerator index_generator, size_t index_count)
{
	AddIndex(index_generator, index_count);
}

template<typename Index, typename Factor, typename Value>
template<typename IndexGenerator, typename FactorsGenerator>
DataFrame<Index, Factor, Value>::DataFrame(
	IndexGenerator index_generator,
	size_t index_count,
	FactorsGenerator factors_generator,
	size_t factors_count)
{
	AddIndex(index_generator, index_count);
	AddFactor(factors_generator, factors_count);
}

template<typename Index, typename Factor, typename Value>
template<typename ForwardIterator>
DataFrame<Index, Factor, Value>::DataFrame(ForwardIterator first, ForwardIterator last)
{
	AddIndex(first, last);
}

template<typename Index, typename Factor, typename Value>
template<typename ForwardIterator1, typename ForwardIterator2>
DataFrame<Index, Factor, Value>::DataFrame(
	ForwardIterator1 first1,
	ForwardIterator1 last1,
	ForwardIterator2 first2,
	ForwardIterator2 last2)
{
	AddIndex(first1, last1);
	AddFactor(first2, last2);
}

template<typename Index, typename Factor, typename Value>
void DataFrame<Index, Factor, Value>::AddIndex(const IndexType& index)
{
	ind_to_row_[index] = indexes_.size();
	indexes_.push_back(index);
	data_.emplace_back(FactorsSize(), Value());
}

template<typename Index, typename Factor, typename Value>
template<typename Generator>
void DataFrame<Index, Factor, Value>::AddIndex(Generator generator, size_t count)
{
	auto prev_size = indexes_.size();
	indexes_.resize(prev_size + count);
	data_.resize(prev_size + count, std::vector<Value>(FactorsSize(), Value()));

	for (size_t i = 0; i < count; ++i)
	{
		indexes_[prev_size + i] = generator();
		ind_to_row_[indexes_[prev_size + i]] = prev_size + i;
	}
}

template<typename Index, typename Factor, typename Value>
template<typename ForwardIterator>
void DataFrame<Index, Factor, Value>::AddIndex(ForwardIterator first, ForwardIterator last)
{
	for (auto iter = first; iter != last; ++iter)
	{
		indexes_.push_back(*iter);
		data_.emplace_back(FactorsSize(), Value());
		ind_to_row_[*iter] = data_.size() - 1;
	}
}

template<typename Index, typename Factor, typename Value>
void DataFrame<Index, Factor, Value>::AddFactor(const FactorType& factor)
{
	fac_to_column_[factor] = FactorsSize();
	factors_.push_back(factor);
	for (auto& row : data_)
	{
		row.push_back(Value());
	}
}

template<typename Index, typename Factor, typename Value>
template<typename Generator>
void DataFrame<Index, Factor, Value>::AddFactor(Generator g, size_t count)
{
	auto prev_size = factors_.size();
	factors_.resize(factors_.size() + count);
	for (size_t i = 0; i < count; ++i)
	{
		factors_[prev_size + i] = g();
		fac_to_column_[factors_[prev_size + i]] = prev_size + i;
	}

	for (auto& row : data_)
	{
		row.resize(prev_size + count, Value());
	}
}

template<typename Index, typename Factor, typename Value>
template<typename ForwardIterator>
void DataFrame<Index, Factor, Value>::AddFactor(ForwardIterator first, ForwardIterator last)
{
	size_t sequence_len = 0;
	for (auto iter = first; iter != last; ++iter, ++sequence_len)
	{
		fac_to_column_[*iter] = factors_.size();
		factors_.push_back(*iter);
	}
	for (auto& row : data_)
	{
		row.resize(row.size() + sequence_len, Value());
	}
}

template<typename Index, typename Factor, typename Value>
size_t DataFrame<Index, Factor, Value>::IndexSize() const
{
	return indexes_.size();
}

template<typename Index, typename Factor, typename Value>
size_t DataFrame<Index, Factor, Value>::FactorsSize() const
{
	return factors_.size();
}

template<typename Index, typename Factor, typename Value>
void DataFrame<Index, Factor, Value>::Join(const DataFrame& other)
{
	for (const auto& index : other.indexes_)
	{
		auto row_num = ind_to_row_.find(index);
		if (row_num == std::end(ind_to_row_))
		{
			AddIndex(index);
		}
	}

	for (const auto& factor : other.factors_)
	{
		auto col_num = fac_to_column_.find(factor);
		if (col_num == std::end(fac_to_column_))
		{
			AddFactor(factor);
		}
	}

	for (const auto& index : other.indexes_)
	{
		for (const auto& factor : other.factors_)
		{
			data_[ind_to_row_[index]][fac_to_column_[factor]] = other(index, factor);
		}
	}
}

template<typename Index, typename Factor, typename Value>
std::vector<typename DataFrame<Index, Factor, Value>::IndexType> DataFrame<Index, Factor, Value>::GetIndex() const
{
	return indexes_;
}

template<typename Index, typename Factor, typename Value>
std::vector<typename DataFrame<Index, Factor, Value>::FactorType> DataFrame<Index, Factor, Value>::GetFactors() const
{
	return factors_;
}

template<typename Index, typename Factor, typename Value>
void DataFrame<Index, Factor, Value>::ReplaceIndexName(const std::string& old_name, const std::string& new_name)
{
	ind_to_row_[new_name] = ind_to_row_[old_name];
	ind_to_row_.erase(old_name);
	std::replace(std::begin(indexes_), std::end(indexes_), old_name, new_name);
}

template<typename Index, typename Factor, typename Value>
void DataFrame<Index, Factor, Value>::ReplaceFactorName(const std::string& old_name, const std::string& new_name)
{
	fac_to_column_[new_name] = fac_to_column_[old_name];
	fac_to_column_.erase(old_name);
	std::replace(std::begin(factors_), std::end(factors_), old_name, new_name);
}

template<typename Index, typename Factor, typename Value>
typename DataFrame<Index, Factor, Value>::ValueType& DataFrame<Index, Factor, Value>::operator()(
	const IndexType& index,
	const FactorType& factor)
{
	auto row_ind = ind_to_row_.find(index);
	auto col_ind = fac_to_column_.find(factor);

	if ((row_ind != std::end(ind_to_row_)) && (col_ind != std::end(fac_to_column_)))
	{
		return data_[row_ind->second][col_ind->second];
	}

	if (row_ind == std::end(ind_to_row_))
	{
		AddIndex(index);
	}

	if (col_ind == std::end(fac_to_column_))
	{
		AddFactor(factor);
	}

	return data_[ind_to_row_[index]][fac_to_column_[factor]];
}

template<typename Index, typename Factor, typename Value>
const typename DataFrame<Index, Factor, Value>::ValueType& DataFrame<Index, Factor, Value>::operator()(
	const IndexType& index,
	const FactorType& factor) const
{
	auto row_ind = ind_to_row_.find(index);
	auto col_ind = fac_to_column_.find(factor);

	if (row_ind == std::end(ind_to_row_))
	{
		throw std::range_error("Index out of range.");
	}

	if (col_ind == std::end(fac_to_column_))
	{
		throw std::range_error("Factor out of range.");
	}

	return data_[row_ind->second][col_ind->second];
}

template<typename Index, typename Factor, typename Value>
typename DataFrame<Index, Factor, Value>::iterator DataFrame<Index, Factor, Value>::begin()
{
	return iterator{&data_, 0, 0};
}

template<typename Index, typename Factor, typename Value>
typename DataFrame<Index, Factor, Value>::const_iterator DataFrame<Index, Factor, Value>::begin() const
{
	return const_iterator{&data_, 0, 0};
}

template<typename Index, typename Factor, typename Value>
typename DataFrame<Index, Factor, Value>::iterator DataFrame<Index, Factor, Value>::end()
{
	return iterator{&data_, IndexSize(), 0};
}

template<typename Index, typename Factor, typename Value>
typename DataFrame<Index, Factor, Value>::const_iterator DataFrame<Index, Factor, Value>::end() const
{
	return const_iterator{&data_, IndexSize(), 0};
}

#endif // ITP_DFRAME_H_INCLUDED_
