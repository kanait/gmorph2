/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

static int process_hits(GLint hits, GLuint buffer[])
{
  int i, j;
  GLuint names, *ptr;
  GLuint match[2];
  
  ptr = (GLuint *)buffer;
  for (i = 0; i < hits; i++) {
    names = *ptr;
    ptr++;
    ptr++;
    ptr++;
    for (j = 0; j < names; j++) {
      return *ptr;
      match[j] = (int)(*ptr);
      ptr++;
    }
  }
  return SMDNULL;
}

static int *process_hits_area(GLint hits, GLuint buffer[])
{
  int i, j;
  GLuint names, *ptr;
  int    *sel;
  
  ptr = (GLuint *)buffer;

  sel = (int *) malloc(hits * sizeof(int));
  
  for (i = 0; i < hits; i++) {
    names = *ptr;
    ptr++;
    ptr++;
    ptr++;
    for (j = 0; j < names; j++) {
      sel[i] = (int)(*ptr);
      ptr++;
    }
  }
  return sel;
}

#define PICKPOINTSIZE 10

int pick_on_point(double x, double y, double ix, double iy, double *dis)
{

  double subx, suby;

  subx = x - ix; suby = y - iy;
  
  if ( (fabs( subx ) < PICKPOINTSIZE) &&
       (fabs( suby ) < PICKPOINTSIZE) ) {
    *dis = subx*subx+suby*suby;
    return SMD_ON;
  }
  return SMD_OFF;
}

