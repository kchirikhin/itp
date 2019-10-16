#include "compressors.h"

#include <cassert>
#include <iostream>

namespace itp {
void Compressor::set_ts_params(Symbol, Symbol, size_t) {
  // DO NOTHING
}

Zstd_compressor::Zstd_compressor() {
  context = ZSTD_createCCtx();
}

Zstd_compressor::~Zstd_compressor() {
  ZSTD_freeCCtx(context);
}

size_t Zstd_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **tmp_buffer,
                                    size_t *buffer_size) {
  assert(buffer_size != nullptr);

  size_t dst_capacity = ZSTD_compressBound(size);
  fit_buffer(dst_capacity, tmp_buffer, buffer_size);

  auto result = ZSTD_compressCCtx(context, *tmp_buffer, *buffer_size, data, size, ZSTD_maxCLevel());
  return result;
}

size_t Zlib_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **tmp_buffer,
                                    size_t *buffer_size) {
  size_t dst_capacity = compressBound(size * sizeof(Symbol));
  fit_buffer(dst_capacity, tmp_buffer, buffer_size);
  if (compress2(*tmp_buffer, &dst_capacity,
                data, size, Z_BEST_COMPRESSION) != Z_OK) {
    throw std::runtime_error("zlib: an error occured.");
  }

  return dst_capacity;
}

size_t Ppm_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **tmp_buffer,
                                   size_t *buffer_size) {
  size_t dst_capacity = Ppmd::compress_bound(size);
  fit_buffer(dst_capacity, tmp_buffer, buffer_size);

  auto result = Ppmd::ppmd_compress(*tmp_buffer, *buffer_size, data, size);
  return result;
}

size_t Rp_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **,
                                  size_t *) {
  auto result = Rp::rp_compress(data, size);
  return result;
}

size_t Bzip2_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **tmp_buffer,
                                     size_t *buffer_size) {
  // according to documentation, such capacity guaranties that the compressed data will fit in the buffer
  std::unique_ptr<char[]> src(new char[size]);
  memcpy(src.get(), data, size * sizeof(Symbol));
  uint dst_capacity = static_cast<uint>(size * sizeof(Symbol) + ceil(size * sizeof(Symbol) * 0.01) + 600);
  fit_buffer(dst_capacity, tmp_buffer, buffer_size);
  if (BZ2_bzBuffToBuffCompress(reinterpret_cast<char *>(*tmp_buffer), &dst_capacity,
                               src.get(), static_cast<uint>(size), 9, 0, 30) != BZ_OK) {
    throw std::runtime_error("Bzip2: an error occured.");
  }

  return dst_capacity;
}

size_t Lca_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **,
                                   size_t *) {
  auto result = Lcacomp::lcacomp_compress(data, size);
  return result;
}

Automation_compressor::Automation_compressor()
    : automation {new Sensing_DFA {0, 255, 127}} {
  // DO NOTHING
}

size_t Automation_compressor::operator ()(const unsigned char *data, size_t size,
                                          unsigned char **, size_t *) {
  auto probability = automation->EvalProbability(PlainTimeSeries<Symbol>(data, data+size));
  auto res = ceil(-log2(automation->EvalProbability(PlainTimeSeries<Symbol>(data, data+size))));

  size_t final_result;
  if (std::numeric_limits<size_t>::max() < res) {
    final_result = std::numeric_limits<size_t>::max();
  } else {
    final_result = static_cast<size_t>(res);
  }
  
  return final_result;
}

void Automation_compressor::set_ts_params(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol, size_t) {
  automation->SetMinSymbol(alphabet_min_symbol);
  automation->SetMaxSymbol(alphabet_max_symbol);
}

Compressors_pool::~Compressors_pool() {
  delete[] buffer;
  for (auto &pair : compressor_instances) {
    delete pair.second;
  }
}

Compressors_pool::Compressors_pool() {
  compressor_instances.emplace("lcacomp", new Lca_compressor);
  compressor_instances.emplace("rp", new Rp_compressor);
  compressor_instances.emplace("zstd", new Zstd_compressor);
  compressor_instances.emplace("bzip2", new Bzip2_compressor);
  compressor_instances.emplace("zlib", new Zlib_compressor);
  compressor_instances.emplace("ppmd", new Ppm_compressor);
  compressor_instances.emplace("automation", new Automation_compressor);
}

void Compressors_pool::init_compressors_for_ts(Symbol min_symbol, Symbol max_symbol,
                                               size_t n_sym_to_evaluate) {
  for (auto compressor : compressor_instances) {
    compressor.second->set_ts_params(min_symbol, max_symbol, n_sym_to_evaluate);
  }
}

size_t Compressors_pool::operator()(const std::string &compressor_name,
                                    const unsigned char *data, size_t size) {
  try {
    return compressor_instances.at(compressor_name)->operator()(data, size, &buffer, &buffer_size);
  }
  
  catch (const std::out_of_range &e) {
    throw std::invalid_argument(std::string("Incorrect compressor name ") + compressor_name);
  }
}
} // itp
