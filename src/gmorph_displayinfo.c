/* Status / log lines for Qt and CLI (stdout when prn is set). */

#include "cinc.h"
#include <stdio.h>
#include <string.h>

#include "motif.h"
#include "gldef.h"
#include "smd.h"

void displayinfo(char *fmt, ...)
{
  char buf[BUFSIZ];
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  if (swin->prn) {
    fputs(buf, stdout);
    fflush(stdout);
  }
}
