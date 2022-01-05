#include "NonCompressionAlgorithmAdaptor.h"

#include <numeric>

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

	return ToCodeLengths(state.evaluated_probability);
}

std::vector<NonCompressionAlgorithmAdaptor::SizeInBits> NonCompressionAlgorithmAdaptor::CompressContinuations(
	const std::vector<Symbol>& historical_values,
	const Continuations& possible_endings)
{
	if (possible_endings.empty())
	{
		return {};
	}

	if (!alphabet_min_symbol_ || !alphabet_max_symbol_)
	{
		const auto [min, max] = std::minmax_element(std::cbegin(historical_values), std::cend(historical_values));
		SetTsParams(*min, *max);
	}

	InternalState history_state(*alphabet_max_symbol_);
	EvaluateProbability(historical_values.data(), historical_values.size(), &history_state);

	std::vector<unsigned char> input_buffer(std::size(historical_values) + std::size(possible_endings.front()));
	std::copy(std::cbegin(historical_values), std::cend(historical_values), std::begin(input_buffer));

	std::vector<unsigned char> output_buffer;
	std::vector<SizeInBits> result(std::size(possible_endings));
	for (size_t i = 0; i < std::size(possible_endings); ++i)
	{
		std::copy(
			possible_endings[i].cbegin(),
			possible_endings[i].cend(),
			std::next(std::begin(input_buffer), std::size(historical_values)));

		auto full_state = history_state;
		EvaluateProbability(input_buffer.data(), std::size(input_buffer), &full_state);
		result[i] = ToCodeLengths(full_state.evaluated_probability);
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

	for (auto* current_pos = &internal_state->current_pos; *current_pos < size; ++(*current_pos))
	{
		const auto [guessed_symbol, confidence] = non_compression_algorithm_->GiveNextPrediction(data, *current_pos);
		const auto observed_symbol = data[*current_pos];
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
				const auto total_freq = *current_pos;
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

NonCompressionAlgorithmAdaptor::SizeInBits NonCompressionAlgorithmAdaptor::ToCodeLengths(
	const HighPrecDouble& probability)
{
	return static_cast<NonCompressionAlgorithmAdaptor::SizeInBits>(ceil(-log2(probability)));
}

} // namespace itp
