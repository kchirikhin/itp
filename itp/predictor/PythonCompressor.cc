#include "PythonCompressor.h"


namespace itp
{

// todo: autodetect if an interpreter exists
/*class InterpreterSingleton
{
public:
	static void InitInterpreter()
	{
		static pybind11::scoped_interpreter interpreter{};
	}

private:
	InterpreterSingleton() = default;
	~InterpreterSingleton() = default;
};*/

py::module CreateAlgorithmInstance(std::string_view module_name, std::string_view class_name)
{
	return py::module::import(module_name.data()).attr(class_name.data())();
}

PythonCompressor::PythonCompressor(std::string module_name)
{
	// InterpreterSingleton::InitInterpreter();
	RegisterPathToExtensionsDirectory();

	algorithm_instance_ = CreateAlgorithmInstance(module_name, MakeClassName(module_name));
}

void PythonCompressor::RegisterFullTimeSeries(const unsigned char* data, size_t size)
{
	std::vector<unsigned char> vectorized_data(data, data + size);
	algorithm_instance_.attr("register_full_time_series")(vectorized_data);
}

std::pair<Symbol, ConfidenceLevel> PythonCompressor::GiveNextPrediction()
{
	const auto [prediction, confidence_level_num] = py::cast<std::pair<size_t, size_t>>(algorithm_instance_.attr("give_next_prediction")());
	return std::make_pair(prediction, static_cast<ConfidenceLevel>(confidence_level_num));
}

void PythonCompressor::SetTsParams(const Symbol alphabet_min_symbol, const Symbol alphabet_max_symbol)
{
	algorithm_instance_.attr("set_ts_params")(alphabet_min_symbol, alphabet_max_symbol);
}

std::string PythonCompressor::MakeClassName(std::string module_name) const
{
	for (size_t i = 1; i < module_name.size(); ++i)
	{
		if (module_name[i-1] == '_')
		{
			module_name[i] = std::toupper(module_name[i]);
		}
	}
	module_name.erase(std::remove(std::begin(module_name), std::end(module_name), '_'), std::end(module_name));

	if (!module_name.empty())
	{
		module_name.front() = std::toupper(module_name.front());
	}

	return module_name;
}

void PythonCompressor::RegisterPathToExtensionsDirectory()
{
	py::exec(
		"import sys\n"
		"sys.path.insert(0, \"/home/kon/src/itp/itp/extensions/\")\n"
		"del sys\n"
	);
}

} // namespace itp
