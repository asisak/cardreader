/* ========================================================================= */
/* logger.h -- rfcontrol 'logger' interface                                  */
/* ========================================================================= */

#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>

extern pid_t myPID;

/* ========================================================================= */
/* int logprintf(const char* module, const char* format, ...);               */
/* ------------------------------------------------------------------------- */
/*   logs debug/error messages                                               */
/* ========================================================================= */
int logprintf(const char* module, const char* format, ...) {
  va_list args;
  char buf[2048];
  char myformat[128];
  
  va_start(args, format);
  sprintf(myformat, "[%s %s %d] %s", "rfcontrol", module, myPID, format);
  int result = vsprintf(buf, myformat, args);
  fputs(buf, stderr);
  va_end(args);

  return result;
}
