//
// Created by kon on 25.10.2021.
//

#ifndef ITP_CORE_PREDICTOR_H_INCLUDED_
#define ITP_CORE_PREDICTOR_H_INCLUDED_

#include "../../src/PrimitiveDataTypes.h"
#include "INonCompressionAlgorithm.h"

#include <map>

namespace itp
{

class CompressorsFacade;

class InformationTheoreticPredictor
{
public:
	InformationTheoreticPredictor();
	std::map<std::string, std::vector<itp::Double>> ForecastReal(
		const std::vector<itp::Double>& time_series,
		const itp::ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
		size_t horizon,
		size_t difference,
		size_t quanta_count,
		int sparse);

	std::map<std::string, std::vector<itp::Double>> ForecastMultialphabet(
		const std::vector<double>& history,
		const itp::ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
		size_t horizon,
		size_t difference,
		size_t max_quanta_count,
		int sparse);

	std::map<std::string, std::vector<std::vector<double>>> ForecastMultialphabetVec(
		const std::vector<std::vector<double>>& history,
		const itp::ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
		size_t horizon,
		size_t difference,
		size_t max_quanta_count,
		int sparse);

	std::map<std::string, std::vector<itp::Double>> ForecastDiscrete(
		const std::vector<itp::Symbol>& history,
		const ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
		size_t horizon,
		size_t difference,
		int sparse);

	std::map<std::string, std::vector<itp::VectorDouble>> ForecastDiscreteVec(
		const std::vector<itp::VectorSymbol>& history,
		const ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
		size_t horizon,
		size_t difference,
		int sparse);

	void RegisterNonCompressionAlgorithm(
		const std::string& name,
		itp::INonCompressionAlgorithm* non_compression_algorithm);

private:
	std::shared_ptr<CompressorsFacade> compressors_;
};

} // namespace itp

#endif // ITP_CORE_PREDICTOR_H_INCLUDED_
