/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"
#include "gmorph_qt_bridge.h"

/* "Select" primivite functions */

/* Event/mask constants (same numeric values as X11; used by Qt bridge). */
enum {
  ButtonPress = 4,
  ButtonRelease = 5,
  MotionNotify = 6,
  Button1 = 1,
  Button2 = 2,
  Button3 = 3,
  ShiftMask = 1 << 0,
  ControlMask = 1 << 2,
  Button1Mask = 1 << 8,
  Button2Mask = 1 << 9,
  Button3Mask = 1 << 10,
};

static void make_area(int x, int y, ScreenAtr *screen)
{
  screen->areaatv.x = (double) x;
  screen->areaatv.y = (double) (screen->height - y);
}

/* "Create PPDFace" functions */
static void select_ppdvertex( int x, int y, ScreenAtr *screen )
{
  Spvt *v;
  void FreeSelectList(ScreenAtr *);
  Spvt *pick_ppdvertex_screen(int, int, ScreenAtr *, Sppd *);
  SelList *create_selectlist_vt( ScreenAtr *, Spvt * );
  
  if ( screen->current_ppd == (Sppd *) NULL ) return;
  
  screen->areaflag  = SMD_ON;
  screen->areaorg.x = screen->areaatv.x = (double) x;
  screen->areaorg.y = screen->areaatv.y = (double) ( screen->height - y );
  
  v = pick_ppdvertex_screen( x, y, screen, screen->current_ppd );
  
  if ( v == (Spvt *) NULL ) return;

  /* this vertex has already selected. */
  if ( v->col != PNTGREEN ) {
    displayinfo("This vertex has been already selected.\n"); 
    return;
  }

  create_selectlist_vt( screen, v );
  
}

/* "Create HVertex" functions */
static void select_vertex_create_hvertex(int x, int y, ScreenAtr *screen)
{
  Spvt *v;
  void FreeSelectList(ScreenAtr *);
  Spvt *pick_ppdvertex_screen(int, int, ScreenAtr *, Sppd *);
  SelList *create_selectlist_vt( ScreenAtr *, Spvt * );
  
  if ( screen->current_ppd == (Sppd *) NULL ) return;
  
  screen->areaflag  = SMD_ON;
  screen->areaorg.x = screen->areaatv.x = (double) x;
  screen->areaorg.y = screen->areaatv.y = (double) ( screen->height - y );
  
  v = pick_ppdvertex_screen(x, y, screen, screen->current_ppd);
  
  if ( v == (Spvt *) NULL ) return;

  /* this vertex has already selected. */
  if ( v->col != PNTGREEN ) return;

  FreeSelectList(screen);

  create_selectlist_vt(screen, v);
  
}

static void select_vertex_area_create_hvertex(int x, int y, ScreenAtr *screen)
{
  Spvt *v;
  void FreeSelectList(ScreenAtr *);
  Spvt *pick_ppdvertex_screen_area(int, int, ScreenAtr *, Sppd *);
  SelList *create_selectlist_vt( ScreenAtr *, Spvt * );

  if ( screen->current_ppd == (Sppd *) NULL ) return;
  
  v = pick_ppdvertex_screen_area(x, y, screen, screen->current_ppd);

  if ( v == (Spvt *) NULL ) {
    screen->areaflag  = SMD_OFF;
    screen->areaorg.x = 0.0;
    screen->areaorg.y = 0.0;
    screen->areaatv.x = 0.0;
    screen->areaatv.y = 0.0;
    return;
  }

  /* this vertex has already selected. */
  if ( v->col != PNTGREEN ) {
    screen->areaflag  = SMD_OFF;
    screen->areaorg.x = 0.0;
    screen->areaorg.y = 0.0;
    screen->areaatv.x = 0.0;
    screen->areaatv.y = 0.0;
    return;
  }

  FreeSelectList(screen);

  create_selectlist_vt(screen, v);
  
  screen->areaflag  = SMD_OFF;
  screen->areaorg.x = 0.0;
  screen->areaorg.y = 0.0;
  screen->areaatv.x = 0.0;
  screen->areaatv.y = 0.0;
}


