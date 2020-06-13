#include "compressors.h"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace itp
{

void Compressor::SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol)
{
	// DO NOTHING
}

ZstdCompressor::ZstdCompressor()
{
	if (context_ = ZSTD_createCCtx(); !context_)
	{
		throw CompressorsError{"cannot init zstd compressor"};
	}
}

ZstdCompressor::~ZstdCompressor()
{
	assert(context_);
	ZSTD_freeCCtx(context_);
}

size_t ZstdCompressor::operator()(const unsigned char* data, size_t size, std::vector<unsigned char>* output_buffer)
{
	assert(context_);
	assert(data);
	assert(output_buffer);

	size_t dst_capacity = ZSTD_compressBound(size);
	FitBuffer(dst_capacity, output_buffer);

	return ZSTD_compressCCtx(context_, output_buffer->data(), output_buffer->size(), data, size, ZSTD_maxCLevel());
}

size_t ZlibCompressor::operator()(const unsigned char* data, size_t size, std::vector<unsigned char>* output_buffer)
{
	size_t dst_capacity = compressBound(size * sizeof(Symbol));
	FitBuffer(dst_capacity, output_buffer);
	if (compress2(output_buffer->data(), &dst_capacity,
				  data, size, Z_BEST_COMPRESSION) != Z_OK)
	{
		throw CompressorsError{"zlib: an error is occured"};
	}

	return dst_capacity;
}

size_t PpmCompressor::operator()(const unsigned char* data, size_t size, std::vector<unsigned char>* output_buffer)
{
	size_t dst_capacity = Ppmd::compress_bound(size);
	FitBuffer(dst_capacity, output_buffer);

	return Ppmd::ppmd_compress(output_buffer->data(), output_buffer->size(), data, size);
}

size_t RpCompressor::operator()(const unsigned char* data, size_t size, std::vector<unsigned char>*)
{
	return Rp::rp_compress(data, size);
}

size_t Bzip2Compressor::operator()(const unsigned char* data, size_t size, std::vector<unsigned char>* output_buffer)
{
	assert(output_buffer != nullptr);

	// according to documentation, such capacity guaranties that the compressed data will fit in the buffer
	std::unique_ptr<char[]> src(new char[size]);
	std::copy(data, data + size, src.get());
	uint dst_capacity = static_cast<uint>(size * sizeof(Symbol) + ceil(size * sizeof(Symbol) * 0.01) + 600);
	FitBuffer(dst_capacity, output_buffer);
	if (BZ2_bzBuffToBuffCompress(reinterpret_cast<char*>(output_buffer->data()), &dst_capacity,
								 src.get(), static_cast<uint>(size), 9, 0, 30) != BZ_OK)
	{
		throw CompressorsError{"bzip2: an error occured"};
	}

	return dst_capacity;
}

size_t LcaCompressor::operator()(const unsigned char* data, size_t size, std::vector<unsigned char>*)
{
	return Lcacomp::lcacomp_compress(data, size);
}

namespace
{

class ZpaqReader : public libzpaq::Reader
{
public:
	ZpaqReader(const unsigned char* const data, const size_t size)
			: data_{data}, size_{size}
	{
		// DO NOTHING
	}

	int get() override
	{
		if (curr_pos_ == size_)
		{
			return EOF;
		}

		return data_[curr_pos_++];
	}

	int read(char* buf, int n) override
	{
		if (curr_pos_ == size_)
		{
			return EOF;
		}

		assert(0 <= n);
		const auto bytes_to_read = std::min(static_cast<size_t>(n), size_ - curr_pos_);
		std::copy(data_ + curr_pos_, data_ + curr_pos_ + bytes_to_read, buf);
		curr_pos_ += bytes_to_read;

		return static_cast<int>(bytes_to_read);
	}

private:
	const unsigned char* const data_;
	const size_t size_;
	size_t curr_pos_ = 0;
};

class ZpaqWriter : public libzpaq::Writer
{
public:
	void put(int) override
	{
		++bytes_read_;
	}

