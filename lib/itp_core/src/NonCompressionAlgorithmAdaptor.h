#ifndef ITP_NON_COMPRESSION_ALGORITHM_ADAPTOR_H
#define ITP_NON_COMPRESSION_ALGORITHM_ADAPTOR_H

#include "ICompressor.h"
#include "INonCompressionAlgorithm.h"

#include <optional>

namespace itp
{

class NonCompressionAlgorithmAdaptor : public ICompressor
{
public:
	explicit NonCompressionAlgorithmAdaptor(INonCompressionAlgorithm* non_compression_algorithm);

	SizeInBits Compress(const unsigned char* data, size_t size, std::vector<unsigned char>* output_buffer) override;

	std::vector<SizeInBits> CompressContinuations(
		const std::vector<Symbol>& historical_values,
		const Continuations& possible_endings) override;

	void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol) override;

private:
	struct InternalState
	{
		explicit InternalState(Symbol alphabet_max_symbol)
			: letters_freq(alphabet_max_symbol + 1)
			, confident_guess_freq(alphabet_max_symbol + 1)
		{
			// DO NOTHING
		}

		size_t confident_estimations_series_len = 0;
		HighPrecDouble evaluated_probability = 1.0;
		size_t current_pos = 0;

		std::vector<size_t> letters_freq;
		std::vector<size_t> confident_guess_freq;
	};

	[[nodiscard]] auto GetAlphabetRange() const
	{
		static_assert(std::is_unsigned_v<Symbol>, "Symbol must be an unsigned integer");
		assert(alphabet_min_symbol_ && alphabet_max_symbol_);
		return static_cast<size_t>(*alphabet_max_symbol_) - static_cast<size_t>(*alphabet_min_symbol_) + 1u;
	}

	void EvaluateProbability(const unsigned char* data, size_t size, InternalState* internal_state) const;

	static SizeInBits ToCodeLengths(const HighPrecDouble& probability);

	INonCompressionAlgorithm* non_compression_algorithm_;

	std::optional<Symbol> alphabet_min_symbol_ = std::nullopt;
	std::optional<Symbol> alphabet_max_symbol_ = std::nullopt;
};

} // namespace itp

#endif // ITP_NON_COMPRESSION_ALGORITHM_ADAPTOR_H