/* "Shortest Path" functions */
static void select_vertex_shortest_path(int x, int y, ScreenAtr *screen)
{
  Spvt *v;
  Spvt *pick_ppdvertex_screen(int, int, ScreenAtr *, Sppd *);
  SelList *create_selectlist_vt( ScreenAtr *, Spvt * );

  if ( screen->current_ppd == (Sppd *) NULL ) return;
  
  screen->areaflag  = SMD_ON;
  screen->areaorg.x = screen->areaatv.x = (double) x;
  screen->areaorg.y = screen->areaatv.y = (double) ( screen->height - y );
  
  v = pick_ppdvertex_screen(x, y, screen, screen->current_ppd);

  if ( v == (Spvt *) NULL ) return;

  /* this vertex has already selected. */
  if ( v->col == PNTRED ) return;
  
  create_selectlist_vt(screen, v);
  
}

static void select_vertex_area_shortest_path(int x, int y, ScreenAtr *screen)
{
  Spvt *v;
  Spvt *pick_ppdvertex_screen_area(int, int, ScreenAtr *, Sppd *);
  SelList *create_selectlist_vt( ScreenAtr *, Spvt * );

  if ( screen->current_ppd == (Sppd *) NULL ) return;
  
  v = pick_ppdvertex_screen_area(x, y, screen, screen->current_ppd);

  if ( v == (Spvt *) NULL ) return;

  /* this vertex has already selected. */
  if ( v->col != PNTGREEN ) return;

  create_selectlist_vt(screen, v);
  
  screen->areaflag  = SMD_OFF;
  screen->areaorg.x = 0.0;
  screen->areaorg.y = 0.0;
  screen->areaatv.x = 0.0;
  screen->areaatv.y = 0.0;
}

/* "Create HFace" functions */
static void select_hvertex_create_hface(int x, int y, ScreenAtr *screen)
{
  HVertex *hv;
  void    FreeSelectList(ScreenAtr *);
  HVertex *pick_hvertex_screen(int, int, ScreenAtr *, HPpd *);
  SelList *create_selectlist_hvt( ScreenAtr *, HVertex * );

  if ( swin->hppd == (HPpd *) NULL ) return;
  
  screen->areaflag  = SMD_ON;
  screen->areaorg.x = screen->areaatv.x = (double) x;
  screen->areaorg.y = screen->areaatv.y = (double) ( screen->height - y );
  
  hv = pick_hvertex_screen(x, y, screen, swin->hppd);

  if ( hv == (HVertex *) NULL ) return;

  /* this vertex has already selected. */
  if ( hv->col != PNTGREEN ) return;

  create_selectlist_hvt(screen, hv);
  
}

static void select_hvertex_area_create_hface(int x, int y, ScreenAtr *screen)
{
  HVertex *hv;
  void    FreeSelectList(ScreenAtr *);
  HVertex *pick_hvertex_screen_area(int, int, ScreenAtr *, HPpd *);
  SelList *create_selectlist_hvt( ScreenAtr *, HVertex * );

  if ( swin->hppd == (HPpd *) NULL ) return;
  
  hv = pick_hvertex_screen_area(x, y, screen, swin->hppd);

  if ( hv == (HVertex *) NULL ) return;

  /* this vertex has already selected. */
  if ( hv->col != PNTGREEN ) return;

  create_selectlist_hvt(screen, hv);
  
  screen->areaflag  = SMD_OFF;
  screen->areaorg.x = 0.0;
  screen->areaorg.y = 0.0;
  screen->areaatv.x = 0.0;
  screen->areaatv.y = 0.0;
}

/* "Delete HVertex" functions */
static void select_hvertex_delete_hvertex( int x, int y, ScreenAtr *screen )
{
  HVertex *hv;
  void    FreeSelectList( ScreenAtr * );
  HVertex *pick_hvertex_screen( int, int, ScreenAtr *, HPpd * );
  SelList *create_selectlist_hvt( ScreenAtr *, HVertex * );

  if ( swin->hppd == (HPpd *) NULL ) return;
  
/*   screen->areaflag  = SMD_ON; */
  screen->areaorg.x = screen->areaatv.x = (double) x;
  screen->areaorg.y = screen->areaatv.y = (double) ( screen->height - y );
  
  hv = pick_hvertex_screen( x, y, screen, swin->hppd );

  if ( hv == (HVertex *) NULL ) return;

  /* this vertex has already selected. */
  if ( hv->col != PNTGREEN ) return;

  create_selectlist_hvt( screen, hv );
  
}

