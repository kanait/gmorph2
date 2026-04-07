/* Shared morph/view helpers used by Qt (-gui) and legacy Motif UI. */

#include "cinc.h"
#include <string.h>
#include <unistd.h>

#include "motif.h"
#include "gldef.h"
#include "smd.h"

extern char versionshort[];

/* Morph animation frame delay (microseconds). */
#define GMORPH_MORPH_FRAME_USEC 20000

void gmorph_play_morph_animation(void)
{
  int a, i;
  Spvt *v;
  Sppd *ppd;
  Spfc *fc;
  int div;
  char file[BUFSIZ];
  double p, q;
  Vec *mvec1, *mvec2;
  void drawwindow(int);
  void calc_fnorm(Spfc *);
  void ppdnorm(Sppd *);
  int saveRgbImage(char *, long, long);

  if (swin->morph_ppd != (Sppd *) NULL) {
    swin->screenatr[0].view_ppd = (Sppd *) NULL;
    swin->screenatr[1].view_ppd = (Sppd *) NULL;
    drawwindow(SCREEN1);
    drawwindow(SCREEN2);
    if (swin->use_qt_gui)
      gmorph_qt_process_events();

    ppd = swin->screenatr[0].view_ppd = swin->morph_ppd;
    div = swin->mdiv - 1;
    if (div < 1)
      div = 1;
    if (ppd != (Sppd *) NULL) {
      for (a = 0; a <= div; ++a) {
        q = (double) a / (double) div;
        p = 1.0 - q;
        mvec1 = ppd->mvec1;
        mvec2 = ppd->mvec2;
        for (i = 0, v = ppd->spvt; v != (Spvt *) NULL; ++i, v = v->nxt) {
          v->vec.x = p * mvec1[i].x + q * mvec2[i].x;
          v->vec.y = p * mvec1[i].y + q * mvec2[i].y;
          v->vec.z = p * mvec1[i].z + q * mvec2[i].z;
        }
        for (fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt) {
          calc_fnorm(fc);
        }
        if (swin->smooth_shading) {
          ppdnorm(ppd);
        }
        drawwindow(SCREEN1);
        if (swin->use_qt_gui)
          gmorph_qt_process_events();
        usleep(GMORPH_MORPH_FRAME_USEC);
        if (swin->saveimg) {
          sprintf(file, "mymorph_%03d.sgi", a);
          (void) saveRgbImage(file, swin->screenatr[0].width - 1,
                              swin->screenatr[0].height - 1);
        }
      }
    }
  }
}

void gmorph_view_original_meshes(void)
{
  void drawwindow(int);

  swin->screenatr[0].view_ppd = swin->screenatr[0].current_ppd;
  swin->screenatr[1].view_ppd = swin->screenatr[1].current_ppd;

  drawwindow(SCREEN1);
  drawwindow(SCREEN2);

  swin->view_attribs = VIEW_ORGMESH;
}

void gmorph_reset_morph_view(void)
{
  Sppd *ppd;
  Spvt *v;
  Spfc *fc;
  Vec *mvec1;
  void calc_fnorm(Spfc *);
  void drawwindow(int);
  void ppdnorm(Sppd *);

  if (swin->hppd == NULL)
    return;
  if (swin->morph_ppd == NULL)
    return;

  ppd = swin->screenatr[0].view_ppd = swin->morph_ppd;
  swin->screenatr[1].view_ppd = (Sppd *) NULL;

  mvec1 = ppd->mvec1;
  for (v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt) {
    v->vec.x = mvec1[v->no].x;
    v->vec.y = mvec1[v->no].y;
    v->vec.z = mvec1[v->no].z;
  }
  for (fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt) {
    calc_fnorm(fc);
  }
  if (swin->smooth_shading) {
    ppdnorm(ppd);
  }

  drawwindow(SCREEN1);
  drawwindow(SCREEN2);

  swin->view_attribs = VIEW_INTPMESH;
}

void set_windowtitle(char *str)
{
  char title[BUFSIZ];

  snprintf(title, sizeof(title), "%s: %s", versionshort, str);
  if (swin->use_qt_gui) {
    gmorph_qt_set_window_title(title);
    return;
  }
}
