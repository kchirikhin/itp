#include "compressors.h"

#include <cassert>
#include <iostream>

void itp::Compressor::set_ts_params(Symbol_t, Symbol_t, size_t) {
    // DO NOTHING
}

itp::Zstd_compressor::Zstd_compressor() {
    context = ZSTD_createCCtx();
}

itp::Zstd_compressor::~Zstd_compressor() {
    ZSTD_freeCCtx(context);
}

size_t itp::Zstd_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **tmp_buffer,
                                      size_t *buffer_size) {
    assert(buffer_size != nullptr);

    size_t dst_capacity = ZSTD_compressBound(size);
    fit_buffer(dst_capacity, tmp_buffer, buffer_size);
    return ZSTD_compressCCtx(context, *tmp_buffer, *buffer_size, data, size, ZSTD_maxCLevel());
}

size_t itp::Zlib_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **tmp_buffer,
                                      size_t *buffer_size) {
    size_t dst_capacity = compressBound(size * sizeof(Symbol_t));
    fit_buffer(dst_capacity, tmp_buffer, buffer_size);
    if (compress2(*tmp_buffer, &dst_capacity,
                  data, size, Z_BEST_COMPRESSION) != Z_OK) {
        throw std::runtime_error("zlib: an error occured.");
    }
    std::cout << "Zlib: " << dst_capacity << std::endl;
    return dst_capacity;
}

size_t itp::Ppm_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **tmp_buffer,
                                     size_t *buffer_size) {
    size_t dst_capacity = Ppmd::compress_bound(size);
    fit_buffer(dst_capacity, tmp_buffer, buffer_size);
    auto result = Ppmd::ppmd_compress(*tmp_buffer, *buffer_size, data, size);
    return result;
}

size_t itp::Rp_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **,
                                    size_t *) {
    return Rp::rp_compress(data, size);
}

size_t itp::Bzip2_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **tmp_buffer,
                                       size_t *buffer_size) {
    // according to documentation, such capacity guaranties that the compressed data will fit in the buffer
    std::unique_ptr<char[]> src(new char[size]);
    memcpy(src.get(), data, size * sizeof(Symbol_t));
    uint dst_capacity = static_cast<uint>(size * sizeof(Symbol_t) + ceil(size * sizeof(Symbol_t) * 0.01) + 600);
    fit_buffer(dst_capacity, tmp_buffer, buffer_size);
    if (BZ2_bzBuffToBuffCompress(reinterpret_cast<char *>(*tmp_buffer), &dst_capacity,
                                 src.get(), static_cast<uint>(size), 9, 0, 30) != BZ_OK) {
        throw std::runtime_error("Bzip2: an error occured.");
    }

    return dst_capacity;
}

size_t itp::Lca_compressor::operator ()(const unsigned char *data, size_t size, unsigned char **,
                                     size_t *) {
    return Lcacomp::lcacomp_compress(data, size);
}

itp::Automation_compressor::Automation_compressor()
    : automation {new Sensing_DFA {0, 255, 127}} {}

size_t itp::Automation_compressor::operator ()(const unsigned char *data, size_t size,
                                               unsigned char **tmp_buffer, size_t *buffer_size) {
    auto probability = automation->EvalProbability(Plain_tseries<Symbol_t>(data, data+size));
    auto res = ceil(-log2(automation->EvalProbability(Plain_tseries<Symbol_t>(data, data+size))));
    size_t res_to_ret;
    if (std::numeric_limits<size_t>::max() < res) {
        res_to_ret = std::numeric_limits<size_t>::max();
    } else {
        res_to_ret = static_cast<size_t>(res);
    }
    std::cout << "Automation: " << res_to_ret << std::endl;
    return res_to_ret;
    // std::cout << res << std::endl;
    // return res;
}

void itp::Automation_compressor::set_ts_params(Symbol_t alphabet_min_symbol,
                                               Symbol_t alphabet_max_symbol,
                                               size_t n_sym_to_evaluate) {
    automation->set_min_symbol(alphabet_min_symbol);
    automation->set_max_symbol(alphabet_max_symbol);
}

itp::Compressors_pool::~Compressors_pool() {
    delete[] buffer;
    for (auto &pair : compressor_instances) {
        delete pair.second;
    }
}

itp::Compressors_pool::Compressors_pool() {
    compressor_instances.emplace("lcacomp", new Lca_compressor);
    compressor_instances.emplace("rp", new Rp_compressor);
    compressor_instances.emplace("zstd", new Zstd_compressor);
    compressor_instances.emplace("bzip2", new Bzip2_compressor);
    compressor_instances.emplace("zlib", new Zlib_compressor);
    compressor_instances.emplace("ppmd", new Ppm_compressor);
    compressor_instances.emplace("automation", new Automation_compressor);
}

void itp::Compressors_pool::init_compressors_for_ts(Symbol_t min_symbol, Symbol_t max_symbol,
                                                    size_t n_sym_to_evaluate) {
    for (auto compressor : compressor_instances) {
        compressor.second->set_ts_params(min_symbol, max_symbol, n_sym_to_evaluate);
    }
}

size_t itp::Compressors_pool::operator()(const std::string &compressor_name,
                                         const unsigned char *data, size_t size) {
  try {
    return compressor_instances.at(compressor_name)->operator()(data, size, &buffer, &buffer_size);
  }
  
  catch (const std::out_of_range &e) {
    throw std::invalid_argument(std::string("Incorrect compressor name ") + compressor_name);
  }
}