/* "Create Loop" functions */
static void select_vertex_make_loop( int x, int y, ScreenAtr *screen )
{
  Spvt *v;
  void FreeSelectList(ScreenAtr *);
  Spvt *pick_ppdvertex_screen(int, int, ScreenAtr *, Sppd *);
  SelList *create_selectlist_vt( ScreenAtr *, Spvt * );

  if ( screen->current_ppd == (Sppd *) NULL ) return;
  
  screen->areaorg.x = screen->areaatv.x = (double) x;
  screen->areaorg.y = screen->areaatv.y = (double) ( screen->height - y );
  
  v = pick_ppdvertex_screen(x, y, screen, screen->current_ppd);

  if ( v == (Spvt *) NULL ) return;

  /* this vertex has already selected. */
  if ( v->col != PNTGREEN ) return;

  FreeSelectList(screen);

  create_selectlist_vt(screen, v);
  
  screen->areaflag  = SMD_ON;
}

static void select_vertex_area_make_loop( int x, int y, ScreenAtr *screen )
{
  Spvt *v;
  void FreeSelectList(ScreenAtr *);
  Spvt *pick_ppdvertex_screen_area(int, int, ScreenAtr *, Sppd *);
  SelList *create_selectlist_vt( ScreenAtr *, Spvt * );

  if ( screen->current_ppd == (Sppd *) NULL ) return;
  
  v = pick_ppdvertex_screen_area( x, y, screen, screen->current_ppd );

  if ( v == (Spvt *) NULL ) {
    screen->areaflag  = SMD_OFF;
    screen->areaorg.x = 0.0;
    screen->areaorg.y = 0.0;
    screen->areaatv.x = 0.0;
    screen->areaatv.y = 0.0;
    return;
  }

  /* this vertex has already selected. */
  if ( v->col != PNTGREEN ) {
    screen->areaflag  = SMD_OFF;
    screen->areaorg.x = 0.0;
    screen->areaorg.y = 0.0;
    screen->areaatv.x = 0.0;
    screen->areaatv.y = 0.0;
    return;
  }

  FreeSelectList(screen);

  create_selectlist_vt(screen, v);
  
  screen->areaflag  = SMD_OFF;
  screen->areaorg.x = 0.0;
  screen->areaorg.y = 0.0;
  screen->areaatv.x = 0.0;
  screen->areaatv.y = 0.0;
}

/* "Delete HFace" functions */
static void select_ppdface( int x, int y, ScreenAtr *screen )
{
  void FreeSelectList( ScreenAtr * );
  Sppd *ppd;
  Spfc *fc;
  Spfc *pick_ppdface_screen( int, int, ScreenAtr *, Sppd * );
  SelList *create_selectlist_ppdface( ScreenAtr *, Spfc * );

/*   if ( swin->hppd == (HPpd *) NULL ) return; */
  
  screen->areaorg.x = screen->areaatv.x = (double) x;
  screen->areaorg.y = screen->areaatv.y = (double) ( screen->height - y );

  ppd = screen->current_ppd;

  if ( ppd == (Sppd *) NULL ) return;
  
  fc = pick_ppdface_screen( x, y, screen, ppd );
  
  if ( fc == (Spfc *) NULL ) return;

  /* this vertex has already selected. */
  if ( fc->col == FACERED ) return;

  FreeSelectList(screen);

  create_selectlist_ppdface( screen, fc );
}

int CreatePPDFaceCheck( ScreenAtr *screen, Sppd *ppd )
{
  SelList *sl;
  Spvt *sv, *ev;
  Sped *ed;
  Sped *find_ppdedge( Spvt *, Spvt * );
  Spfc *find_ppdface_ppdedge( Sped *, Spvt *, Spvt * );
  
  /* faces must be triangle */
  if ( screen->n_sellist != TRIANGLE ) {
    displayinfo("The number of selected vertices are not three.\n");
    return SMD_OFF;
  }

  /* boundary edge check */
  for ( sl = screen->sel_first; sl != (SelList *) NULL; sl = sl->nxt ) {
    
    sv = sl->vt;
    ev = ( sl->nxt != NULL ) ? sl->nxt->vt : screen->sel_first->vt;
    
    if ( (ed = find_ppdedge( sv, ev )) != (Sped *) NULL ) {
      if ( ed->fn == 2 ) {
	displayinfo("Edge No.%d has two faces any longer.\n", ed->no);
	return SMD_OFF;
      }
      if ( find_ppdface_ppdedge( ed, sv, ev ) != (Spfc *) NULL ) {
	displayinfo("Edge No.%d has a face in the same side of an edge any longer.\n",
		    ed->no);
	return SMD_OFF;
      }
    }
    
  }
  
  return SMD_ON;
  
}

