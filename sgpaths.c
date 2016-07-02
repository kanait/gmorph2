/* Copyright (c) 1997-1998 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

#include <unistd.h>

/* Motif functions */
void SGMakeShortestPathsCB( Widget w, XtPointer cld, XtPointer *cad )
{
  HPpd *hppd;
  void clear_paths( HPpd * );
  void make_shortest_path( HPpd * );
  void SGShortestPathsCreate( HPpd * );
  Boolean isokCalcShortestPath( HPpd * );
  void DisplayBusyCursor ( Widget w );
  void drawwindow( int );

  if ( (hppd = swin->hppd) == (HPpd *) NULL ) return;

  if ( isokCalcShortestPath( hppd ) == False ) {
    return;
  }
  
  clear_paths( hppd );

  /* make shortest path and make hloop */
  displayinfo("calculate shortest-path... ");
/*   sleep(1); */
  DisplayBusyCursor ( w );
  SGShortestPathsCreate( hppd );
  
  displayinfo("done. \n");
  drawwindow( SCREEN1 );
  drawwindow( SCREEN2 );
}

Boolean isokCalcShortestPath( HPpd *hppd )
{
  return True;
}

void SGClearShortestPathCB( Widget w, XtPointer cld, XtPointer *cad )
{
  HPpd *hppd;
  void clear_paths( HPpd * );
  
  if ( (hppd = swin->hppd) == (HPpd *) NULL ) {
    return;
  }
  clear_paths( hppd );
}

/* make shortest-path using SGraph */
void SGShortestPathsCreate( HPpd *hppd )
{
  HEdge  *hed;
  Sppd   *ppd;
  SGraph *sg;
  Spvt   *sv, *ev;
  void   time_start( void );
  void   time_stop( void );
  SGraph *initialize_sgraph( Sppd * );
  void   free_sgraph( SGraph * );
  Splp   *SGShortestPath( SGraph *, Sppd *, Spvt *, Spvt * );
  
  /* initialize */
  /* for calculate shortest-paths */

  display("number of hedge: %d\n", hppd->en );
  time_start();
  display("ppd1:\n");
  ppd = hppd->ppd1;
  sg  = initialize_sgraph( ppd );
  for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {
    display("\thedge %d/%d... ", hed->no+1, hppd->en);
    sv = hed->sv->vt1;
    ev = hed->ev->vt1;
    hed->lp1 = SGShortestPath( sg, ppd, sv, ev );
    hed->lp1->hed = hed;
    display("done.\n");
  }
  free_sgraph( sg );
  
  display("ppd2:\n");
  ppd = hppd->ppd2;
  sg  = initialize_sgraph( ppd );
  for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {
    display("\thedge %d/%d... ", hed->no+1, hppd->en);
    sv = hed->sv->vt2;
    ev = hed->ev->vt2;
    hed->lp2 = SGShortestPath( sg, ppd, sv, ev );
    hed->lp2->hed = hed;
    display("done.\n");
  }
  free_sgraph( sg );
  
  display("time:shortest-path\n");
  time_stop();
  
}

Splp *SGShortestPath( SGraph *sg, Sppd *ppd, Spvt *ppdsv, Spvt *ppdev )
{
  Splp  *lp;
  SGvt  *find_sgvt_from_spvt( SGraph *, Spvt * );
  Splp  *ShortestPath_Boundary( Sppd *, Spvt *, Spvt * );
  Splp  *CalcShortestPath( SGraph *, Sppd * );
  
  if ( (ppdsv->atr == VTXEXTERNAL) && (ppdev->atr == VTXEXTERNAL) ) {

    display(" boundary sv %d ev %d ", ppdsv->no, ppdev->no );
    lp = ShortestPath_Boundary( ppd, ppdsv, ppdev );
    
  } else {

    display(" onface sv %d ev %d ", ppdsv->no, ppdev->no );
    sg->src  = find_sgvt_from_spvt( sg, ppdsv );
    sg->dist = find_sgvt_from_spvt( sg, ppdev );
    
    if ( (sg->src == NULL) || (sg->dist == NULL) ) {
      return NULL;
    }
    
    lp = CalcShortestPath( sg, ppd );

  }

  return lp;
}

extern double sub_ratio;

