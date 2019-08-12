#ifndef MOCK_SAMPLER_INCLUDED
#define MOCK_SAMPLER_INCLUDED

#include "sampler.h"

#include <gmock/gmock.h>

namespace itp {
  class SamplerMock : public Sampler {
    MOCK_METHOD3(sample, Sampling_result(const PlainTimeSeries<Double> &, size_t,
                                                  Double));
    MOCK_METHOD1(normalize, Sampling_result(const PlainTimeSeries<Symbol> &));
      MOCK_METHOD2(desample, Double(Double s, const Desample_info &info));
  };
}

#endif