void CreateFace( ScreenAtr *screen, Sppd *ppd )
{
  SelList *sl;
  Spfc *fc;
  Spvt *sv, *ev;
  Sphe *he;
  Vted *ve;
  Sped *ed;
  Sped *find_ppdedge( Spvt *, Spvt * );
  Spfc *create_ppdface( Sppd * );
  Sphe *create_ppdhalfedge( Spfc * );
  Sped *create_ppdedge( Sppd * );
  Vted *create_vtxed( Spvt *, Sped * );
  void calc_fnorm( Spfc * );
  
  fc = create_ppdface( ppd );

  for ( sl = screen->sel_first; sl != (SelList *) NULL; sl = sl->nxt ) {
    
    sv = sl->vt;

    he = create_ppdhalfedge( fc );
    he->vt = sv;
    
    ev = ( sl->nxt != NULL ) ? sl->nxt->vt : screen->sel_first->vt;
    
    if ( (ed = find_ppdedge( sv, ev )) == (Sped *) NULL ) {
      
      ed = create_ppdedge( ppd );
      /* edge - vertex */
      ed->sv = sv;
      ed->ev = ev;
      /* vertex - edge */
      ve = create_vtxed( sv, ed );
      ve = create_vtxed( ev, ed );
      /* edge - face */
      ed->lf =  fc;
      
    } else {
      
      /* edge - face */
      if ( (ed->sv == sv) && (ed->ev == ev ) ) {
	ed->lf  = fc;
      } else {
	ed->rf  = fc;
      }
      
    }
    
    he->ed  = ed;
    ++(ed->fn);
  }

  calc_fnorm( fc );
  
}

static void edit_create_ppdface( ScreenAtr *screen )
{
  Sppd *ppd;
  int  CreatePPDFaceCheck( ScreenAtr *, Sppd * );
  void CreateFace( ScreenAtr *, Sppd * );
  void FreeSelectList( ScreenAtr * );
  
  ppd = screen->current_ppd;
  if ( ppd == NULL ) return;

  if ( CreatePPDFaceCheck( screen, ppd ) != SMD_ON ) {
    displayinfo("Can't create ppd face. \n");
    FreeSelectList(screen);
    return;
  }

 CreateFace( screen, ppd );
 FreeSelectList(screen);
 
}

/* Delete PPD Face */
static void DeleteFace( Spfc *fc, Sppd *ppd )
{
  Sped *ed;
  Sphe *he;
  void free_ppdedge( Sped *, Sppd * );
  Sphe *free_ppdhalfedge( Sphe *, Spfc * );
  
  if (fc == (Spfc *) NULL) return;

  he = fc->sphe;
  while ( he != (Sphe *) NULL ) {
    ed = he->ed;
    he = free_ppdhalfedge( he, fc );
    /* important. */
    if ( !(ed->fn) ) {
      free_ppdedge( ed, ppd );
    }
  }

  if ( ppd->spfc == fc ) {
    if ((ppd->spfc = fc->nxt) != (Spfc *) NULL)
      fc->nxt->prv = (Spfc *) NULL;
    else {
      ppd->epfc = (Spfc *) NULL;
    }
  } else if (ppd->epfc == fc) {
    fc->prv->nxt = (Spfc *) NULL;
    ppd->epfc = fc->prv;
    
  } else {
    fc->prv->nxt = fc->nxt;
    fc->nxt->prv = fc->prv;
  }

  free(fc);
  --( ppd->fn );
  
}

static void edit_delete_ppdface( ScreenAtr *screen )
{
  SelList *sl;
  Sppd *ppd;
  void free_ppdface( Spfc *, Sppd * );
  void FreeSelectList( ScreenAtr * );

  ppd = screen->current_ppd;
  
  for ( sl = screen->sel_first; sl != (SelList *) NULL; sl = sl->nxt ) {
    DeleteFace( sl->fc, ppd );
    sl->fc = (Spfc *) NULL;
  }
  FreeSelectList(screen);
}