Spvt *pick_ppdvertex_screen( int x, int y, ScreenAtr *screen, Sppd *ppd )
{
  int    cnt;
  Spvt   *v, *pvt;
  double ix, iy;
  Vec2d  win2;
  double dis,dis0;
  void   get_GL_attributes(ScreenAtr *);
  void   world_to_win(Vec *, Vec2d *, double [16], double [16], int [4]);

  if ( ppd == (Sppd *) NULL ) return (Spvt *) NULL;
  
  get_GL_attributes(screen);

  ix = (double) x;
  iy = (double) screen->viewport[3] - (double) y;

  cnt = 0;
  pvt = (Spvt *) NULL;
  for ( v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {
    world_to_win( &(v->vec), &(win2), screen->mmat, screen->pmat,
                  screen->viewport );
    if ( pick_on_point( win2.x, win2.y, ix, iy, &dis ) ) {
      
      if ( cnt ) {
	if ( dis < dis0 ) {
	  dis0 = dis;
	  pvt  = v;
	  ++cnt;
	}
      } else {
	dis0 = dis;
	pvt  = v;
	++cnt;
      }
      
    }
  }

  return pvt;
}

Spvt *pick_ppdvertex_spath_screen( int x, int y, ScreenAtr *screen, Sppd *ppd )
{
  int    cnt;
  Spvt   *v, *pvt, *org_vt;
  Vted   *ve;
  double ix, iy;
  Vec2d  win2;
  double dis, dis0;
  void   get_GL_attributes(ScreenAtr *);
  void   world_to_win(Vec *, Vec2d *, double [16], double [16], int [4]);
  Spvt   *pick_ppdvertex_screen(int, int, ScreenAtr *, Sppd *);
  Spvt   *another_vt( Sped *, Spvt * );

  if ( ppd == (Sppd *) NULL ) return (Spvt *) NULL;

  if ( screen->n_sellist ) {
    
    get_GL_attributes( screen );

    ix = (double) x;
    iy = (double) screen->viewport[3] - (double) y;

    cnt = 0;
    pvt = (Spvt *) NULL;

    org_vt = screen->sel_last->vt;
    
    for ( ve = org_vt->svted; ve != (Vted *) NULL; ve = ve->nxt ) {

      v = another_vt( ve->ed, org_vt );
      world_to_win( &(v->vec), &(win2), screen->mmat, screen->pmat,
		    screen->viewport );
      if ( pick_on_point( win2.x, win2.y, ix, iy, &dis ) ) {
	
	if ( cnt ) {
	  if ( dis < dis0 ) {
	    dis0 = dis;
	    pvt  = v;
	    ++cnt;
	  }
	} else {
	  dis0 = dis;
	  pvt  = v;
	  ++cnt;
	}
      
      }
    }

    return pvt;
  
  } else {
    return pick_ppdvertex_screen( x, y, screen, ppd );
  }
    
}

static int sc_in_area(Vec2d * vec, Vec2d * org, Vec2d * atv)
{
  if ((vec->x >= org->x) && (vec->x <= atv->x) &&
      (vec->y <= org->y) && (vec->y >= atv->y))
    return SMD_ON;

  return SMD_OFF;
}

Spvt *pick_ppdvertex_screen_area(int x, int y, ScreenAtr *screen, Sppd *ppd)
{
  Spvt   *v;
  double ix, iy;
  Vec2d  win2;
  void   get_GL_attributes(ScreenAtr *);
  void   world_to_win(Vec *, Vec2d *, double [16], double [16], int [4]);

  if ( ppd == (Sppd *) NULL ) return (Spvt *) NULL;
  
  get_GL_attributes(screen);

  ix = (double) x;
  iy = (double) screen->viewport[3] - (double) y;

  for ( v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {
    world_to_win( &(v->vec), &(win2), screen->mmat, screen->pmat,
                  screen->viewport );
    if ( sc_in_area(&(win2), &(screen->areaorg), &(screen->areaatv)) ) {
      return v;
    }
  }

  return (Spvt *) NULL;
}

HVertex *pick_hvertex_screen(int x, int y, ScreenAtr *screen, HPpd *hppd)
{
  HVertex *hvt, *pvt;
  Spvt   *vt;
  double ix, iy;
  int    cnt;
  double dis, dis0;
  Vec2d  win2;
  void   get_GL_attributes(ScreenAtr *);
  void   world_to_win(Vec *, Vec2d *, double [16], double [16], int [4]);

  if ( hppd == (HPpd *) NULL ) return (HVertex *) NULL;
  
  get_GL_attributes(screen);

  ix = (double) x;
  iy = (double) screen->viewport[3] - (double) y;

  pvt = (HVertex *) NULL;
  cnt = 0;
  for ( hvt = hppd->shvt; hvt != (HVertex *) NULL; hvt = hvt->nxt ) {

    if ( screen->no == SCREEN1 ) vt = hvt->vt1;
    else vt = hvt->vt2;
    
    world_to_win( &(vt->vec), &(win2), screen->mmat, screen->pmat,
                  screen->viewport );
    if ( pick_on_point( win2.x, win2.y, ix, iy, &dis) ) {

      if ( cnt ) {
	if ( dis < dis0 ) {
	  dis0 = dis;
	  pvt  = hvt;
	  ++cnt;
	}
      } else {
	dis0 = dis;
	pvt  = hvt;
	++cnt;
      }

    }
  }

  return (HVertex *) pvt;
}

HVertex *pick_hvertex_screen_area(int x, int y, ScreenAtr *screen, HPpd *hppd)
{
  HVertex *hvt;
  Spvt   *v;
  double ix, iy;
  Vec2d  win2;
  void   get_GL_attributes(ScreenAtr *);
  void   world_to_win(Vec *, Vec2d *, double [16], double [16], int [4]);

  if ( hppd == (HPpd *) NULL ) return (HVertex *) NULL;
  
  get_GL_attributes( screen );

  ix = (double) x;
  iy = (double) screen->viewport[3] - (double) y + PICKPOINTSIZE;

  for ( hvt = hppd->shvt; hvt != (HVertex *) NULL; hvt = hvt->nxt ) {
    
    if ( screen->no == SCREEN1 ) v = hvt->vt1;
    else v = hvt->vt2;
    
    world_to_win( &(v->vec), &(win2), screen->mmat, screen->pmat,
                  screen->viewport );
    if ( sc_in_area(&(win2), &(screen->areaorg), &(screen->areaatv)) ) {
      return hvt;
    }
  }

  return (HVertex *) NULL;
}

HFace *pick_hppdface_screen( int x, int y, ScreenAtr *screen, HPpd *hppd )
{
  HFace  *hfc;
  double ix, iy;
  void   get_GL_attributes( ScreenAtr * );
  int    point_in_hface( HFace *, double, double, ScreenAtr * );

  if ( hppd == (HPpd *) NULL ) return (HFace *) NULL;
  
  get_GL_attributes( screen );

  ix = (double) x;
  iy = (double) screen->viewport[3] - (double) y;

  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {

    if( point_in_hface( hfc, ix, iy, screen ) ) {
      return hfc;
    }
    
  }
  return (HFace *) NULL;
}

int point_in_hface( HFace *fc, double x, double y, ScreenAtr *screen )
{
  HHalfedge *he1, *he2;
  Spvt   *vt1, *vt2;
  Vec2d  v1, v2, sub1, sub0;
  int    first_sign, sign;
  void   world_to_win(Vec *, Vec2d *, double [16], double [16], int [4]);
  int    V2LR( Vec2d *, Vec2d * );
  Vec2d  *V2Sub( Vec2d *, Vec2d *, Vec2d * );

  he1 = fc->shhe;
  he2 = he1->nxt;

  vt1 = ( !screen->no ) ? he1->vt->vt1 : he1->vt->vt2;
  vt2 = ( !screen->no ) ? he2->vt->vt1 : he2->vt->vt2;
  
  /* first edge */ 
  world_to_win( &(vt1->vec), &(v1), screen->mmat, screen->pmat,
		screen->viewport );
  world_to_win( &(vt2->vec), &(v2), screen->mmat, screen->pmat,
		screen->viewport );
  
  V2Sub( &(v2), &(v1), &sub0 );
  sub1.x = x - v1.x;
  sub1.y = y - v1.y;
  first_sign = V2LR( &(sub0), &(sub1) );

  he1 = he2;
  he2 = he2->nxt;

  do {
    vt1 = ( !screen->no ) ? he1->vt->vt1 : he1->vt->vt2;
    vt2 = ( !screen->no ) ? he2->vt->vt1 : he2->vt->vt2;
    world_to_win( &(vt1->vec), &(v1), screen->mmat, screen->pmat,
		  screen->viewport );
    world_to_win( &(vt2->vec), &(v2), screen->mmat, screen->pmat,
		  screen->viewport );
  
    V2Sub( &(v2), &(v1), &sub0 );
    sub1.x = x - v1.x;
    sub1.y = y - v1.y;
    sign = V2LR( &(sub0), &(sub1) );

    if ( sign != first_sign ) {
      return SMD_OFF;
    }

    he1 = he2;
    he2 = he2->nxt;
    
  } while ( he1 != fc->shhe );

  return SMD_ON;
}    

/* left or right ? */
int V2LR(Vec2d *a, Vec2d *b)
{
  double fg;
  
  fg =a->x * b->y - b->x * a->y;

  if ( fg >= 0.0 ) return SMD_ON;
  else return SMD_OFF;
}

Splp *pick_splp_screen( int x, int y, ScreenAtr *screen, HPpd *hppd )
{
  int    cnt;
  HEdge  *hed;
  Splp   *lp, *rlp;
  double dis0, dis;
  Splv   *lv, *nlv;
  Vec2d  ivec, vec1, vec2;
  void   get_GL_attributes(ScreenAtr *);
  void   world_to_win(Vec *, Vec2d *, double [16], double [16], int [4]);
  int    pick_on_line( Vec2d *, Vec2d *, Vec2d *, double * );

  if ( hppd == (HPpd *) NULL ) return (Splp *) NULL;
  
  get_GL_attributes( screen );

  ivec.x = (double) x;
  ivec.y = (double) screen->viewport[3] - (double) y;

  cnt = 0;
  rlp = (Splp *) NULL;
  for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {
    
    lp = (!screen->no) ? hed->lp1 : hed->lp2;
    
    if ( lp != (Splp *) NULL ) {
      
      nlv = lp->splv;
      lv = nlv->nxt;

      world_to_win( &(nlv->vt->vec), &(vec1), screen->mmat, screen->pmat,
		    screen->viewport );
      
      while ( lv != (Splv *) NULL ) {
	
	world_to_win( &(lv->vt->vec), &(vec2), screen->mmat, screen->pmat,
		      screen->viewport );
	
	if ( pick_on_line( &(ivec), &(vec1), &vec2, &dis) ) {
	  if ( cnt ) {
	    if ( dis < dis0 ) {
	      dis0 = dis;
	      rlp = lp;
	      ++cnt;
	    }
	  } else {
	    ++cnt;
	    dis0 = dis;
	    rlp = lp;
	  }
	}

	nlv = lv;
	lv = lv->nxt;
	vec1.x = vec2.x;
	vec1.y = vec2.y;
	
      }
      
    }
  }

  return rlp;
}

int pick_on_line( Vec2d *v, Vec2d *ev1, Vec2d *ev2, double *dis )
{
  double t;
  double point_line_distance_param( Vec2d *, Vec2d *, Vec2d *, double * );

  *dis = point_line_distance_param( v, ev1, ev2, &t );

  if ( (*dis < 0.5) && ( t > 0.0 ) && ( t < 1.0) ) {
    return SMD_ON;
  } else return SMD_OFF;
}

Spfc *pick_ppdface_screen( int x, int y, ScreenAtr *screen, Sppd *ppd )
{
  Spfc  *fc;
  double ix, iy;
  void   get_GL_attributes( ScreenAtr * );
  int    point_in_ppdface( Spfc *, double, double, ScreenAtr * );

  if ( ppd == (Sppd *) NULL ) return (Spfc *) NULL;
  
  get_GL_attributes( screen );

  display("aa\n");
  ix = (double) x;
  iy = (double) screen->viewport[3] - (double) y;

  for ( fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt ) {

    if( point_in_ppdface( fc, ix, iy, screen ) ) {
      return fc;
    }
    
  }
  return (Spfc *) NULL;
}

int point_in_ppdface( Spfc *fc, double x, double y, ScreenAtr *screen )
{
  Sphe *he1, *he2;
  Spvt   *vt1, *vt2;
  Vec2d  v1, v2, sub1, sub0;
  int    first_sign, sign;
  void   world_to_win(Vec *, Vec2d *, double [16], double [16], int [4]);
  int    V2LR( Vec2d *, Vec2d * );
  Vec2d  *V2Sub( Vec2d *, Vec2d *, Vec2d * );

  he1 = fc->sphe;
  he2 = he1->nxt;

  vt1 = he1->vt; vt2 = he2->vt;
  
  /* first edge */ 
  world_to_win( &(vt1->vec), &(v1), screen->mmat, screen->pmat,
		screen->viewport );
  world_to_win( &(vt2->vec), &(v2), screen->mmat, screen->pmat,
		screen->viewport );
  
  V2Sub( &(v2), &(v1), &sub0 );
  sub1.x = x - v1.x;
  sub1.y = y - v1.y;
  first_sign = V2LR( &(sub0), &(sub1) );

  he1 = he2;
  he2 = he2->nxt;

  do {
    vt1 = he1->vt;
    vt2 = he2->vt;
    world_to_win( &(vt1->vec), &(v1), screen->mmat, screen->pmat,
		  screen->viewport );
    world_to_win( &(vt2->vec), &(v2), screen->mmat, screen->pmat,
		  screen->viewport );
  
    V2Sub( &(v2), &(v1), &sub0 );
    sub1.x = x - v1.x;
    sub1.y = y - v1.y;
    sign = V2LR( &(sub0), &(sub1) );

    if ( sign != first_sign ) {
      return SMD_OFF;
    }

    he1 = he2;
    he2 = he2->nxt;
    
  } while ( he1 != fc->sphe );

  return SMD_ON;
}    

