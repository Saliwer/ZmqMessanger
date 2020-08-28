#ifndef __DEBUG_H__
#define __DEBUG_H__

const int DBG_LEVEL_ERROR    =  0;
const int DBG_LEVEL_INFO     =  1;
const int DBG_LEVEL_DEFAULT  =  2;
const int DBG_LEVEL_VERBOSE  =  3;
const int DBG_LEVEL_DEBUG    = 10;

namespace ZmqMessanger
{
  extern int DLG_DEBUG_LEVEL;

  void Print(int debug_level, const char* fmt, ...);

} // namespace ZmqMessanger

#endif