/* "Delete HFace" functions */
static void select_hface( int x, int y, ScreenAtr *screen )
{
  HFace *fc;
  void FreeSelectList( ScreenAtr * );
  HFace *pick_hppdface_screen( int, int, ScreenAtr *, HPpd * );
  SelList *create_selectlist_hface( ScreenAtr *, HFace * );

  if ( swin->hppd == (HPpd *) NULL ) return;
  
  screen->areaorg.x = screen->areaatv.x = (double) x;
  screen->areaorg.y = screen->areaatv.y = (double) ( screen->height - y );

  fc = pick_hppdface_screen( x, y, screen, swin->hppd );
  
  if ( fc == (HFace *) NULL ) return;

  /* this vertex has already selected. */
  if ( fc->col == FACERED ) return;

  FreeSelectList(screen);

  create_selectlist_hface( screen, fc );
  displayinfo(" hface No.%d selected.\n", fc->no);
}

static void select_shortest_path( int x, int y, ScreenAtr *screen )
{
  Splp *l;
  void FreeSelectList( ScreenAtr * );
  Splp *pick_splp_screen( int, int, ScreenAtr *, HPpd * );
  SelList *create_selectlist_lp( ScreenAtr *, Splp * );

  if ( swin->hppd == (HPpd *) NULL ) return;
  
  l = pick_splp_screen( x, y, screen, swin->hppd );

  if ( l == (Splp *) NULL ) return;

  if ( l->col != LOOPBLUE ) { 
    displayinfo("this loop has been already selected.\n"); 
    return; 
  } 

  create_selectlist_lp( screen, l );
}

static void select_manual_shortest_path( int x, int y, ScreenAtr *screen )
{
  Sppd *ppd;
  Spvt *vt;
  void FreeSelectList( ScreenAtr * );
  Spvt *pick_ppdvertex_spath_screen( int, int, ScreenAtr *, Sppd * );
  SelList *create_selectlist_vt( ScreenAtr *, Spvt * );

  if ( (ppd = screen->current_ppd) == (Sppd *) NULL ) return;

  vt = pick_ppdvertex_spath_screen( x, y, screen, ppd );

  if ( vt == (Spvt *) NULL ) return;
  
  if ( vt->col != PNTGREEN ) {
    displayinfo("This vertex has been already selected.\n"); 
    return;
  }

  create_selectlist_vt( screen, vt );
  
}

void cancel_all( void )
{
  ScreenAtr *screen1, *screen2;
  void      FreeSelectList( ScreenAtr * );
  void      free_sglp( SGlp * );

  screen1 = &(swin->screenatr[0]);
  screen2 = &(swin->screenatr[1]);

  if ( screen1->lp != (SGlp *) NULL ) {
    free_sglp( screen1->lp );
    screen1->lp = NULL;
  }
  if ( screen2->lp != (SGlp *) NULL ) {
    free_sglp( screen2->lp );
  }
  
  FreeSelectList( screen1 );
  FreeSelectList( screen2 );
}

void gmorph_change_edit_type(int kind)
{
  int prev;
  void screen_initialize_sgraph(ScreenAtr *);
  void screen_exit_sgraph(ScreenAtr *);

  prev = swin->edit_type;
  cancel_all();
  if (prev == EDIT_SPATH) {
    screen_exit_sgraph(&(swin->screenatr[SCREEN1]));
    screen_exit_sgraph(&(swin->screenatr[SCREEN2]));
  }
  swin->edit_type = kind;
  if (swin->edit_type == EDIT_SPATH) {
    screen_initialize_sgraph(&(swin->screenatr[SCREEN1]));
    screen_initialize_sgraph(&(swin->screenatr[SCREEN2]));
  }
}

#define SHOW_ONE  0
#define SHOW_ALL  1

