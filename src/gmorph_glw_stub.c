/* Minimal GLw symbols for linking when libGLw is unavailable (e.g. macOS + Homebrew X11
 * without XQuartz). The Motif GL canvas is never used when running with -gui (Qt).
 * If these run, the process exits with a clear message. */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glx.h>
#include <X11/Intrinsic.h>

Widget GLwCreateMDrawingArea(Widget parent, String name, ArgList arglist, Cardinal argcount)
{
  (void)parent;
  (void)name;
  (void)arglist;
  (void)argcount;
  fprintf(stderr,
          "GMorph: GLwCreateMDrawingArea is unavailable (no libGLw). "
          "Run with -gui for Qt, or install GLw (e.g. XQuartz on macOS).\n");
  abort();
}

void GLwDrawingAreaMakeCurrent(Widget w, GLXContext ctx)
{
  (void)w;
  (void)ctx;
  fprintf(stderr,
          "GMorph: GLwDrawingAreaMakeCurrent is unavailable (no libGLw).\n");
  abort();
}
