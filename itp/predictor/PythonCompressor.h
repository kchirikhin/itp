#ifndef ITP_PYTHONCOMPRESSOR_H
#define ITP_PYTHONCOMPRESSOR_H

#include "INonCompressionAlgorithm.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace itp
{

class PythonCompressor : public INonCompressionAlgorithm
{
public:
	explicit PythonCompressor(std::string module_name);
	~PythonCompressor() = default;

	void RegisterFullTimeSeries(const unsigned char* data, size_t size) override;
	[[nodiscard]] std::pair<Symbol, ConfidenceLevel> GiveNextPrediction() override;
	void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol) override;

private:
	std::string MakeClassName(std::string module_name) const;
	void RegisterPathToExtensionsDirectory();

	py::module algorithm_instance_;
};

} // namespace itp

#endif //ITP_PYTHONCOMPRESSOR_H
