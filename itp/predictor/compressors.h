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
	virtual void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol)
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

	size_t operator()(const unsigned char* data, size_t size,
					  std::vector<unsigned char>* output_buffer) override;

private:
	ZSTD_CCtx* context_ = nullptr;
};

class ZlibCompressor : public CompressorBase
{
public:
	size_t operator()(const unsigned char* data, size_t size,
					  std::vector<unsigned char>* output_buffer) override;
};

class PpmCompressor : public CompressorBase
{
public:
	size_t operator()(const unsigned char* data, size_t size,
					  std::vector<unsigned char>* output_buffer) override;
};

class RpCompressor : public CompressorBase
{
public:
	size_t operator()(const unsigned char* data, size_t size,
					  std::vector<unsigned char>* output_buffer) override;
};

class Bzip2Compressor : public CompressorBase
{
public:
	size_t operator()(const unsigned char* data, size_t size,
					  std::vector<unsigned char>* output_buffer) override;
};

class LcaCompressor : public CompressorBase
{
public:
	size_t operator()(const unsigned char* data, size_t size,
					  std::vector<unsigned char>* output_buffer) override;
};

class ZpaqCompressor : public CompressorBase
{
public:
	size_t operator()(const unsigned char* data, size_t size,
					  std::vector<unsigned char>* output_buffer) override;
};

class AutomatonCompressor : public CompressorBase
{
public:
	AutomatonCompressor();

	size_t operator()(const unsigned char* data, size_t size,
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
	 * @return The obtain code length in bytes.
	 */
	virtual size_t Compress(const std::string& compressor_name, const unsigned char* data, size_t size) const = 0;

	/**
	 * Some compressors need to know the size of the alphabel. This method allows to specify it before compressing
	 * a series.
	 * @param alphabet_description Minimal and maximal letters of the integer alphabet.
	 */
	virtual void ResetAlphabetDescription(AlphabetDescription alphabet_description) = 0;
};

/**
 * Implementation of CompressorsFacade, which avoids unnecessary allocations of memory to improve efficiency.
 */
class CompressorsPool : public CompressorsFacade
{
public:
	explicit CompressorsPool(AlphabetDescription alphabet_description);

	void RegisterCompressor(std::string name, std::unique_ptr<ICompressor> compressor);

	size_t Compress(const std::string& compressor_name, const unsigned char* data, size_t size) const override;

	void ResetAlphabetDescription(AlphabetDescription alphabet_description) override;

private:
	AlphabetDescription alphabet_description_;

	std::unordered_map<std::string, std::unique_ptr<ICompressor>> compressor_instances_;
	mutable std::vector<unsigned char> output_buffer_;
};

std::unique_ptr<CompressorsFacade> MakeStandardCompressorsPool(AlphabetDescription alphabet_description);

} // of namespace itp

#endif // ITP_COMPRESSORS_H_INCLUDED_
