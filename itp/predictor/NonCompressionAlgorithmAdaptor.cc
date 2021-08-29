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
	if (data == nullptr)
	{
		throw std::runtime_error{"data is nullptr"};
	}

	if (!alphabet_min_symbol_ || !alphabet_max_symbol_)
	{
		const auto [min, max] = std::minmax_element(data, data + size);
		SetTsParams(*min, *max);
	}

	InternalState state{*alphabet_max_symbol_};
	EvaluateProbability(data, size, &state);

	return static_cast<NonCompressionAlgorithmAdaptor::SizeInBits>(ceil(-log2(state.evaluated_probability)));
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

void NonCompressionAlgorithmAdaptor::SetTsParams(const Symbol alphabet_min_symbol, const Symbol alphabet_max_symbol)
{
	alphabet_min_symbol_ = alphabet_min_symbol;
	alphabet_max_symbol_ = alphabet_max_symbol;

	non_compression_algorithm_->SetTsParams(alphabet_min_symbol, alphabet_max_symbol);
}

void NonCompressionAlgorithmAdaptor::EvaluateProbability(
	const unsigned char* data,
	size_t size,
	InternalState* internal_state) const {
	assert(internal_state != nullptr);

	for (size_t current_pos = 0; current_pos < size; ++current_pos)
	{
		const auto [guessed_symbol, confidence] = non_compression_algorithm_->GiveNextPrediction(data, current_pos);
		const auto observed_symbol = data[current_pos];
		switch (confidence)
		{
			case ConfidenceLevel::kConfident:
			{
				++internal_state->confident_estimations_series_len;
				internal_state->confident_guess_freq[guessed_symbol] =
						internal_state->confident_estimations_series_len;
				const auto total_freq = internal_state->confident_estimations_series_len;
				internal_state->evaluated_probability *= KrichevskyPredictor(
					internal_state->confident_guess_freq[observed_symbol],
					total_freq,
					GetAlphabetRange());
				internal_state->confident_guess_freq[guessed_symbol] = 0;
			}
				break;
			case ConfidenceLevel::kNotConfident:
			{
				internal_state->confident_estimations_series_len = 0;
				const auto total_freq = current_pos;
				internal_state->evaluated_probability *= KrichevskyPredictor(
					internal_state->letters_freq[observed_symbol],
					total_freq,
					GetAlphabetRange());
			}
				break;
			default:
				assert(false);
		}
		internal_state->letters_freq[observed_symbol] += 1;
	}
}

} // namespace itp
