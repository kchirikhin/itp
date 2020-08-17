#include "NonCompressionAlgorithmAdaptor.h"


namespace itp
{

namespace
{

HighPrecDouble KrichevskyPredictor(const size_t sym_freq, const size_t total_freq, const size_t alphabet_size) {
	return (HighPrecDouble(sym_freq) + 1.0 / 2) / (HighPrecDouble(total_freq) + alphabet_size / 2.0);
}

} // namespace

NonCompressionAlgorithmAdaptor::NonCompressionAlgorithmAdaptor(INonCompressionAlgorithm* non_compression_algorithm)
	: non_compression_algorithm_{non_compression_algorithm}
{
	assert(non_compression_algorithm_ != nullptr);
}

size_t NonCompressionAlgorithmAdaptor::operator()(
	const unsigned char* data,
	const size_t size,
	std::vector<unsigned char>* /*output_buffer*/)
{
	if (data == nullptr)
	{
		throw std::runtime_error{"data is nullptr"};
	}

	if (!alphabet_min_symbol_ || !alphabet_max_symbol_)
	{
		const auto [min, max] = std::minmax_element(data, data + size);
		SetTsParams(*min, *max);
	}

	non_compression_algorithm_->RegisterFullTimeSeries(data, size);
	for (size_t current_pos = 0; current_pos < size; ++current_pos)
	{
		const auto [guessed_symbol, confidence] = non_compression_algorithm_->GiveNextPrediction();
		const auto observed_symbol = data[current_pos];
		switch (confidence)
		{
			case ConfidenceLevel::kConfident:
				{
					++confident_estimations_series_len_;
					confident_guess_freq_[guessed_symbol] = confident_estimations_series_len_;
					const auto total_freq = confident_estimations_series_len_;
					evaluated_probability_ *= KrichevskyPredictor(confident_guess_freq_[observed_symbol], total_freq,
																  GetAlphabetRange());
					confident_guess_freq_[guessed_symbol] = 0;
				}
				break;
			case ConfidenceLevel::kNotConfident:
				{
					confident_estimations_series_len_ = 0;
					const auto total_freq = current_pos;
					evaluated_probability_ *= KrichevskyPredictor(letters_freq_[observed_symbol], total_freq,
																  GetAlphabetRange());
				}
				break;
			default:
				assert(false);
		}
		letters_freq_[observed_symbol] += 1;
	}

	return ceil(-log2(evaluated_probability_));
}

void NonCompressionAlgorithmAdaptor::SetTsParams(const Symbol alphabet_min_symbol, const Symbol alphabet_max_symbol)
{
	alphabet_min_symbol_ = alphabet_min_symbol;
	alphabet_max_symbol_ = alphabet_max_symbol;

	non_compression_algorithm_->SetTsParams(alphabet_min_symbol, alphabet_max_symbol);
	letters_freq_.resize(alphabet_max_symbol + 1);
	confident_guess_freq_.resize(alphabet_max_symbol + 1);
}

} // namespace itp
