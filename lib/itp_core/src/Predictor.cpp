#include "PredictorPrivate.h"

#include "Builders.h"
#include "CompressionPrediction.h"
#include "NonCompressionAlgorithmAdaptor.h"

namespace itp
{

namespace
{

void CheckArgs(size_t horizont, size_t difference, int sparse)
{
	if (50 < horizont)
	{
		throw std::invalid_argument("Forecasting horizont is too long (> 50)");
	}

	if (10 < difference)
	{
		throw std::invalid_argument("Difference order is too big (> 10)");
	}

	if (20 < sparse)
	{
		throw std::invalid_argument("Sparse value is too big (> 20)");
	}
}

void CheckQuantaCountRange(size_t quanta_count)
{
	if ((0 == quanta_count) || (256 < quanta_count))
	{
		throw std::invalid_argument("Quants count should be greater than zero and not greater than 256");
	}
}

} // namespace

std::vector<itp::VectorDouble> Convert(const std::vector<std::vector<double>>& series)
{
	if (series.empty())
	{
		return {};
	}

	const auto number_of_points = std::size(series[0]);
	for (size_t i = 1; i < std::size(series); ++i)
	{
		if (std::size(series[i]) != number_of_points)
		{
			throw itp::DifferentHistoryLengthsError(
				"The length of series with number " + std::to_string(i)
				+ " differs from the length of the first series");
		}
	}

	std::vector<itp::VectorDouble> to_return(number_of_points, itp::VectorDouble(std::size(series)));
	for (size_t series_num = 0; series_num < std::size(series); ++series_num)
	{
		for (size_t point_num = 0; point_num < std::size(series[series_num]); ++point_num)
		{
			to_return[point_num][series_num] = series[series_num][point_num];
		}
	}

	return to_return;
}

std::vector<std::vector<double>> Convert(const std::vector<itp::VectorDouble>& res)
{
	if (res.empty())
	{
		return {};
	}

	const size_t kNumberOfSeries = res[0].size();
	std::vector<std::vector<double>> to_return(kNumberOfSeries, std::vector<double>(res.size()));
	for (size_t point_num = 0; point_num < res.size(); ++point_num)
	{
		if (res[point_num].size() != kNumberOfSeries)
		{
			throw itp::DifferentHistoryLengthsError(
				"The number of series for element with number " + std::to_string(point_num)
				+ " differs from the number of the first series");
		}

		for (size_t series_num = 0; series_num < kNumberOfSeries; ++series_num)
		{
			to_return[series_num][point_num] = res[point_num][series_num];
		}
	}

	return to_return;
}

std::map<std::string, std::vector<std::vector<double>>> Convert(
	const std::map<std::string, std::vector<itp::VectorDouble>>& res)
{
	std::map<std::string, std::vector<std::vector<double>>> to_return;
	for (const auto& pair : res)
	{
		to_return[pair.first] = Convert(pair.second);
	}

	return to_return;
}

InformationTheoreticPredictor::InformationTheoreticPredictor()
	: compressors_(itp::MakeStandardCompressorsPool())
{
	// DO NOTHING
}

std::map<std::string, std::vector<itp::Double>> InformationTheoreticPredictor::ForecastReal(
	const std::vector<itp::Double>& time_series,
	const itp::ConcatenatedCompressorNamesVec& compressors_groups,
	size_t horizon,
	size_t difference,
	size_t quanta_count,
	int sparse)
{
	CheckArgs(horizon, difference, sparse);
	CheckQuantaCountRange(quanta_count);

	ForecastingAlgorithmReal<itp::Double> forecasting_algorithm(compressors_);
	forecasting_algorithm.SetQuantaCount(quanta_count);
	return forecasting_algorithm(time_series, compressors_groups, horizon, difference, sparse);
}

std::map<std::string, std::vector<itp::Double>> InformationTheoreticPredictor::ForecastMultialphabet(
	const std::vector<double>& history,
	const itp::ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
	size_t horizon,
	size_t difference,
	size_t max_quanta_count,
	int sparse)
{
	CheckArgs(horizon, difference, sparse);
	CheckQuantaCountRange(max_quanta_count);
	if (!itp::IsPowerOfTwo(max_quanta_count))
	{
		throw std::invalid_argument("Max quants count should be greater a power of two.");
	}

	ForecastingAlgorithmMultialphabet<itp::Double> forecasting_algorithm(compressors_);
	forecasting_algorithm.SetQuantaCount(max_quanta_count);

	std::vector<itp::Double> transformed_history;
	std::copy(begin(history), end(history), std::back_inserter(transformed_history));
	return forecasting_algorithm(transformed_history, concatenated_compressor_groups, horizon, difference, sparse);
}

std::map<std::string, std::vector<std::vector<double>>> InformationTheoreticPredictor::ForecastMultialphabetVec(
	const std::vector<std::vector<double>>& history,
	const itp::ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
	size_t horizon,
	size_t difference,
	size_t max_quanta_count,
	int sparse)
{
	CheckArgs(horizon, difference, sparse);
	CheckQuantaCountRange(max_quanta_count);
	if (!itp::IsPowerOfTwo(max_quanta_count))
	{
		throw std::invalid_argument("Max quants count should be greater a power of two.");
	}

	ForecastingAlgorithmMultialphabet<itp::VectorDouble> forecasting_algorithm{compressors_};
	forecasting_algorithm.SetQuantaCount(max_quanta_count);

	return Convert(
		forecasting_algorithm(Convert(history), concatenated_compressor_groups, horizon, difference, sparse));
}

std::map<std::string, std::vector<itp::Double>> InformationTheoreticPredictor::ForecastDiscrete(
	const std::vector<itp::Symbol>& history,
	const ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
	size_t horizon,
	size_t difference,
	int sparse)
{
	CheckArgs(horizon, difference, sparse);

	ForecastingAlgorithmDiscrete<itp::Double, itp::Symbol> forecasting_algorithm{compressors_};
	return forecasting_algorithm(history, concatenated_compressor_groups, horizon, difference, sparse);
}

std::map<std::string, std::vector<itp::VectorDouble>> InformationTheoreticPredictor::ForecastDiscreteVec(
	const std::vector<itp::VectorSymbol>& history,
	const ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
	size_t horizon,
	size_t difference,
	int sparse)
{
	CheckArgs(horizon, difference, sparse);

	ForecastingAlgorithmDiscrete<itp::VectorDouble, itp::VectorSymbol> forecasting_algorithm{compressors_};
	return forecasting_algorithm(history, concatenated_compressor_groups, horizon, difference, sparse);
}

void InformationTheoreticPredictor::RegisterNonCompressionAlgorithm(
	const std::string& name,
	itp::INonCompressionAlgorithm* non_compression_algorithm)
{
	auto compressor = std::make_unique<itp::NonCompressionAlgorithmAdaptor>(non_compression_algorithm);
	compressors_->RegisterCompressor(name, std::move(compressor));
}

} // namespace itp
