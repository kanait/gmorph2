/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"
#include "color.h"

Swin *create_swin( void )
{
  Swin *swin;
  int   i;
  void  display3d_initialize(Disp3D *);
  void  screenatr_initialize(ScreenAtr *);
  Material *material_initialize(void);

  swin = (Swin *) malloc( sizeof(Swin) );

  display3d_initialize( &(swin->dis3d) );
  for (i = 0; i < 2; ++i) {
    screenatr_initialize( &(swin->screenatr[i]) );
    swin->screenatr[i].no = i;
  }

  /* tile display */
  swin->tileno   = 0;

  /* view attribs */
  swin->view_attribs = VIEW_ORGMESH;
  
  /* material initialize */

  swin->material = material_initialize();

  /* select type initialize */
  swin->select_type = SEL_CLEAR;
  swin->edit_type   = EDIT_NONE;
  
  /* morph ppd */
  swin->morph_ppd = (Sppd *) NULL;

  /* harmonic ppd */
  swin->hppd = (HPpd *) NULL;

  return swin;
}

void display3d_initialize(Disp3D *disp)
{
  /* 既定はシェーディングのみ ON（ワイヤは OFF） */
  disp->wire    = SMD_OFF;
  disp->shading = SMD_ON;
  disp->cpoint  = SMD_OFF;
  disp->cmesh   = SMD_OFF;
  disp->loop    = SMD_OFF;
  disp->group   = SMD_OFF;
  disp->spath   = SMD_OFF;
  disp->hmap    = SMD_OFF;
  disp->coaxis  = SMD_OFF;
}

void screenatr_initialize(ScreenAtr *screen)
{
  void set_default_view(ScreenAtr *);
  
  /* screen size */
/*   screen->width  = 700; */
/*   screen->height = 500; */
  screen->width  = 500;
  screen->height = 500;

  screen->xcenter = (double) screen->width  / 2.0;
  screen->ycenter = (double) screen->height / 2.0;
  
  /* GLX window */
  screen->glw    = NULL;
  screen->fr3d   = NULL;
  screen->vi     = NULL;
  screen->xc     = NULL;
  screen->glx_fbc = NULL;
  
  /* resize or not */
  screen->resize = SMD_OFF;

  /* zoom parameter */
  screen->zoom_active = SMD_OFF;
  screen->zoom = 1.0;

  /* rotate parameter */
  screen->rotate_type   = ROTATE_XY;
  screen->rotate_active = SMD_OFF;
  screen->rotate_x = 0.0;
  screen->rotate_y = 0.0;
  screen->rotate_z = 0.0;

  /* background color */
/*   screen->bgrgb[0] = 0.0; */
/*   screen->bgrgb[1] = 0.0; */
/*   screen->bgrgb[2] = 0.0; */
/*   screen->bgrgb[0] = 0.0; */
/*   screen->bgrgb[1] = 0.0; */
/*   screen->bgrgb[2] = 0.5; */
/*   screen->bgrgb[0] = blackvec[0]; */
/*   screen->bgrgb[1] = blackvec[1]; */
/*   screen->bgrgb[2] = blackvec[2]; */
/*   screen->bgrgb[0] = whitevec[0]; */
/*   screen->bgrgb[1] = whitevec[1]; */
/*   screen->bgrgb[2] = whitevec[2]; */
  screen->bgrgb[0] = bisquevec[0];
  screen->bgrgb[1] = bisquevec[1];
  screen->bgrgb[2] = bisquevec[2];

  /* set view initialize */

  set_default_view(screen);
  
  /* for select area */
  screen->areaflag  = SMD_OFF;
  screen->areaorg.x = 0.0;
  screen->areaorg.y = 0.0;
  screen->areaatv.x = 0.0;
  screen->areaatv.y = 0.0;

  /* select list */
  screen->sellist_flag = SMD_OFF;
  screen->n_sellist    = 0;
  screen->sel_first = screen->sel_last = (SelList *) NULL;

  /* for current_ppd */
  screen->current_ppd = (Sppd *) NULL;
  screen->view_ppd = (Sppd *) NULL;

  /* for shortest-path */
  screen->sg  = (SGraph *) NULL;
  screen->nsg = (SGraph *) NULL;
  screen->lp  = (SGlp *) NULL;
}

Material *material_initialize(void)
{
  Material *matl;

  matl = (Material *) malloc(sizeof(Material));

  matl->ambient[0] = 0.2;
  matl->ambient[1] = 0.2;
  matl->ambient[2] = 0.2;
  matl->ambient[3] = 1.0;
  matl->diffuse[0] = 0.8;
  matl->diffuse[1] = 0.8;
  matl->diffuse[2] = 0.8;
  matl->diffuse[3] = 1.0;
  matl->emission[0] = 0.0;
  matl->emission[1] = 0.0;
  matl->emission[2] = 0.0;
  matl->emission[3] = 1.0;
  matl->specular[0] = 0.0;
  matl->specular[1] = 0.0;
  matl->specular[2] = 0.0;
  matl->specular[3] = 1.0;
  matl->shininess[0] = 0.0;

  return matl;
}

void swin_free_ppd( Swin *swin )
{
  Sppd  *free_ppd( Sppd * );

  if ( swin->screenatr[0].current_ppd != (Sppd *) NULL ) 
    free_ppd( swin->screenatr[0].current_ppd );
  
  if ( swin->screenatr[1].current_ppd != (Sppd *) NULL ) 
    free_ppd( swin->screenatr[1].current_ppd );
  
  if ( swin->morph_ppd != (Sppd *) NULL ) 
    free_ppd( swin->morph_ppd );
}

void swin_free_hppd( Swin *swin )
{
  void free_hppd( HPpd * );

  if ( swin->hppd == (HPpd *) NULL ) return;
  free_hppd( swin->hppd );

  swin->screenatr[0].current_ppd = (Sppd *) NULL;
  swin->screenatr[1].current_ppd = (Sppd *) NULL;
  swin->morph_ppd = (Sppd *) NULL;
}
  
void free_swin( Swin *swin )
{
  void swin_free_hppd( Swin * );
  void swin_free_ppd( Swin * );

  swin_free_hppd( swin );
  swin_free_ppd( swin );
  
  free( swin->material );
  free( swin );
}
