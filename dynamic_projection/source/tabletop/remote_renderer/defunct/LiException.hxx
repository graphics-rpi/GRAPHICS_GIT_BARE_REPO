#ifndef _LI_EXCEPTION_HXX_
#define _LI_EXCEPTION_HXX_

#include "LiPrerequisites.hxx"

#include <stdexcept>

namespace LI {
  class FatalRuntimeError : public std::runtime_error {
  public:
    explicit FatalRuntimeError()
      : std::runtime_error("Fatal runtime error") {}

    explicit FatalRuntimeError(const char* error)
      : std::runtime_error(std::string("Fatal runtime error: ").append(error)) {}
  };

  class HardwareError : public std::runtime_error {
  public:
    explicit HardwareError()
      : std::runtime_error("Hardware error") {}

    explicit HardwareError(const char* error)
      : std::runtime_error(std::string("Hardware error: ").append(error)) {}
  };
}

#endif
