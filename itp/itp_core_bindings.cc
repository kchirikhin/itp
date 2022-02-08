/**
 * @file   pyexport.cpp
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Fri Jun  8 21:46:27 2018
 *
 * @brief  Implementation of Python binding.
 */
#include <itp_core/INonCompressionAlgorithm.h>
#include <itp_core/Predictor.h>
#include <itp_core/Selector.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

class PyINonCompressionAlgorithm : public itp::INonCompressionAlgorithm
{
public:
	virtual Guess PyGiveNextPrediction(const pybind11::bytes& bytes) = 0;

	Guess GiveNextPrediction(const unsigned char* data, size_t size) final
	{
		return PyGiveNextPrediction({reinterpret_cast<const char*>(data), size});
	}
};

class INonCompressionAlgorithm_ : public itp::INonCompressionAlgorithm
{
public:
	using INonCompressionAlgorithm::INonCompressionAlgorithm;

	Guess GiveNextPrediction(const unsigned char* data, size_t size) override
	{
		PYBIND11_OVERRIDE_PURE(Guess, INonCompressionAlgorithm, GiveNextPrediction, data, size);
	}

	void SetTsParams(itp::Symbol alphabet_min_symbol, itp::Symbol alphabet_max_symbol) override
	{
		PYBIND11_OVERRIDE_PURE(void, INonCompressionAlgorithm, SetTsParams, alphabet_min_symbol, alphabet_max_symbol);
	}
};

class PyINonCompressionAlgorithm_ : public PyINonCompressionAlgorithm
{
public:
	using PyINonCompressionAlgorithm::PyINonCompressionAlgorithm;

	Guess PyGiveNextPrediction(const pybind11::bytes& bytes) override
	{
		PYBIND11_OVERRIDE_PURE(Guess, PyINonCompressionAlgorithm, PyGiveNextPrediction, bytes);
	}

	void SetTsParams(itp::Symbol alphabet_min_symbol, itp::Symbol alphabet_max_symbol) override
	{
		PYBIND11_OVERRIDE_PURE(void, PyINonCompressionAlgorithm, SetTsParams, alphabet_min_symbol, alphabet_max_symbol);
	}
};

PYBIND11_MODULE(itp_core_bindings, m)
{
	m.doc() = "Information-theoretic predictor for time series with real or discrete values.";

	py::class_<itp::Share>(m, "Share").def(py::init<double>());

	py::enum_<itp::ConfidenceLevel>(m, "ConfidenceLevel")
		.value("CONFIDENT", itp::ConfidenceLevel::Confident)
		.value("NOT_CONFIDENT", itp::ConfidenceLevel::NotConfident);

	py::class_<itp::INonCompressionAlgorithm, INonCompressionAlgorithm_>(m, "INonCompressionAlgorithm")
		.def(py::init<>())
		.def("GiveNextPrediction", &itp::INonCompressionAlgorithm::GiveNextPrediction)
		.def("SetTsParams", &itp::INonCompressionAlgorithm::SetTsParams);
	py::class_<PyINonCompressionAlgorithm, itp::INonCompressionAlgorithm, PyINonCompressionAlgorithm_>(
		m,
		"NonCompressionAlgorithm")
		.def(py::init<>())
		.def("PyGiveNextPrediction", &PyINonCompressionAlgorithm::PyGiveNextPrediction)
		.def("SetTsParams", &PyINonCompressionAlgorithm::SetTsParams);

	py::class_<itp::InformationTheoreticPredictor>(m, "InformationTheoreticPredictor")
		.def(py::init<>())
		.def(
			"forecast_real",
			&itp::InformationTheoreticPredictor::ForecastReal,
			"Forecast real-valued time series with single partition on discretization",
			py::arg("time_series"),
			py::arg("groups"),
			py::arg("h") = 1,
			py::arg("difference") = 0,
			py::arg("quants_count") = 8,
			py::arg("sparse") = -1)
		.def(
			"forecast_multialphabet",
			&itp::InformationTheoreticPredictor::ForecastMultialphabet,
			"Make forecast with multiple partitions for real-valued time series",
			py::arg("time_series"),
			py::arg("groups"),
			py::arg("h") = 1,
			py::arg("difference") = 0,
			py::arg("max_quants_count") = 8,
			py::arg("sparse") = -1)
		.def(
			"forecast_multialphabet_vec",
			&itp::InformationTheoreticPredictor::ForecastMultialphabetVec,
			"Make forecast with multiple partitions for real-valued vector time series",
			py::arg("time_series"),
			py::arg("groups"),
			py::arg("h") = 1,
			py::arg("difference") = 0,
			py::arg("max_quants_count") = 8,
			py::arg("sparse") = -1)
		.def(
			"forecast_discrete",
			&itp::InformationTheoreticPredictor::ForecastDiscrete,
			"Make forecast without quantization for discrete time series",
			py::arg("time_series"),
			py::arg("groups"),
			py::arg("h") = 1,
			py::arg("difference") = 0,
			py::arg("sparse") = -1)
		.def(
			"forecast_discrete_vec",
			&itp::InformationTheoreticPredictor::ForecastDiscreteVec,
			"Make forecast without quantization for discrete vector time series",
			py::arg("time_series"),
			py::arg("groups"),
			py::arg("h") = 1,
			py::arg("difference") = 0,
			py::arg("sparse") = -1)
		.def(
			"register_non_compression_algorithm",
			&itp::InformationTheoreticPredictor::RegisterNonCompressionAlgorithm,
			"Adds an algorithm written in Python to the set of available algorithms",
			py::arg("name"),
			py::arg("algorithm"));

	m.def(
		"select_best_compressors_multialphabet",
		&itp::SelectBestCompressors<double>,
		"Select specified amount of best compressors by compressing a part of the input sequence",
		py::arg("time_series"),
		py::arg("compressors"),
		py::arg("difference"),
		py::arg("quanta_count"),
		py::arg("share"),
		py::arg("target_number"));
	m.def(
		"select_best_compressors_discrete",
		&itp::SelectBestCompressors<itp::Symbol>,
		"Select specified amount of best compressors by compressing a part of the input sequence",
		py::arg("time_series"),
		py::arg("compressors"),
		py::arg("difference"),
		py::arg("quanta_count"),
		py::arg("share"),
		py::arg("target_number"));
}
