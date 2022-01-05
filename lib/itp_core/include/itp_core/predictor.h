//
// Created by kon on 25.10.2021.
//

#ifndef ITP_CORE_PREDICTOR_H_INCLUDED_
#define ITP_CORE_PREDICTOR_H_INCLUDED_

#include "../../src/primitive_dtypes.h"
#include "INonCompressionAlgorithm.h"

#include <map>

namespace itp
{

class CompressorsFacade;

class InformationTheoreticPredictor
{
public:
	InformationTheoreticPredictor();
	std::map<std::string, std::vector<itp::Double>> make_forecast_real(
			const std::vector<itp::Double> &time_series,
			const itp::Names &compressors_groups,
			size_t horizon,
			size_t difference,
			size_t quanta_count,
			int sparse);

	std::map<std::string, std::vector<itp::Double>> make_forecast_multialphabet(
			const std::vector<double> &history,
			const itp::Names &compressors_groups,
			size_t horizon,
			size_t difference,
			size_t max_quanta_count,
			int sparse);

	std::map<std::string, std::vector<std::vector<double>>> make_forecast_multialphabet_vec(
			const std::vector<std::vector<double>> &history,
			const itp::Names &compressors_groups,
			size_t horizon,
			size_t difference,
			size_t max_quanta_count,
			int sparse);

	std::map<std::string, std::vector<itp::Double>> make_forecast_discrete(
			const std::vector<itp::Symbol> &history,
			const std::vector<std::string> &compressors_groups,
			size_t horizon,
			size_t difference,
			int sparse);

	std::map<std::string, std::vector<itp::VectorDouble>> make_forecast_discrete_vec(
			const std::vector<itp::VectorSymbol> &history,
			const std::vector<std::string> &compressors_groups,
			size_t horizon,
			size_t difference,
			int sparse);

	void RegisterNonCompressionAlgorithm(
			const std::string& name,
			itp::INonCompressionAlgorithm* non_compression_algorithm);

private:
	std::shared_ptr<CompressorsFacade> compressors_;
};

} // itp

#endif // ITP_CORE_PREDICTOR_H_INCLUDED_
