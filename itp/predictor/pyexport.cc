/**
 * @file   pyexport.cpp
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Fri Jun  8 21:46:27 2018
 *
 * @brief  Implementation of Python binding.
 *
 *
 */
#include "builders.h"
#include "selector.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(predictor, m) {
    m.doc() = "Information-theoretic predictor for time series with real or discrete values.";

    py::class_<itp::Share>(m, "Share").def(py::init<double>());

    m.def("make_forecast_real", &make_forecast_real,
          "Forecast real-valued time series with single partition on discretization",
          py::arg("time_series"), py::arg("groups"), py::arg("h") = 1, py::arg("difference") = 0,
          py::arg("quants_count") = 8, py::arg("sparse") = -1);
    m.def("make_forecast_multialphabet", &make_forecast_multialphabet,
          "Make forecast with multiple partitions for real-valued time series",
          py::arg("time_series"), py::arg("groups"), py::arg("h") = 1, py::arg("difference") = 0,
          py::arg("max_quants_count") = 8, py::arg("sparse") = -1);
    m.def("make_forecast_real", &make_forecast_real, "Make forecast with single for time series",
          py::arg("time_series"), py::arg("groups"), py::arg("h") = 1, py::arg("difference") = 0,
          py::arg("max_quants_count") = 8, py::arg("sparse") = -1);
    m.def("make_forecast_discrete", &make_forecast_discrete, "Make forecast for time series",
          py::arg("time_series"), py::arg("groups"), py::arg("h") = 1, py::arg("difference") = 0,
          py::arg("sparse") = -1);
    m.def("make_forecast_multialphabet_vec", &make_forecast_multialphabet_vec,
          "Make forecast with multiple partitions for real-valued vector time series",
          py::arg("time_series"), py::arg("groups"), py::arg("h") = 1, py::arg("difference") = 0,
          py::arg("max_quants_count") = 8, py::arg("sparse") = -1);
    m.def("select_best_compressors_multialphabet", &itp::SelectBestCompressors<double>, "Select specified amount of best compressors by compressing a part of the input sequence",
            py::arg("time_series"), py::arg("compressors"), py::arg("difference"), py::arg("quanta_count"),
            py::arg("share"), py::arg("target_number"));
	m.def("select_best_compressors_discrete", &itp::SelectBestCompressors<itp::Symbol>, "Select specified amount of best compressors by compressing a part of the input sequence",
		  py::arg("time_series"), py::arg("compressors"), py::arg("difference"), py::arg("quanta_count"),
		  py::arg("share"), py::arg("target_number"));
}
