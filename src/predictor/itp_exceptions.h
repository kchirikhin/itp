#ifndef ITP_EXCEPTIONS_H_INCLUDED_
#define ITP_EXCEPTIONS_H_INCLUDED_

#include <exception>
#include <string_view>

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

class RangeError : public ItpException {
  using ItpException::ItpException;
};
} // itp

#endif // ITP_EXCEPTIONS_H_INCLUDED_
