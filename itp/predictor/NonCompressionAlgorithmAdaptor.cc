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
	: non_compression_algorithm_{std::move(non_compression_algorithm)}
{
	assert(non_compression_algorithm_ != nullptr);
}

NonCompressionAlgorithmAdaptor::SizeInBits NonCompressionAlgorithmAdaptor::Compress(
	const unsigned char* data,
	const size_t size,
	std::vector<unsigned char>* /*output_buffer*/)
{
	ResetInternalData();

	if (data == nullptr)
	{
		throw std::runtime_error{"data is nullptr"};
	}

	if (!alphabet_min_symbol_ || !alphabet_max_symbol_)
	{
		const auto [min, max] = std::minmax_element(data, data + size);
		SetTsParams(*min, *max);
	}

	for (size_t current_pos = 0; current_pos < size; ++current_pos)
	{
		const auto [guessed_symbol, confidence] = non_compression_algorithm_->GiveNextPrediction(data, current_pos);
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

	return static_cast<NonCompressionAlgorithmAdaptor::SizeInBits>(ceil(-log2(evaluated_probability_)));
}

std::vector<NonCompressionAlgorithmAdaptor::SizeInBits> NonCompressionAlgorithmAdaptor::CompressEndings(
	const std::vector<Symbol>& historical_values,
	const Trajectories& possible_endings)
{
	if (possible_endings.empty())
	{
		return {};
	}

	const auto full_series_length = std::size(historical_values) + std::size(possible_endings.front());
	auto buffer = std::make_unique<Symbol[]>(full_series_length);
	std::copy(std::cbegin(historical_values), std::cend(historical_values), buffer.get());

	std::vector<unsigned char> output_buffer;
	std::vector<SizeInBits> result(std::size(possible_endings));
	for (size_t i = 0; i < std::size(possible_endings); ++i)
	{
		std::copy(possible_endings[i].cbegin(), possible_endings[i].cend(), buffer.get() + std::size(historical_values));
		result[i] = Compress(buffer.get(), full_series_length, &output_buffer);
	}

	return result;
}

void NonCompressionAlgorithmAdaptor::ResetInternalData()
{
	evaluated_probability_ = 1.0;
	confident_estimations_series_len_ = 0;

	std::fill(std::begin(letters_freq_), std::end(letters_freq_), 0);
	std::fill(std::begin(confident_guess_freq_), std::end(confident_guess_freq_), 0);
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
