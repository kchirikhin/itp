#ifndef ITP_NONCOMPRESSIONALGORITHMADAPTOR_H
#define ITP_NONCOMPRESSIONALGORITHMADAPTOR_H

#include "INonCompressionAlgorithm.h"

#include <optional>

namespace itp
{

class NonCompressionAlgorithmAdaptor : public ICompressor
{
public:
	explicit NonCompressionAlgorithmAdaptor(INonCompressionAlgorithmPtr non_compression_algorithm);

	SizeInBits operator()(const unsigned char* data, size_t size, std::vector<unsigned char>* output_buffer) override;
	void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol) override;

private:
	void ResetInternalData();

	auto GetAlphabetRange() const
	{
		static_assert(std::is_unsigned_v<Symbol>, "Symbol must be an unsigned integer");
		assert(alphabet_min_symbol_ && alphabet_max_symbol_);
		return static_cast<size_t>(*alphabet_max_symbol_) - static_cast<size_t>(*alphabet_min_symbol_) + 1u;
	}

	INonCompressionAlgorithmPtr non_compression_algorithm_;

	std::optional<Symbol> alphabet_min_symbol_ = std::nullopt;
	std::optional<Symbol> alphabet_max_symbol_ = std::nullopt;

	HighPrecDouble evaluated_probability_ = 1.0;
	size_t confident_estimations_series_len_ = 0;

	std::vector<size_t> letters_freq_;
	std::vector<size_t> confident_guess_freq_;
};

} // namespace itp

#endif //ITP_NONCOMPRESSIONALGORITHMADAPTOR_H