Splp *CalcShortestPath( SGraph *sg, Sppd *ppd )
{
  int    i, iter;
  Boolean isCalculated;
  SGraph *org_sg, *new_sg;
  SGlp   *org_lp, *tmp_lp;
  Splp   *lp;
  void   add_vertices_edges_sgraph( SGraph * );
  SGraph *sglp_to_sgraph( SGlp *, SGraph * );
  SGlp   *ShortestPath_Dijkstra_SGraph( SGraph *, SGvt *, SGvt * );
  Splp   *sglp_to_splp( SGlp *, Sppd * );
  void   free_sglp( SGlp * );
  void   free_sgraph( SGraph * );
  
  i = 0;
  iter = 20;
  org_sg = sg;
  org_lp = NULL;
  new_sg = NULL;
  tmp_lp = NULL;
  isCalculated = False;
  sub_ratio = 0.5;

  while ( (i < iter) && (isCalculated == False) ) {

    /* STEP1 */
    if ( org_sg != sg ) {
      add_vertices_edges_sgraph( org_sg );
    }
    /* STEP2 */
    tmp_lp = ShortestPath_Dijkstra_SGraph( org_sg, org_sg->src, org_sg->dist );
    
    /* STEP3 */
    new_sg = sglp_to_sgraph( tmp_lp, org_sg );

    display("i = %d graph v %d f %d e %d length %g\n", i,
	    org_sg->sgvtn, org_sg->sgfcn, org_sg->sgedn, tmp_lp->length );
    
    if ( i > 4 ) {
      if ( fabs( new_sg->lp->length - org_sg->lp->length ) < 1.0e-05 ) {
	isCalculated = True;
      }
    }
    
    free_sglp( tmp_lp );
    tmp_lp = NULL;

    /* delete old graph and path */
    if ( org_sg != sg ) {
      free_sgraph( org_sg );
      org_sg = NULL;
      org_lp = NULL;
    }
    org_sg = new_sg;
    org_lp = new_sg->lp;
    ++i;
    sub_ratio *= 0.5;
  }

  /* create a new shortest-path loop */
  lp = sglp_to_splp( org_sg->lp, ppd );

  free_sgraph( org_sg );

  return lp;
}
  
Splp *sglp_to_splp( SGlp *sglp, Sppd *ppd )
{
  Boolean isVertexDeleted;
  Splp *lp;
  Spvt *ppddist, *ppdsv, *ppdev, *ppdvt;
  Splv *ppdlv;
  Sped *ppded;
  SGvt *sgsrc, *sgdist, *sgvt;
  SGed *sged, *next_sged;
  SGlpvt *lv, *next_lv;
  Spvt *create_ppdvertex( Sppd * );
  Splv *create_ppdloopvertex( Splp *lp );
  Vted *create_vtxed( Spvt *, Sped * );
  Sped *find_ppdedge( Spvt *, Spvt * );
  Sped *create_ppdedge( Sppd * );
  Splp *create_splp( void );
  SGed *find_sged( SGvt *, SGvt * );
  double V3DistanceBetween2Points( Vec *, Vec * );
  
  if ( sglp == NULL ) return NULL;

  lp = create_splp();
  lp->type  = SHORTESTPATH;

  /* starts from distination vertex */

  sgsrc  = sglp->elpvt->vt; 
  sgdist = sglp->slpvt->vt; ppddist = sgdist->spvt;
  sgvt = sgsrc;
  sged = NULL;

  lv = sglp->elpvt;
  next_lv = lv;

  while ( next_lv != sglp->slpvt ) {

    isVertexDeleted = False;
    next_lv   = lv->prv;
    next_sged = find_sged( lv->vt, next_lv->vt );
    
    if ( sged != NULL ) {
      if ( ( sged->sped != NULL ) && ( next_sged->sped != NULL ) &&
	   ( sged->sped == next_sged->sped ) ) {
	isVertexDeleted = True;
      }
    }

    if ( isVertexDeleted == False ) {
      /*
        ppdloopvertex is created.
	If sgvt doesn't have a ppdvertex, ppdvertex is also created.
      */
      sgvt = lv->vt;
      if ( sgvt->spvt == NULL ) {
	ppdvt = create_ppdvertex( ppd );
	ppdvt->vec.x = sgvt->vec.x;
	ppdvt->vec.y = sgvt->vec.y;
	ppdvt->vec.z = sgvt->vec.z;
	ppdvt->type = VERTEX_ADDED;
	ppdvt->bpso = ppd->spso;
	ppdvt->sp_type = SP_VERTEX_STEINER;
	/* if sgvt has spvt, sgvt always has sged. */
	ppdvt->sp_ed  = sgvt->sped;
	ppdvt->sp_val = ( V3DistanceBetween2Points( &(ppdvt->vec),
						    &(ppdvt->sp_ed->sv->vec) )
			  / ppdvt->sp_ed->length );
	
      } else {
	ppdvt = sgvt->spvt;
      }

      ppdlv = create_ppdloopvertex( lp );
      ppdlv->vt = ppdvt;
      ++( ppdvt->spn );

      /* change vt->sp_type */
      if ( ppdvt != ppddist ) {
	ppdvt->sp_type = SP_VERTEX_BOUNDARY;
	/* for grouping */
	ppdvt->lp = lp;
      }

    }
    
    lv = next_lv;
    sged = next_sged;
    
  }
  
  /* include a last vertex(src) */
  ppdvt = ppddist;
  ppdlv = create_ppdloopvertex( lp );
  ppdlv->vt = ppdvt; ++( ppdvt->spn );
  
  ppdlv = lp->splv;
  while ( ppdlv != lp->eplv ) {

    ppdsv = ppdlv->vt;
    ppdev = ppdlv->nxt->vt;

    /*
      If there is no ppdedge between two sgvts, ppdedge is also created.
    */
    if ( (ppded = find_ppdedge( ppdsv, ppdev )) == (Sped *) NULL ) {
      ppded = create_ppdedge( ppd );
      ppded->sv = ppdsv;
      ppded->ev = ppdev;
      ppded->type = EDGE_ADDED;
      (void) create_vtxed( ppdsv, ppded );
      (void) create_vtxed( ppdev, ppded );
    }
    ppded->sp_type = SP_EDGE_BOUNDARY;

    ppdlv = ppdlv->nxt;
  }
  
  return lp;
}