	void write(const char*, int n) override
	{
		assert(0 <= n);
		bytes_read_ += static_cast<size_t>(n);
	}

	[[nodiscard]] size_t GetCompressedSize() const
	{
		return bytes_read_;
	}

private:
	size_t bytes_read_ = 0;
};

} // of namespace

} // of namespace itp

void libzpaq::error(const char* msg)
{
	throw itp::CompressorsError{msg};
}

namespace itp
{

size_t ZpaqCompressor::operator()(const unsigned char* data, size_t size, std::vector<unsigned char>*)
{
	const char* kMaxCompressionLevel = "5";
	ZpaqReader reader{data, size};
	ZpaqWriter writer;
	libzpaq::compress(&reader, &writer, kMaxCompressionLevel);

	return writer.GetCompressedSize();
}

AutomatonCompressor::AutomatonCompressor()
		: automation{new Sensing_DFA{0, 255}}
{
	// DO NOTHING
}

size_t AutomatonCompressor::operator()(const unsigned char* data, size_t size,
									   std::vector<unsigned char> *)
{
	auto probability = automation->EvalProbability(PlainTimeSeries<Symbol>(data, data + size));
	auto res = ceil(-log2(automation->EvalProbability(PlainTimeSeries<Symbol>(data, data + size))));

	if (std::numeric_limits<size_t>::max() < res)
	{
		return std::numeric_limits<size_t>::max();
	}
	else
	{
		return static_cast<size_t>(res);
	}
}

void AutomatonCompressor::SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol)
{
	  automation->SetMinSymbol(alphabet_min_symbol);
	  automation->SetMaxSymbol(alphabet_max_symbol);
}

CompressorsPool::CompressorsPool(AlphabetDescription alphabet_description)
	: alphabet_description_{alphabet_description}
{
	// DO NOTHING
}

void CompressorsPool::RegisterCompressor(std::string name, std::unique_ptr<Compressor> compressor)
{
	if (name.empty())
	{
		throw CompressorsError{"RegisterCompressor: name is empty"};
	}

	if (!compressor)
	{
		throw CompressorsError{"RegisterCompressor: compressor is nullptr"};
	}

	if (auto [compressor_iter, success] = compressor_instances_.emplace(std::move(name), std::move(compressor)); success)
	{
		compressor_iter->second->SetTsParams(alphabet_description_.min_symbol, alphabet_description_.max_symbol);
	}
	else
	{
		throw CompressorsError{"RegisterCompressor: trying to register already registered compressor"};
	}
}

size_t CompressorsPool::Compress(const std::string& compressor_name, const unsigned char* data, size_t size) const
{
	try
	{
		return compressor_instances_.at(compressor_name)->operator()(data, size, &output_buffer_);
	}
	catch (const std::out_of_range& e)
	{
		throw CompressorsError{"Incorrect compressor name " + compressor_name};
	}
}

void CompressorsPool::ResetAlphabetDescription(AlphabetDescription alphabet_description)
{
	for (auto& [name, compressor] : compressor_instances_)
	{
		compressor->SetTsParams(alphabet_description.min_symbol, alphabet_description.max_symbol);
	}
}

std::unique_ptr<CompressorsFacade> MakeStandardCompressorsPool(AlphabetDescription alphabet_description)
{
	auto to_return = std::make_unique<CompressorsPool>(alphabet_description);

	to_return->RegisterCompressor("lcacomp", std::make_unique<LcaCompressor>());
	to_return->RegisterCompressor("rp", std::make_unique<RpCompressor>());
	to_return->RegisterCompressor("zstd", std::make_unique<ZstdCompressor>());
	to_return->RegisterCompressor("bzip2", std::make_unique<Bzip2Compressor>());
	to_return->RegisterCompressor("zlib", std::make_unique<ZlibCompressor>());
	to_return->RegisterCompressor("ppmd", std::make_unique<PpmCompressor>());
	to_return->RegisterCompressor("automation", std::make_unique<AutomatonCompressor>());
	to_return->RegisterCompressor("zpaq", std::make_unique<ZpaqCompressor>());

	return to_return;
}

} // of namespace itp
