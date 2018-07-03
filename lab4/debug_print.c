
#include <stdarg.h>
#include <stdio.h>
#include "debug_print.h"

void printdbg(const char *format, ...){
  va_list args;
  va_start(args, format);

  if(ENABLE_DBG_PRINT)
          vprintf(format, args);

  va_end(args);

}