void gmorph_gl_pointer_event(int i, int event_type, unsigned int button,
			     unsigned int state, int x, int y)
{
  int    dx, dy;
  int    drawflag;
  int    camera_changed;
  Vec2d  xvec, ovec, nvec;
  double oangle, nangle, angle;
  ScreenAtr *screen;
  static unsigned int cur_button, cur_modify;
  static int scrn_x, scrn_y;
  /* rot, zoom */
  void   start_screen3d_rotate(ScreenAtr *);
  void   start_screen3d_zoom(ScreenAtr *);
  void   update_screen3d_rotate(ScreenAtr *, int, int);
  void   update_screen3d_rotate_z(ScreenAtr *, double);
  void   update_screen3d_zoom(ScreenAtr *, int, int);
  void   finish_screen3d_rotate(ScreenAtr *);
  void   finish_screen3d_zoom(ScreenAtr *);
  void   drawwindow(int);
  double V2Cosign(Vec2d *, Vec2d *);
  double V2Sign(Vec2d *, Vec2d *);
  void   create_hvertex( void );
  void   delete_hvertex( void );
  void   cancel_create_hvertex( void );
  void   cancel_delete_hvertex( void );
  void   create_hface( void );
  void   delete_hface( void );
  void   cancel_create_hface( void );
  void   cancel_delete_hface( void );
  void   cancel_create_ppdface( void );
  void   cancel_delete_ppdface( void );
  void   create_shortest_path( ScreenAtr * );
  void   delete_shortest_path( ScreenAtr * );
  void   manual_shortest_path( ScreenAtr * );
  void   cancel_create_shortestpath( ScreenAtr * );
  void   cancel_delete_shortestpath( ScreenAtr * );
  void   make_loop( ScreenAtr * );
  void   cancel_make_loop( ScreenAtr * );
  void   cancel_edit_sp_vertex( void );

  screen = &(swin->screenatr[i]);

  drawflag = SHOW_ONE;
  camera_changed = SMD_OFF;
  switch( event_type ) {
  case ButtonPress:
    cur_button = button;
    cur_modify = state;
    scrn_x = x; scrn_y = y;
    if (cur_modify & ShiftMask) {
      if (cur_button == Button1) {
	start_screen3d_rotate(screen);
      } else if (cur_button == Button2) {
	start_screen3d_zoom(screen);
      }
    } else {
      if (cur_button == Button1) {
	
	switch ( swin->edit_type ) {
	  
	case EDIT_HPOINT:
	  select_vertex_create_hvertex( x, y, screen );
	  break;
	  
	case EDIT_DEL_HPOINT:
	  select_hvertex_delete_hvertex( x, y, screen );
	  drawflag = SHOW_ALL;
	  break;
	  
	case EDIT_FACE:
	  select_ppdvertex( x, y, screen );
	  break;
	  
	case EDIT_DEL_FACE:
	  select_ppdface( x, y, screen );
	  drawflag = SHOW_ALL;
	  break;
	  
	case EDIT_HFACE:
	  select_hvertex_create_hface( x, y, screen );
	  drawflag = SHOW_ALL;
	  break;
	  
	case EDIT_LOOP:
	  select_vertex_make_loop( x, y, screen );
	  drawflag = SHOW_ALL;
	  break;
	  
	case EDIT_DEL_HFACE:
	  select_hface( x, y, screen );
	  drawflag = SHOW_ALL;
	  break;
	  
	case EDIT_SPATH:
	  select_vertex_shortest_path( x, y, screen );
	  break;
	  
	case EDIT_DEL_SPATH:
	  select_shortest_path( x, y, screen );
	  break;
	  
	case EDIT_SPATH_MANU:
	  select_manual_shortest_path( x, y, screen );
	  break;
	  
	}
	
      } else if (cur_button == Button2) {

	switch ( swin->edit_type ) {
	  
	case EDIT_HPOINT:
	  create_hvertex();
	  drawflag = SHOW_ALL;
	  break;
	  
	case EDIT_DEL_HPOINT:
	  delete_hvertex();
	  drawflag = SHOW_ALL;
	  break;
	  
	case EDIT_HFACE:
	  create_hface();
	  drawflag = SHOW_ALL;
	  break;
	  
	case EDIT_FACE:
	  edit_create_ppdface( screen );
	  break;
	  
	case EDIT_DEL_FACE:
	  edit_delete_ppdface( screen );
	  break;
	  
	case EDIT_LOOP:
	  make_loop( screen );
	  break;
	  
	case EDIT_DEL_HFACE:
	  delete_hface();
	  break;
	  
	case EDIT_SPATH:
	  create_shortest_path( screen );
	  break;
	  
	case EDIT_DEL_SPATH:
	  delete_shortest_path( screen );
	  break;
	  
	case EDIT_SPATH_MANU:
	  manual_shortest_path( screen );
	  break;
	  
	}
	
      } else if (cur_button == Button3) {

	switch ( swin->edit_type ) {

	case EDIT_HPOINT:
	  cancel_create_hvertex();
	  displayinfo("Create HVertex Canceled.\n");
	  drawflag = SHOW_ALL;
	  break;

	case EDIT_DEL_HPOINT:
	  cancel_delete_hvertex();
	  displayinfo("Delete HVertex Canceled.\n");
	  drawflag = SHOW_ALL;
	  break;
	  
	case EDIT_HFACE:
	  cancel_create_hface();
	  displayinfo("Create HFace Canceled.\n");
	  drawflag = SHOW_ALL;
	  break;
	  
	case EDIT_LOOP:
	  cancel_make_loop( screen );
	  displayinfo("Create PPD Loop Canceled.\n");
	  break;
	  
	case EDIT_DEL_HFACE:
	  cancel_delete_hface();
	  displayinfo("Delete HFace Canceled.\n");
	  break;
	  
	case EDIT_FACE:
	  cancel_create_ppdface();
	  displayinfo("Create PPD Face Canceled.\n");
	  break;
	  
	case EDIT_DEL_FACE:
	  cancel_delete_ppdface();
	  displayinfo("Delete PPD Face Canceled.\n");
	  break;
	  
	case EDIT_SPATH:
	  cancel_create_shortestpath( screen );
	  displayinfo("Create SPath Canceled.\n");
	  break;
	  
	case EDIT_DEL_SPATH:
	  cancel_delete_shortestpath( screen );
	  displayinfo("Delete SPath Canceled.\n");
	  break;
	  
	}
      }
    }
    break;
 case ButtonRelease:
    finish_screen3d_rotate(screen);
    finish_screen3d_zoom(screen);
    if ((cur_modify & ShiftMask) != 0 ||
	(((cur_modify & ControlMask) != 0) && cur_button == Button1)) {
      camera_changed = SMD_ON;
    }
    if (cur_modify & ShiftMask) { 
    } else {
      if (cur_button == Button1) {
	
	switch ( swin->edit_type ) {
	case EDIT_HPOINT:
	  select_vertex_area_create_hvertex( x, y, screen );
	  break;
	case EDIT_HFACE:
	  select_hvertex_area_create_hface( x, y, screen );
	  drawflag = SHOW_ALL;
	  break;
	case EDIT_SPATH:
	  select_vertex_area_shortest_path( x, y, screen );
	  break;
	case EDIT_LOOP:
	  select_vertex_area_make_loop( x, y, screen );
	  break;
	}
	
      }
    }
    cur_button = 0;
    break;
 case MotionNotify:
    dx = x - scrn_x;
    dy = y - scrn_y;
    if (cur_modify & ShiftMask) { 
      if (cur_button == Button1) { /* rotate */
	update_screen3d_rotate(screen, dx, dy);
	camera_changed = SMD_ON;
      } else if (cur_button == Button2) { /* zoom */
	update_screen3d_zoom(screen, dx, dy);
	camera_changed = SMD_ON;
      } 
    } else if (cur_modify & ControlMask) {
      if (cur_button == Button1) {
	xvec.x = 1.0; xvec.y = 0.0;
	ovec.x = (double) scrn_x - screen->xcenter;
	ovec.y = (double) scrn_y - screen->ycenter;
	nvec.x = (double) x - screen->xcenter;
	nvec.y = (double) y - screen->ycenter;
	angle  = 180.0 * acos(V2Cosign(&(nvec), &(ovec))) / SMDPI;
	if (V2Sign(&(nvec), &(ovec)) < 0.0) angle *= -1.0;
	update_screen3d_rotate_z(screen, (double) angle);
	camera_changed = SMD_ON;
      }
    } else {
      if (cur_button == Button1) {
	  
	switch ( swin->edit_type ) {
	case EDIT_HPOINT:
	  make_area( x, y, screen );
	  break;
	case EDIT_HFACE:
	  make_area( x, y, screen );
	  break;
	case EDIT_SPATH:
	  make_area( x, y, screen );
	  break;
	case EDIT_LOOP:
	  make_area( x, y, screen );
	  break;
	}
      }
    }

    scrn_x = x;
    scrn_y = y;
    break;

  }

  if (swin->use_qt_gui && swin->qt_sync_views == SMD_ON && camera_changed == SMD_ON) {
    int other = (i == SCREEN1) ? SCREEN2 : SCREEN1;
    gqt_copy_3d_camera(&swin->screenatr[other], screen);
  }

  if ( drawflag != SHOW_ALL ) {
    drawwindow( i );
    if (swin->use_qt_gui && swin->qt_sync_views == SMD_ON && camera_changed == SMD_ON) {
      int other = (i == SCREEN1) ? SCREEN2 : SCREEN1;
      drawwindow( other );
    }
  } else {
    drawwindow( SCREEN1 );
    drawwindow( SCREEN2 );
  }
  
}

