#ifndef MOCK_SAMPLER_INCLUDED
#define MOCK_SAMPLER_INCLUDED

#include "sampler.h"

#include <gmock/gmock.h>

namespace itp {
  class SamplerMock : public Sampler {
    MOCK_METHOD3(sample, Sampling_result(const Plain_tseries<Double_t> &, size_t,
                                                  Double_t));
    MOCK_METHOD1(normalize, Sampling_result(const Plain_tseries<Symbol_t> &));
      MOCK_METHOD2(desample, Double_t(Double_t s, const Desample_info &info));
  };
}

#endif
