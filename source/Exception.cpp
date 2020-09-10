#include "Exception.h"

namespace ZmqMessanger
{
  Exception::Exception(const char* message,...)
  {
    char msg[MAX_EXCEPTION_MSG_LENGTH];
    va_list ap;
    va_start(ap,message);
    vsnprintf(msg,sizeof(msg)-1,message,ap);
    msg[MAX_EXCEPTION_MSG_LENGTH-1] = 0;
    _what = msg;
    va_end(ap);
  }
}
