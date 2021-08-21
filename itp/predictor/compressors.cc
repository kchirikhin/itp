#include "compressors.h"

#include "NonCompressionAlgorithmAdaptor.h"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace itp
{

std::vector<CompressorBase::SizeInBits> CompressorBase::CompressEndings(
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

ZstdCompressor::SizeInBits ZstdCompressor::Compress(
	const unsigned char* data,
	size_t size,
	std::vector<unsigned char>* output_buffer)
{
	assert(context_);
	assert(data);
	assert(output_buffer);

	size_t dst_capacity = ZSTD_compressBound(size);
	FitBuffer(dst_capacity, output_buffer);

	return BytesToBits(ZSTD_compressCCtx(
		context_,
		output_buffer->data(),
		output_buffer->size(),
		data,
		size,
		ZSTD_maxCLevel()));
}

ZlibCompressor::SizeInBits ZlibCompressor::Compress(
	const unsigned char* data,
	size_t size,
	std::vector<unsigned char>* output_buffer)
{
	size_t dst_capacity = compressBound(size * sizeof(Symbol));
	FitBuffer(dst_capacity, output_buffer);
	if (compress2(output_buffer->data(), &dst_capacity,
				  data, size, Z_BEST_COMPRESSION) != Z_OK)
	{
		throw CompressorsError{"zlib: an error is occured"};
	}

	return BytesToBits(dst_capacity);
}

PpmCompressor::SizeInBits PpmCompressor::Compress(
	const unsigned char* data,
	size_t size,
	std::vector<unsigned char>* output_buffer)
{
	size_t dst_capacity = Ppmd::compress_bound(size);
	FitBuffer(dst_capacity, output_buffer);

	return BytesToBits(Ppmd::ppmd_compress(output_buffer->data(), output_buffer->size(), data, size));
}

RpCompressor::SizeInBits RpCompressor::Compress(
	const unsigned char* data,
	size_t size,
	std::vector<unsigned char>*)
{
	return BytesToBits(Rp::rp_compress(data, size));
}

Bzip2Compressor::SizeInBits Bzip2Compressor::Compress(
	const unsigned char* data,
	size_t size,
	std::vector<unsigned char>* output_buffer)
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

	return BytesToBits(dst_capacity);
}

LcaCompressor::SizeInBits LcaCompressor::Compress(
	const unsigned char* data,
	size_t size,
	std::vector<unsigned char>*)
{
	return BytesToBits(Lcacomp::lcacomp_compress(data, size));
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

ZpaqCompressor::SizeInBits ZpaqCompressor::Compress(
	const unsigned char* data,
	size_t size,
	std::vector<unsigned char>*)
{
	const char* kMaxCompressionLevel = "5";
	ZpaqReader reader{data, size};
	ZpaqWriter writer;
	libzpaq::compress(&reader, &writer, kMaxCompressionLevel);

	return BytesToBits(writer.GetCompressedSize());
}

AutomatonCompressor::AutomatonCompressor()
		: automation{new Sensing_DFA{0, 255}}
{
	// DO NOTHING
}

AutomatonCompressor::SizeInBits AutomatonCompressor::Compress(
	const unsigned char* data,
	size_t size,
	std::vector<unsigned char>*)
{
	auto probability = automation->EvalProbability(PlainTimeSeries<Symbol>(data, data + size));
	const auto code_length = ceil(-log2(probability));

	if (HighPrecDouble(std::numeric_limits<AutomatonCompressor::SizeInBits>::max()) < code_length)
	{
		return std::numeric_limits<AutomatonCompressor::SizeInBits>::max();
	}

	return static_cast<AutomatonCompressor::SizeInBits>(code_length);
}

void AutomatonCompressor::SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol)
{
	automation->SetMinSymbol(alphabet_min_symbol);
	automation->SetMaxSymbol(alphabet_max_symbol);
}

void CompressorsPool::RegisterCompressor(std::string name, std::unique_ptr<ICompressor> compressor)
{
	if (name.empty())
	{
		throw CompressorsError{"RegisterCompressor: name is empty"};
	}

	if (!compressor)
	{
		throw CompressorsError{"RegisterCompressor: compressor is nullptr"};
	}

	if (auto[compressor_iter, success] = compressor_instances_.emplace(std::move(name), std::move(compressor)); !success)
	{
		throw CompressorsError{"RegisterCompressor: trying to register already registered compressor"};
	}
}

ICompressor::SizeInBits CompressorsPool::Compress(
	const std::string& compressor_name,
	const unsigned char* data,
	size_t size)
{
	try
	{
		return compressor_instances_.at(compressor_name)->Compress(data, size, &output_buffer_);
	}
	catch (const std::out_of_range& e)
	{
		throw CompressorsError{"Incorrect compressor name " + compressor_name};
	}
}

std::vector<ICompressor::SizeInBits> CompressorsPool::CompressEndings(
	const std::string& compressor_name,
	const std::vector<Symbol>& historical_values,
	const ICompressor::Trajectories& possible_endings)
{
	try
	{
		return compressor_instances_.at(compressor_name)->CompressEndings(historical_values, possible_endings);
	}
	catch (const std::out_of_range& e)
	{
		throw CompressorsError{"Incorrect compressor name " + compressor_name};
	}
}

void CompressorsPool::SetAlphabetDescription(AlphabetDescription alphabet_description)
{
	for (auto&[name, compressor] : compressor_instances_)
	{
		compressor->SetTsParams(alphabet_description.min_symbol, alphabet_description.max_symbol);
	}
}

CompressorsFacadePtr MakeStandardCompressorsPool()
{
	auto to_return = std::make_shared<CompressorsPool>();

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
