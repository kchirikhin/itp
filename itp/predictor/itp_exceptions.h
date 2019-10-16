
#ifndef ITP_EXCEPTIONS_H_INCLUDED_
#define ITP_EXCEPTIONS_H_INCLUDED_

#include <exception>
#include <string_view>

#define DECLARE_ITP_EXCEPTION_SUBTYPE(Name) \
  class Name : public ItpException { \
    using ItpException::ItpException; \
  }

namespace itp {

class ItpException : public std::exception {
 public:
  ItpException(std::string_view description)
      : description_(description) {
    // DO NOTHING
  }

  const char* what() const noexcept override {
    return description_.c_str();
  }
  
 private:
  std::string description_;
};

DECLARE_ITP_EXCEPTION_SUBTYPE(RangeError);
DECLARE_ITP_EXCEPTION_SUBTYPE(SeriesTooShortError);
DECLARE_ITP_EXCEPTION_SUBTYPE(EmptyInputError);
DECLARE_ITP_EXCEPTION_SUBTYPE(InvalidDigitError);
DECLARE_ITP_EXCEPTION_SUBTYPE(InvalidBaseError);
DECLARE_ITP_EXCEPTION_SUBTYPE(NotImplementedError);
DECLARE_ITP_EXCEPTION_SUBTYPE(DifferentHistoryLengthsError);
DECLARE_ITP_EXCEPTION_SUBTYPE(IntervalsCountError);
} // itp

#endif // ITP_EXCEPTIONS_H_INCLUDED_
