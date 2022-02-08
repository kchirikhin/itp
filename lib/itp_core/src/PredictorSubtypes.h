#ifndef ITP_PREDICTOR_SUBTYPES_H_INCLUDED_
#define ITP_PREDICTOR_SUBTYPES_H_INCLUDED_

#include "ItpExceptions.h"
#include "Sampler.h"
#include "TableTransformations.h"
#include "Types.h"

#include <cmath>
#include <iostream>
#include <iterator>
#include <memory>

namespace itp
{

template<typename T>
std::ostream& operator<<(std::ostream& ost, const ContinuationsDistribution<T>& table);

constexpr bool IsPowerOfTwo(std::size_t number)
{
	return (number > 0 && ((number & (number - 1)) == 0));
}

template<typename OrigType, typename NewType>
class DistributionPredictor
{
	// static_assert(std::is_arithmetic<NewType>::value, "NewType should be an arithmetic type");

public:
	virtual ~DistributionPredictor() = default;

	virtual ContinuationsDistribution<OrigType> Predict(
		PreprocessedTimeSeries<OrigType, NewType> history,
		size_t horizont,
		const CompressorNamesVec& compressor_groups) const = 0;
};

template<typename OrigType, typename NewType>
using DistributionPredictorPtr = std::shared_ptr<DistributionPredictor<OrigType, NewType>>;

template<typename OrigType, typename NewType>
class PointwisePredictor
{
	// static_assert(std::is_arithmetic<NewType>::value, "NewType should be an arithmetic type");

public:
	virtual ~PointwisePredictor() = default;

	virtual Forecast<OrigType> Predict(
		PreprocessedTimeSeries<OrigType, NewType> history,
		size_t horizont,
		const CompressorNamesVec& compressor_groups) const = 0;
};

template<typename OrigType, typename NewType>
using PointwisePredictorPtr = std::shared_ptr<PointwisePredictor<OrigType, NewType>>;

template<typename OrigType, typename NewType>
class BasicPointwisePredictor : public PointwisePredictor<OrigType, NewType>
{
public:
	BasicPointwisePredictor(DistributionPredictorPtr<OrigType, NewType> distribution_predictor);
	Forecast<OrigType> Predict(
		PreprocessedTimeSeries<OrigType, NewType> history,
		size_t horizont,
		const CompressorNamesVec& compressor_groups) const override;

private:
	DistributionPredictorPtr<OrigType, NewType> distribution_predictor_;
};

/**
 * Decorator pattern implementation.
 *
 */
template<typename OrigType, typename NewType>
class SparsePredictor : public PointwisePredictor<OrigType, NewType>
{
public:
	SparsePredictor(PointwisePredictorPtr<OrigType, NewType> pointwise_predictor, size_t sparse);

	Forecast<OrigType> Predict(
		PreprocessedTimeSeries<OrigType, NewType> history,
		size_t horizont,
		const CompressorNamesVec& compressor_groups) const override final;

private:
	PointwisePredictorPtr<OrigType, NewType> pointwise_predictor_;
	size_t sparse_;
};

template<typename Forward_iterator, typename New_value>
New_value MinValueOfAllTables(Forward_iterator first, Forward_iterator last);

template<typename Forward_iterator, typename T>
void AddValueToEach(Forward_iterator first, Forward_iterator last, T value);

} // namespace itp

template<typename T>
std::ostream& itp::operator<<(std::ostream& ost, const ContinuationsDistribution<T>& table)
{
	ost << "-\t";
	for (const auto& compressor : table.GetFactors())
	{
		ost << compressor << '\t';
	}
	ost << '\n';
	for (const auto& continuation : table.GetIndex())
	{
		ost << continuation << '\t';
		for (const auto& compressor : table.GetFactors())
		{
			ost << table(continuation, compressor) << '\t';
		}
		ost << '\n';
	}

	return ost;
}

template<typename OrigType, typename NewType>
itp::Forecast<OrigType> itp::SparsePredictor<OrigType, NewType>::Predict(
	PreprocessedTimeSeries<OrigType, NewType> history,
	size_t horizont,
	const CompressorNamesVec& compressor_groups) const
{
	std::vector<Forecast<OrigType>> results(sparse_);
	size_t sparsed_horizont = ceil(horizont / static_cast<double>(sparse_));
	for (size_t i = 0; i < sparse_; ++i)
	{
		PreprocessedTimeSeries<OrigType, NewType> sparse_ts_data;
		sparse_ts_data.CopyPreprocessingInfoFrom(history);
		for (size_t j = i; j < history.size(); j += sparse_)
		{
			sparse_ts_data.push_back(history[j]);
		}
		results[i] = pointwise_predictor_->Predict(sparse_ts_data, sparsed_horizont, compressor_groups);
	}

	Forecast<OrigType> result;
	Forecast<OrigType> full_first_steps = pointwise_predictor_->Predict(history, sparsed_horizont, compressor_groups);
	for (size_t i = 0; i < sparsed_horizont; ++i)
	{
		for (const auto& compressor : full_first_steps.GetIndex())
		{
			result(compressor, i) = full_first_steps(compressor, i);
		}
	}

	for (size_t i = 0; i < sparsed_horizont; ++i)
	{
		for (size_t j = 0; j < sparse_; ++j)
		{
			if ((i * sparse_ + j >= sparsed_horizont) && (i * sparse_ + j < horizont))
			{
				for (const auto compressor : results[j].GetIndex())
				{
					result(compressor, i * sparse_ + j) = results[j](compressor, i);
				}
			}
		}
	}
	return result;
}

template<typename OrigType, typename NewType>
itp::BasicPointwisePredictor<OrigType, NewType>::BasicPointwisePredictor(
	DistributionPredictorPtr<OrigType, NewType> distribution_predictor)
	: distribution_predictor_{distribution_predictor}
{
	// DO NOTHING
}

template<typename OrigType, typename NewType>
itp::Forecast<OrigType> itp::BasicPointwisePredictor<OrigType, NewType>::Predict(
	PreprocessedTimeSeries<OrigType, NewType> ts,
	size_t horizont,
	const CompressorNamesVec& compressor_groups) const
{
	auto distribution = distribution_predictor_->Predict(ts, horizont, compressor_groups);
	auto forecasts = ToPointwiseForecasts(distribution, horizont);
	Integrate(forecasts);
	return forecasts;
}

template<typename OrigType, typename NewType>
itp::SparsePredictor<OrigType, NewType>::SparsePredictor(
	PointwisePredictorPtr<OrigType, NewType> pointwise_predictor,
	size_t sparse)
	: pointwise_predictor_{pointwise_predictor}
	, sparse_{sparse}
{
	assert(pointwise_predictor != nullptr);
}

template<typename Forward_iterator, typename New_value>
New_value itp::MinValueOfAllTables(Forward_iterator first, Forward_iterator last)
{
	New_value global_minimum{-1};
	while (first != last)
	{
		auto local_minimum = *std::min_element(begin(*first), end(*first));
		++first;
		if ((local_minimum < global_minimum) || (global_minimum < 0))
		{
			global_minimum = static_cast<New_value>(local_minimum);
		}
	}

	return global_minimum;
}

template<typename Forward_iterator, typename T>
void itp::AddValueToEach(Forward_iterator first, Forward_iterator last, T value)
{
	while (first != last)
	{
		*first += value;
		++first;
	}
}

#endif // ITP_PREDICTOR_SUBTYPES_H_INCLUDED_
