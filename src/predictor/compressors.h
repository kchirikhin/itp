#ifndef ITP_COMPRESSORS_H_INCLUDED_
#define ITP_COMPRESSORS_H_INCLUDED_

#include "dtypes.h"

#include <zstd.h>
#include <bzlib.h>
#include <zlib.h>
#include <rp.h>
#include <lcacomp.h>
#include <ppmd.h>
#include <sdfa.h>

#include <iostream>
#include <memory>
#include <memory.h>
#include <cmath>
#include <unordered_map>

namespace itp {

class Compressor {
 public:
  virtual size_t operator ()(const unsigned char *data, size_t size,
                             unsigned char **tmp_buffer, size_t *buffer_size) = 0;
  virtual ~Compressor() = default;

  virtual void set_ts_params(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol,
                             size_t n_sym_to_evaluate);
 protected:
  void fit_buffer(size_t desired_size, unsigned char **tmp_buffer, size_t *buffer_size) {
    if (*buffer_size < desired_size) {
      delete[] *tmp_buffer;
      *tmp_buffer = new unsigned char[desired_size];
      *buffer_size = desired_size;
    }
  }
};

class Zstd_compressor : public Compressor {
 public:
  Zstd_compressor();
  ~Zstd_compressor() override;

  size_t operator ()(const unsigned char *data, size_t size,
                     unsigned char **tmp_buffer, size_t *buffer_size) override;
 private:
  ZSTD_CCtx *context;
};

class Zlib_compressor : public Compressor {
 public:
  size_t operator ()(const unsigned char *data, size_t size,
                     unsigned char **tmp_buffer, size_t *buffer_size) override;
};

class Ppm_compressor : public Compressor {
 public:
  size_t operator ()(const unsigned char *data, size_t size,
                     unsigned char **tmp_buffer, size_t *buffer_size) override;
};

class Rp_compressor : public Compressor {
 public:
  size_t operator ()(const unsigned char *data, size_t size,
                     unsigned char **tmp_buffer, size_t *buffer_size) override;
};

class Bzip2_compressor : public Compressor {
 public:
  size_t operator ()(const unsigned char *data, size_t size,
                     unsigned char **tmp_buffer, size_t *buffer_size) override;
};

class Lca_compressor : public Compressor {
 public:
  size_t operator ()(const unsigned char *data, size_t size,
                     unsigned char **tmp_buffer, size_t *buffer_size) override;
};

class Automation_compressor : public Compressor {
 public:
  Automation_compressor();
        
  size_t operator ()(const unsigned char *data, size_t size,
                     unsigned char **tmp_buffer, size_t *buffer_size) override;

  void set_ts_params(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol,
                     size_t n_sym_to_evaluate) override;
 private:
  PredictionAutomationPtr automation;
};

/**
 * Singleton pattern (see Gamma et al., 1995).
 * Main purpose - to avoid unnecessary memory allocations by maintaining a single
 * buffer for handling all temporary compressed data.
 */
class Compressors_pool {
 public:
  Compressors_pool(const Compressors_pool &) = delete;
  void operator =(const Compressors_pool &) = delete;

  static Compressors_pool& get_instance() {
    static Compressors_pool instance;

    return instance;
  }

  void init_compressors_for_ts(Symbol, Symbol, size_t);
        
  size_t operator()(const std::string &, const unsigned char *, size_t);
  virtual ~Compressors_pool();
 protected:
  Compressors_pool();
 protected:
  std::unordered_map<std::string, Compressor*> compressor_instances;
  unsigned char *buffer = nullptr;
  size_t buffer_size = 0;
};
} // of itp

#endif // ITP_COMPRESSORS_H_INCLUDED_
