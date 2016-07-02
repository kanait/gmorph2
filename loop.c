/* Copyright (c) 1997 Takashi Kanai; All rights researved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

Splp *ppdfindloop( Sppd *ppd, Spvt *v )
{
  Splp *loop;
  Splv *lv;
  Sped *e0, *e1, *ed;
  Spvt *v0, *v1, *nv;
  Spvt *another_vt( Sped *, Spvt * );
  Sped *find_boundaryedge( Spvt * );
  Sped *find_nextedge( Spvt *, Sped *, Splp * );
  Sped *find_ppdedge( Spvt *, Spvt * );
  Splp *create_ppdloop( Sppd * );
  Splv *create_ppdloopvertex( Splp * );
  void free_ppdloop( Splp *, Sppd * );

/*   display("aa\n"); */
  if ( v == (Spvt *) NULL ) return (Splp *) NULL;

  if ( (e0 = find_boundaryedge( v )) == (Sped *) NULL ) return (Splp *) NULL;
  
  /* initialize loop */
  v0 = v1 = v;
  loop = create_ppdloop( ppd );
  lv = create_ppdloopvertex( loop );
  lv->vt = v0;

  while ( 1 ) {
    
    if ( (e1 = find_nextedge( v0, e0, loop )) == (Sped *) NULL ) {
      displayinfo("Create Loop: can't find a loop.\n");
      free_ppdloop( loop, ppd );
      return (Splp *) NULL;
    }
    nv = another_vt( e1, v0 );
    if ( nv == v1 ) {
      displayinfo("loop created. sum of vertices: %d.\n", loop->lvn);
      loop->type = CLOSEDLOOP;

      for ( lv = loop->splv; lv->nxt != (Splv *) NULL; lv = lv->nxt ) {
	if ( (ed = find_ppdedge( lv->vt, lv->nxt->vt )) != NULL ) {
	  ed->used_loop = SMD_ON;
	}
      }
      if ( (ed = find_ppdedge( loop->splv->vt, loop->eplv->vt )) != NULL ) {
	ed->used_loop = SMD_ON;
      }

      return (Splp *) loop;
    }
    
    lv = create_ppdloopvertex( loop );
    lv->vt = nv;
    
    if (loop->lvn > ppd->vn ) {
      displayinfo("Create Loop: can't find closed loop.\n");
      free_ppdloop( loop, ppd );
      return (Splp *) NULL;
    }
    
    /* next step */
    e0 = e1;
    v0 = nv;
    
  }
  
}

Sped *find_boundaryedge( Spvt *v )
{
  Vted *ve;
  
  if ( v == (Spvt *) NULL ) return (Sped *) NULL;

  for ( ve = v->svted; ve != (Vted *) NULL; ve = ve->nxt ) {
    if ( ve->ed->atr == EDGEBOUNDARY ) return ve->ed;
  }
  
}

Sped *find_nextedge( Spvt *vt, Sped *ed, Splp *lp )
{
  Vted *ve;
  Spvt *nvt;
  Spvt *another_vt( Sped *, Spvt * );
  Splv *find_ppdloopvertex( Splp *, Spvt * );

  if ( vt == (Spvt *) NULL ) return (Sped *) NULL;
  
  for ( ve = vt->svted; ve != (Vted *) NULL; ve = ve->nxt ) {
    
    if ( (ve->ed != ed) && (ve->ed->atr == EDGEBOUNDARY) ) {

      nvt = another_vt( ve->ed, vt );

      /* first vertex */
      if ( (lp->splv != (Splv *) NULL) && (lp->lvn != 1) ) {
	if ( nvt == lp->splv->vt ) return ve->ed;
      }
      
      if ( find_ppdloopvertex( lp, nvt ) == (Splv *) NULL ) {
	return ve->ed;
      }

    }

  }
  return (Sped *) NULL;
}

void make_loop( ScreenAtr *screen )
{
  Sppd *ppd;
  Spvt *vt;
  Splp *ppdfindloop( Sppd *, Spvt * );
  void FreeSelectList( ScreenAtr * );
  void drawwindow( int );
  
  if ( screen->current_ppd == (Sppd *) NULL ) return;
  if ( screen->n_sellist != 1 ) {
    FreeSelectList(screen);
    return;
  }

  ppd  = screen->current_ppd;
  vt   = screen->sel_first->vt;

  (void) ppdfindloop( ppd, vt );

  FreeSelectList(screen);
  
  drawwindow(screen->no);

}

void cancel_make_loop( ScreenAtr *screen )
{
  void FreeSelectList( ScreenAtr * );
  
  FreeSelectList(screen);
}

