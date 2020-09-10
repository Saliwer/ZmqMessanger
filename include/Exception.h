#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <exception>
#include "Config.h"

namespace ZmqMessanger
{
  class Exception : public std::exception
  {
    std::string _what;
  public:
    Exception(const char* message, ...);
    virtual ~Exception() throw() {};
    virtual const char* what() const throw() { return _what.c_str(); }
  };

}

#endif
