/* Stubs for the pure CLI target (no Qt, no Motif, no OpenGL). */

#include "cinc.h"
#include "smd.h"

void drawwindow(int i)
{
  (void) i;
}

void gmorph_change_edit_type(int kind)
{
  /* Keep state consistent for any CLI code paths. */
  swin->edit_type = kind;
}

void gmorph_gl_pointer_event(int screen_idx, int x11_event_type,
                             unsigned int button, unsigned int state, int x, int y)
{
  (void) screen_idx;
  (void) x11_event_type;
  (void) button;
  (void) state;
  (void) x;
  (void) y;
}

void gmorph_qt_make_gl_current(int screen_idx) { (void) screen_idx; }
void gmorph_qt_request_update(int screen_idx) { (void) screen_idx; }
void gmorph_qt_process_events(void) {}
void gmorph_qt_set_window_title(const char *path) { (void) path; }

void gmorph_run_qt6_gui(int argc, char **argv, const char *loaded_gmh_path)
{
  (void) argc;
  (void) argv;
  (void) loaded_gmh_path;
  fprintf(stderr, "GMorph: GUI is not available in the CLI-only build.\n");
  exit(1);
}

int saveRgbImage(char *filename, long width, long height)
{
  (void) filename;
  (void) width;
  (void) height;
  return 0;
}

