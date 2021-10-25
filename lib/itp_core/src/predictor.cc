#include <predictor.h>

#include "compression_prediction.h"
#include "builders.h"

namespace itp
{

namespace
{

void check_args(size_t horizont, size_t difference, int sparse) {
	if (50 < horizont) {
		throw std::invalid_argument("Forecasting horizont is too long (> 50)");
	}

	if (10 < difference) {
		throw std::invalid_argument("Difference order is too big (> 10)");
	}

	if (20 < sparse) {
		throw std::invalid_argument("Sparse value is too big (> 20)");
	}
}

void check_quants_count_range(size_t quants_count) {
	if ((0 == quants_count) || (256 < quants_count)) {
		throw std::invalid_argument("Quants count should be greater than zero and not greater than 256");
	}
}

} // namespace

InformationTheoreticPredictor::InformationTheoreticPredictor()
		: compressors_(itp::MakeStandardCompressorsPool())
{
	// DO NOTHING
}

std::map<std::string, std::vector<itp::Double>> InformationTheoreticPredictor::make_forecast_real(
		const std::vector<itp::Double> &time_series,
		const itp::Names &compressors_groups,
		size_t horizon,
		size_t difference,
		size_t quanta_count, int sparse)
{
	check_args(horizon, difference, sparse);
	check_quants_count_range(quanta_count);

	Forecasting_algorithm_real<itp::Double> make_forecast(compressors_);
	make_forecast.set_quants_count(quanta_count);
	return make_forecast(time_series, compressors_groups, horizon, difference, sparse);
}

std::map<std::string, std::vector<itp::Double>> InformationTheoreticPredictor::make_forecast_multialphabet(
		const std::vector<double> &history,
		const itp::Names &compressors_groups,
		size_t horizon,
		size_t difference,
		size_t max_quanta_count,
		int sparse)
{
	check_args(horizon, difference, sparse);
	check_quants_count_range(max_quanta_count);
	if (!itp::IsPowerOfTwo(max_quanta_count)) {
		throw std::invalid_argument("Max quants count should be greater a power of two.");
	}

	Forecasting_algorithm_multialphabet<itp::Double> make_forecast(compressors_);
	make_forecast.set_quants_count(max_quanta_count);

	std::vector<itp::Double> transformed_history;
	std::copy(begin(history), end(history), std::back_inserter(transformed_history));
	return make_forecast(transformed_history, compressors_groups, horizon, difference, sparse);
}

std::map<std::string, std::vector<std::vector<double>>> InformationTheoreticPredictor::make_forecast_multialphabet_vec(
		const std::vector<std::vector<double>> &history,
		const itp::Names &compressors_groups,
		size_t horizon,
		size_t difference,
		size_t max_quanta_count,
		int sparse) {
	check_args(horizon, difference, sparse);
	check_quants_count_range(max_quanta_count);
	if (!itp::IsPowerOfTwo(max_quanta_count)) {
		throw std::invalid_argument("Max quants count should be greater a power of two.");
	}

	Forecasting_algorithm_multialphabet<itp::VectorDouble> make_forecast{compressors_};
	make_forecast.set_quants_count(max_quanta_count);

	return Convert(make_forecast(Convert(history), compressors_groups, horizon, difference, sparse));
}

std::map<std::string, std::vector<itp::Double>> InformationTheoreticPredictor::make_forecast_discrete(
		const std::vector<itp::Symbol> &history,
		const std::vector<std::string> &compressors_groups,
		size_t horizon,
		size_t difference,
		int sparse) {
	check_args(horizon, difference, sparse);

	Forecasting_algorithm_discrete<itp::Double, itp::Symbol> make_forecast{compressors_};
	auto res = make_forecast(history, compressors_groups, horizon, difference, sparse);
	return res;
}

std::map<std::string, std::vector<itp::VectorDouble>> InformationTheoreticPredictor::make_forecast_discrete_vec(
		const std::vector<itp::VectorSymbol> &history,
		const std::vector<std::string> &compressors_groups,
		size_t horizon,
		size_t difference,
		int sparse) {
	check_args(horizon, difference, sparse);

	Forecasting_algorithm_discrete<itp::VectorDouble, itp::VectorSymbol> make_forecast{compressors_};
	auto res = make_forecast(history, compressors_groups, horizon, difference, sparse);

	return res;
}

void InformationTheoreticPredictor::RegisterNonCompressionAlgorithm(
		const std::string& name,
		itp::INonCompressionAlgorithm* non_compression_algorithm)
{
	auto compressor = std::make_unique<itp::NonCompressionAlgorithmAdaptor>(non_compression_algorithm);
	compressors_->RegisterCompressor(name, std::move(compressor));
}

} // itp
