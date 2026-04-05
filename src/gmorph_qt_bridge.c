#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"
#include "gmorph_qt_bridge.h"

#include <X11/Xlib.h>
#include <string.h>

void init_gl3d(ScreenAtr *);
void update_screen3d_zoom(ScreenAtr *, int, int);
void finish_screen3d_zoom(ScreenAtr *);

static Display *font_display;

void gmorph_qt_bridge_init_font_display(void)
{
  if (font_display == NULL)
    font_display = XOpenDisplay(NULL);
}

void gmorph_qt_bridge_bind_screen_display(int screen_idx)
{
  if (screen_idx < 0 || screen_idx > 1)
    return;
  swin->screenatr[screen_idx].x11_display = (void *) font_display;
}

void gmorph_qt_bridge_init_gl_screen(int screen_idx)
{
  if (screen_idx < 0 || screen_idx > 1)
    return;
  init_gl3d(&swin->screenatr[screen_idx]);
}

void gqt_set_file_context(int filed, int opend)
{
  swin->filed = (unsigned short) filed;
  swin->opend = (unsigned short) opend;
}

void gqt_set_qt_in_paint_gl(int on)
{
  swin->qt_in_paint_gl = on ? 1 : 0;
}

void gqt_copy_3d_camera(ScreenAtr *dst, const ScreenAtr *src)
{
  double asp;

  if (dst == NULL || src == NULL)
    return;
  asp = dst->pers.scraspct;
  dst->zoom = src->zoom;
  dst->zoom_active = src->zoom_active;
  dst->rotate_active = src->rotate_active;
  dst->rotate_type = src->rotate_type;
  dst->rotate_x = src->rotate_x;
  dst->rotate_y = src->rotate_y;
  dst->rotate_z = src->rotate_z;
  dst->zrot_rad = src->zrot_rad;
  memcpy(dst->rmat, src->rmat, sizeof(dst->rmat));
  dst->rotate_xvec = src->rotate_xvec;
  dst->rotate_yvec = src->rotate_yvec;
  dst->rotate_zvec = src->rotate_zvec;
  dst->pers = src->pers;
  dst->pers.scraspct = asp;
}

void gqt_screen_apply_wheel_zoom(int screen_idx, int angle_delta_y)
{
  ScreenAtr *s;
  int other;

  if (screen_idx < 0 || screen_idx > 1)
    return;
  s = &swin->screenatr[screen_idx];
  /* Match update_screen3d_zoom: dz += 5e-3*(dx-dy). Wheel up (positive y) zooms in. */
  update_screen3d_zoom(s, 0, -(angle_delta_y / 8));
  finish_screen3d_zoom(s);
  if (swin->qt_sync_views == SMD_ON) {
    other = 1 - screen_idx;
    gqt_copy_3d_camera(&swin->screenatr[other], s);
  }
}

void gmorph_qt_bridge_set_screen_geom(int screen_idx, int w, int h)
{
  ScreenAtr *s;

  if (screen_idx < 0 || screen_idx > 1)
    return;
  s = &swin->screenatr[screen_idx];
  s->width = w;
  s->height = h;
  s->xcenter = (double) w / 2.0;
  s->ycenter = (double) h / 2.0;
  s->resize = SMD_ON;
}

int *gqt_ptr_dis3d_wire(void) { return &swin->dis3d.wire; }
int *gqt_ptr_dis3d_shading(void) { return &swin->dis3d.shading; }
int *gqt_ptr_dis3d_cpoint(void) { return &swin->dis3d.cpoint; }
int *gqt_ptr_dis3d_cmesh(void) { return &swin->dis3d.cmesh; }
int *gqt_ptr_dis3d_loop(void) { return &swin->dis3d.loop; }
int *gqt_ptr_dis3d_group(void) { return &swin->dis3d.group; }
int *gqt_ptr_dis3d_spath(void) { return &swin->dis3d.spath; }
int *gqt_ptr_dis3d_hmap(void) { return &swin->dis3d.hmap; }
int *gqt_ptr_dis3d_coaxis(void) { return &swin->dis3d.coaxis; }
int *gqt_ptr_ext_display(void) { return &swin->ext_display; }
int *gqt_ptr_smooth_shading(void) { return &swin->smooth_shading; }
int *gqt_ptr_mdiv(void) { return &swin->mdiv; }
int *gqt_ptr_qt_sync_views(void) { return &swin->qt_sync_views; }
int *gqt_ptr_edit_type(void) { return &swin->edit_type; }

void gqt_shortest_paths_create(void)
{
  void makeshortestpathcb(Widget, XtPointer, XtPointer *);

  makeshortestpathcb((Widget) NULL, (XtPointer) NULL, (XtPointer *) NULL);
}

void gqt_shortest_paths_clear(void)
{
  void clearshortestpathcb(Widget, XtPointer, XtPointer *);

  clearshortestpathcb((Widget) NULL, (XtPointer) NULL, (XtPointer *) NULL);
}
