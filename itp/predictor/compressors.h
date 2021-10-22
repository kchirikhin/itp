/**
 * Implementations of an access point to external data compression libraries.
 */

#ifndef ITP_COMPRESSORS_H_INCLUDED_
#define ITP_COMPRESSORS_H_INCLUDED_

#include "dtypes.h"
#include "ICompressor.h"
#include "PythonCompressor.h"

#include <zstd.h>
#include <bzlib.h>
#include <zlib.h>
#include <rp.h>
#include <lcacomp.h>
#include <ppmd.h>
#include <sdfa.h>
#include <libzpaq.h>

#include <iostream>
#include <memory>
#include <cmath>
#include <stdexcept>
#include <unordered_map>

namespace itp
{

/**
 * Converts size in bytes to size in bits.
 *
 * \param n_bytes Size in bytes.
 *
 * \return Size in bits.
 */
constexpr size_t BytesToBits(size_t n_bytes)
{
	return 8 * n_bytes;
}

/**
 * The only excpetion type can be thrown in this module.
 */
class CompressorsError : public std::runtime_error
{
public:
	explicit CompressorsError(const std::string& what_arg)
			: runtime_error{what_arg}
	{
		// DO NOTHING
	}
};

class CompressorBase : public ICompressor
{
public:
	void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol) override
	{
		// DO NOTHING
	};

protected:
	/**
	 * Allocates memory for output data if it's not enough.
	 * @param desired_size Size to set.
	 * @param output_buffer Buffer to resize.
	 */
	static void FitBuffer(size_t desired_size, std::vector<unsigned char>* output_buffer)
	{
		if (output_buffer->size() < desired_size)
		{
			output_buffer->resize(desired_size);
		}
	}
};

class ZstdCompressor : public CompressorBase
{
public:
	ZstdCompressor();
	~ZstdCompressor() override;

	SizeInBits operator()(
		const unsigned char* data,
		size_t size,
		std::vector<unsigned char>* output_buffer) override;

private:
	ZSTD_CCtx* context_ = nullptr;
};

class ZlibCompressor : public CompressorBase
{
public:
	SizeInBits operator()(
		const unsigned char* data,
		size_t size,
		std::vector<unsigned char>* output_buffer) override;
};

class PpmCompressor : public CompressorBase
{
public:
	SizeInBits operator()(
		const unsigned char* data,
		size_t size,
		std::vector<unsigned char>* output_buffer) override;
};

class RpCompressor : public CompressorBase
{
public:
	SizeInBits operator()(
		const unsigned char* data,
		size_t size,
		std::vector<unsigned char>* output_buffer) override;
};

class Bzip2Compressor : public CompressorBase
{
public:
	SizeInBits operator()(
		const unsigned char* data,
		size_t size,
		std::vector<unsigned char>* output_buffer) override;
};

class LcaCompressor : public CompressorBase
{
public:
	SizeInBits operator()(
		const unsigned char* data,
		size_t size,
		std::vector<unsigned char>* output_buffer) override;
};

class ZpaqCompressor : public CompressorBase
{
public:
	SizeInBits operator()(
		const unsigned char* data,
		size_t size,
		std::vector<unsigned char>* output_buffer) override;
};

class AutomatonCompressor : public CompressorBase
{
public:
	AutomatonCompressor();

	SizeInBits operator()(
		const unsigned char* data,
		size_t size,
		std::vector<unsigned char>* output_buffer) override;

	void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol) override;

private:
	PredictionAutomationPtr automation;
};

/**
 * Defines an integer alphabet by specifying its min and max symbols.
 */
struct AlphabetDescription
{
	Symbol min_symbol;
	Symbol max_symbol;
};

/**
 * An iterface to access all integrated data compression algorithms.
 */
class CompressorsFacade
{
public:
	virtual ~CompressorsFacade() = default;

	/**
	 * Returns code length, obtained by compression of data by the specified compressor.
	 * @param compressor_name The name of data compression algorithm to use.
	 * @param data Buffer with data to compress.
	 * @param size Size of the data in the buffer.
	 * @return The obtained code length in bits.
	 */
	virtual ICompressor::SizeInBits Compress(
		const std::string& compressor_name,
		const unsigned char* data,
		size_t size) const = 0;

	/**
	 * Some compressors need to know the size of the alphabet. This method allows to specify it before compressing
	 * a series.
	 * @param alphabet_description Minimal and maximal letters of the integer alphabet.
	 */
	virtual void SetAlphabetDescription(AlphabetDescription alphabet_description) = 0;
};
using CompressorsFacadeUPtr = std::unique_ptr<CompressorsFacade>;

/**
 * Implementation of CompressorsFacade, which avoids unnecessary allocations of memory to improve efficiency.
 */
class CompressorsPool : public CompressorsFacade
{
public:
	void RegisterCompressor(std::string name, std::unique_ptr<ICompressor> compressor);

	ICompressor::SizeInBits Compress(
		const std::string& compressor_name,
		const unsigned char* data,
		size_t size) const override;

	void SetAlphabetDescription(AlphabetDescription alphabet_description) override;

private:
	std::unordered_map<std::string, std::unique_ptr<ICompressor>> compressor_instances_;
	mutable std::vector<unsigned char> output_buffer_;
};

CompressorsFacadeUPtr MakeStandardCompressorsPool();

} // of namespace itp

#endif // ITP_COMPRESSORS_H_INCLUDED_
