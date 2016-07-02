/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

/******************************************************************
                       HPpd functions 
******************************************************************/

HPpd *create_hppd(void)
{
  HPpd *ppd;
  
  if ((ppd = (HPpd *) malloc(sizeof(HPpd))) == (HPpd *) NULL) {
    return (HPpd *) NULL;
  }
  
  /* face */
  ppd->fn   = 0;
  ppd->fid  = 0;
  ppd->shfc = (HFace *) NULL;
  ppd->ehfc = (HFace *) NULL;
  
  /* vertex */
  ppd->vn   = 0;
  ppd->vid  = 0;
  ppd->shvt = (HVertex *) NULL;
  ppd->ehvt = (HVertex *) NULL;
  
  /* edge */
  ppd->en   = 0;
  ppd->eid  = 0;
  ppd->shed = (HEdge *) NULL;
  ppd->ehed = (HEdge *) NULL;

  /* for grouping */
  ppd->hgppd1 = (HGppd *) NULL;
  ppd->hgppd2 = (HGppd *) NULL;
  
  /* for morphing */
  ppd->gppd  = (Sppd *) NULL;
  
  return ppd;
}

void FreeHppdEdge ( HPpd *ppd )
{
  HEdge *edge, *nedge;
  void free_hppdedge(HEdge *, HPpd *);
  
  /* edge */
  for (edge = ppd->shed; edge != (HEdge *) NULL; edge = nedge) {
    nedge = edge->nxt;
    free_hppdedge(edge, ppd);
  }
  ppd->en = 0;
  ppd->shed = ppd->ehed = (HEdge *) NULL;
}

void FreeHppdFace( HPpd *ppd )
{
  HFace *face, *nface;
  void free_hppdface(HFace *, HPpd *);

  /* face */
  for (face = ppd->shfc; face != (HFace *) NULL; face = nface) {
    nface = face->nxt;
    free_hppdface(face, ppd);
  }
  ppd->fn = 0;
  ppd->shfc = ppd->ehfc = (HFace *) NULL;
}

void free_hppd( HPpd *ppd )
{
  HFace *face, *nface;
  HVertex *vertex, *nvertex;
  HEdge *edge, *nedge;
  void free_ppd( Sppd * );
  void free_hppdedge(HEdge *, HPpd *);
  void free_hppdface(HFace *, HPpd *);
  void free_hppdvertex(HVertex *, HPpd *);
  void FreeHGppd( HPpd * );

  if ( ppd == (HPpd *) NULL ) return;

  if ( ppd->gppd != (Sppd *) NULL ) free_ppd( ppd->gppd );
  
  FreeHGppd( ppd );

  /* edge */
  for (edge = ppd->shed; edge != (HEdge *) NULL; edge = nedge) {
    nedge = edge->nxt;
    free_hppdedge( edge, ppd );
  }
  /* face */
  for (face = ppd->shfc; face != (HFace *) NULL; face = nface) {
    nface = face->nxt;
    free_hppdface( face, ppd );
  }
  /* vertex */
  for (vertex = ppd->shvt; vertex != (HVertex *) NULL; vertex = nvertex) {
    nvertex = vertex->nxt;
    free_hppdvertex( vertex, ppd );
  }

  if ( ppd->ppd1 != (Sppd *) NULL ) free_ppd( ppd->ppd1 );
  if ( ppd->ppd2 != (Sppd *) NULL ) free_ppd( ppd->ppd2 );
  
  /* ppd */
  free( ppd );
}

void calc_hppdfnorm( HFace *hface  )
{
  HHalfedge *he;
  HVertex *hv1, *hv2, *hv3;
  Spvt   *v1, *v2, *v3;
  Vec    vec1, vec2, vec3, vec4, vec5;
  Vec    *V3Sub(Vec *, Vec *, Vec *);
  Vec    *V3Cross(Vec *, Vec *, Vec *);
  Vec    *V3Normalize(Vec *);

  he = hface->shhe;
  
  hv1 = he->vt;
  hv2 = he->nxt->vt;
  hv3 = he->nxt->nxt->vt;

  /* no.1 */
  vec1.x = hv1->vt1->vec.x;
  vec1.y = hv1->vt1->vec.y;
  vec1.z = hv1->vt1->vec.z;
  vec2.x = hv2->vt1->vec.x;
  vec2.y = hv2->vt1->vec.y;
  vec2.z = hv2->vt1->vec.z;
  vec3.x = hv3->vt1->vec.x;
  vec3.y = hv3->vt1->vec.y;
  vec3.z = hv3->vt1->vec.z;
  V3Sub( &vec2, &vec1, &vec4 );
  V3Sub( &vec3, &vec1, &vec5 );
  V3Cross( &vec4, &vec5, &(hface->nrm1) );
  V3Normalize( &(hface->nrm1) );

  /* no.2 */
  vec1.x = hv1->vt2->vec.x;
  vec1.y = hv1->vt2->vec.y;
  vec1.z = hv1->vt2->vec.z;
  vec2.x = hv2->vt2->vec.x;
  vec2.y = hv2->vt2->vec.y;
  vec2.z = hv2->vt2->vec.z;
  vec3.x = hv3->vt2->vec.x;
  vec3.y = hv3->vt2->vec.y;
  vec3.z = hv3->vt2->vec.z;
  V3Sub( &vec2, &vec1, &vec4 );
  V3Sub( &vec3, &vec1, &vec5 );
  V3Cross( &vec4, &vec5, &(hface->nrm2) );
  V3Normalize( &(hface->nrm2) );
}

/***************************************
  Vertex
****************************************/

/* vertex */
HVertex *create_hppdvertex(HPpd *ppd)
{
  HVertex *vt;

  vt = (HVertex *) malloc(sizeof(HVertex));

  vt->nxt = (HVertex *) NULL;
  if (ppd->shvt == (HVertex *) NULL) {
    vt->prv   = (HVertex *) NULL;
    ppd->ehvt = ppd->shvt = vt;
  } else {
    vt->prv   = ppd->ehvt;
    ppd->ehvt = ppd->ehvt->nxt = vt;
  }

  vt->vt1 = (Spvt *) NULL;
  vt->vt2 = (Spvt *) NULL;

  vt->col = PNTGREEN;
  vt->no  = ppd->vid;
  vt->sid = SMDNULL;

  vt->atr = VTXINTERNAL;
  
  ++( ppd->vn );
  ++( ppd->vid );

  return vt;
}

HVertex *list_hppdvertex(HPpd *ppd, Id id)
{
  HVertex *vt;
  
  for (vt = ppd->shvt; vt != (HVertex *) NULL; vt = vt->nxt) {
    if (vt->no == id) return vt;
  }
  return (HVertex *) NULL;
}

void free_hppdvertex(HVertex *vt, HPpd *ppd)
{
  
  if (vt == (HVertex *) NULL) return;
  
  if (ppd->shvt == vt) {
    if ((ppd->shvt = vt->nxt) != (HVertex *) NULL)
      vt->nxt->prv = (HVertex *) NULL;
    else {
      ppd->ehvt = (HVertex *) NULL;
    }
  } else if (ppd->ehvt == vt) {
    vt->prv->nxt = (HVertex *) NULL;
    ppd->ehvt = vt->prv;
  } else {
    vt->prv->nxt = vt->nxt;
    vt->nxt->prv = vt->prv;
  }

  free(vt);
  --( ppd->vn );
}

/***************************************
  Edge
****************************************/

HEdge *create_hppdedge( HPpd *ppd )
{
  HEdge *ed;

  ed = (HEdge *) malloc(sizeof(HEdge));

  ed->nxt = (HEdge *) NULL;
  if (ppd->shed == (HEdge *) NULL) {
    ed->prv   = (HEdge *) NULL;
    ppd->ehed = ppd->shed = ed;
  } else {
    ed->prv   = ppd->ehed;
    ppd->ehed = ppd->ehed->nxt = ed;
  }

  ed->evn = 0;
  ed->sedvt = ed->eedvt = (Hedvt *) NULL;
  
  ed->sv  = (HVertex *) NULL;
  ed->ev  = (HVertex *) NULL;

  ed->lp1 = (Splp *) NULL;
  ed->lp2 = (Splp *) NULL;
  ed->mlp = (Splp *) NULL;
  
  ed->rf  = (HFace *) NULL;
  ed->lf  = (HFace *) NULL;

  ed->col = EDGEWHITE;
  ed->atr = EDGEINTERNAL;

  ed->no  = ppd->eid;

  ++( ppd->en );
  ++( ppd->eid );
  
  return ed;
}

HEdge *find_hppdedge( HVertex *shvt, HVertex *ehvt, HPpd *hppd )
{
  HEdge *hed;
  
  if ( shvt == (HVertex *) NULL ) return (HEdge *) NULL;
  if ( ehvt == (HVertex *) NULL ) return (HEdge *) NULL;
  
  for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {
    
    if ( (shvt->no == hed->sv->no) && (ehvt->no == hed->ev->no) )
      return hed;
    if ( (ehvt->no == hed->sv->no) && (shvt->no == hed->ev->no) )
      return hed;

  }
  return (HEdge *) NULL;
}
  
HEdge *find_hppdedge_from_spvt( Spvt *sv, Spvt *ev, ScreenAtr *screen, HPpd *hppd )
{
  HEdge *hed;
  Spvt *ppdsv, *ppdev;

  if ( hppd == NULL ) return (HEdge *) NULL;
  if ( sv == (Spvt *) NULL ) return (HEdge *) NULL;
  if ( ev == (Spvt *) NULL ) return (HEdge *) NULL;
  
  for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {

    ppdsv = ( screen->no == SCREEN1 ) ? hed->sv->vt1 : hed->sv->vt2;
    ppdev = ( screen->no == SCREEN1 ) ? hed->ev->vt1 : hed->ev->vt2;
    
    if ( (ppdsv == sv) && (ppdev == ev) ) return hed;
    if ( (ppdsv == ev) && (ppdev == sv) ) return hed;

  }
  return (HEdge *) NULL;
}
  
void free_hppdedge( HEdge *ed, HPpd *ppd )
{
  void free_splp( Splp * );
  void free_hppdedgevertex( HEdge * );
  
  if (ed == (HEdge *) NULL) return;

  free_hppdedgevertex( ed );

  /* delete shortest-path */
  free_splp( ed->lp1 );
  free_splp( ed->lp2 );
  
  if (ppd->shed == ed) {
    if ((ppd->shed = ed->nxt) != (HEdge *) NULL)
      ed->nxt->prv = (HEdge *) NULL;
    else {
      ppd->ehed = (HEdge *) NULL;
    }
  } else if (ppd->ehed == ed) {
    ed->prv->nxt = (HEdge *) NULL;
    ppd->ehed = ed->prv;
  } else {
    ed->prv->nxt = ed->nxt;
    ed->nxt->prv = ed->prv;
  }

  free(ed);
  --( ppd->en );
}

/*
  Edge -> Vertex 
  for hppd_to_gppd (vt is a gppd's vertex)
*/

Hedvt *create_hedvt( HEdge *edge, Spvt *vt )
{
  Hedvt *ev;

  if (vt == (Spvt *) NULL) return (Hedvt *) NULL;
  
  ev = (Hedvt *) malloc(sizeof(Hedvt));

  ev->nxt = (Hedvt *) NULL;
  if (edge->sedvt == (Hedvt *) NULL) {
    ev->prv       = (Hedvt *) NULL;
    edge->eedvt = edge->sedvt = ev;
  } else {
    ev->prv       = edge->eedvt;
    edge->eedvt = edge->eedvt->nxt = ev;
  }

  /* vertex */
  ev->vt = vt;
  ev->val = 0.0;
  
  ++(edge->evn);
  
  return ev;
}

Hedvt *insert_hedvt( HEdge *hed, Spvt *vt, double val )
{
  Hedvt   *ev, *nev;
  Hedvt   *create_hedvt( HEdge *, Spvt * );
  
  /* first */
  if (hed->sedvt == (Hedvt *) NULL) {
    ev = create_hedvt( hed, vt );
    ev->val = val;
    return ev;
  }
  
  /* calc angle */
  ev    = hed->sedvt;
  while ((ev != (Hedvt *) NULL) && (val > ev->val)) {
    ev = ev->nxt;
  }

  /* last */
  if (ev == (Hedvt *) NULL) {
    ev = create_hedvt( hed, vt );
    ev->val = val;
    return ev;
  } 

  nev = (Hedvt *) malloc(sizeof(Hedvt));

  nev->prv = ev->prv;
  nev->nxt = ev;
  if (ev->prv != (Hedvt *) NULL) 
    ev->prv->nxt = nev;
  ev->prv = nev;
  if (hed->sedvt == ev) hed->sedvt = nev;

  nev->vt = vt;
  nev->val = val;

  ++( hed->evn );

  return nev;
}

void free_hppdedgevertex( HEdge *ed )
{
  Hedvt *ev, *evnxt;
  void  free_hedvt( Hedvt *, HEdge * );

  for (ev = ed->sedvt; ev != (Hedvt *) NULL; ev = evnxt) {
    evnxt = ev->nxt;
    free_hedvt( ev, ed );
    
  }
  ed->evn = 0;
  ed->sedvt = ed->eedvt = (Hedvt *) NULL;
}

void free_hedvt( Hedvt *ev, HEdge *ed )
{
  if (ev == (Hedvt *) NULL) return;

  ev->vt = (Spvt *) NULL;
  
  if (ed->sedvt == ev) {
    if ((ed->sedvt = ev->nxt) != (Hedvt *) NULL)
      ev->nxt->prv = (Hedvt *) NULL;
    else {
      ed->eedvt = (Hedvt *) NULL;
    }
  } else if (ed->eedvt == ev) {
    ev->prv->nxt = (Hedvt *) NULL;
    ed->eedvt = ev->prv;
  } else {
    ev->prv->nxt = ev->nxt;
    ev->nxt->prv = ev->prv;
  }
  free(ev);
  --( ed->evn );
}

/***************************************
  Face
****************************************/

/* face */
HFace *create_hppdface( HPpd *ppd )
{
  HFace *fc;

  fc = (HFace *) malloc(sizeof(HFace));
  
  fc->nxt = (HFace *) NULL;

  if (ppd->shfc == (HFace *) NULL) {
    fc->prv = (HFace *) NULL;
    ppd->ehfc = ppd->shfc = fc;
  } else {
    fc->prv   = ppd->ehfc;
    ppd->ehfc = ppd->ehfc->nxt = fc;
  }

  fc->hen   = 0;
  fc->shhe  = (HHalfedge *) NULL;
  fc->no    = ppd->fid;
  fc->col   = FACEBLUE;

  /* hgfc */
  fc->hgfc1 = (HGfc *) NULL;
  fc->hgfc2 = (HGfc *) NULL;
  fc->mhgfc = (HGfc *) NULL;
  
  /* hloop */
  fc->hloop = (HLoop *) NULL;
  
  fc->col = FACEBLUE;

  ++( ppd->fn );
  ++( ppd->fid );

  return fc;
}

HFace *another_hface( HEdge *ed, HFace *fc )
{
  if ( ed == (HEdge *) NULL ) return (HFace *) NULL;
  if ( fc == (HFace *) NULL ) return (HFace *) NULL;

  if ( ed->rf == fc ) return ed->lf;
  else return ed->rf;
}

HFace *list_hppdface( HPpd *ppd, Id id )
{
  HFace *fc;

  if ( id == SMDNULL ) 
    return (HFace *) NULL;
  
  for ( fc = ppd->shfc; fc != (HFace *) NULL; fc = fc->nxt ) {
    if (fc->no == id) return fc;
  }
  return (HFace *) NULL;
}

int find_hppdface_edge_links( HFace *hfc, HEdge *hed )
{
  HHalfedge *he;
  HVertex *vt, *nvt;
  
  he = hfc->shhe;

  do {
    vt  = he->vt;
    nvt = he->nxt->vt;

    if ( (hed->sv == vt) && (hed->ev == nvt ) ) {
      he->hed = hed;
      /* in order */
      return SMD_OFF;
    }
    if ( (hed->ev == vt) && (hed->sv == nvt ) ) {
      he->hed = hed;
      /* reverse */
      return SMD_ON;
    }
  } while ( (he = he->nxt) != hfc->shhe );
}
    
void free_hppdface( HFace *fc, HPpd *ppd )
{
  HHalfedge *he;
  HHalfedge *free_hppdhalfedge(HHalfedge *, HFace *);
  void free_splp( Splp * );
  void free_hgfc( HGfc * );
  
  if (fc == (HFace *) NULL) return;

  free_hgfc( fc->mhgfc );
  
  if ( fc->hloop != (HLoop *) NULL ) {
    free_splp( fc->hloop->lp1 );
    free_splp( fc->hloop->lp2 );
    free( fc->hloop );
  }
    
  /* free halfedge */
  he = fc->shhe;
  while (he != (HHalfedge *) NULL) {
    he = free_hppdhalfedge(he, fc);
  }
  
  if (ppd->shfc == fc) {
    if ((ppd->shfc = fc->nxt) != (HFace *) NULL)
      fc->nxt->prv = (HFace *) NULL;
    else {
      ppd->ehfc = (HFace *) NULL;
    }
  } else if (ppd->ehfc == fc) {
    fc->prv->nxt = (HFace *) NULL;
    ppd->ehfc = fc->prv;
    
  } else {
    fc->prv->nxt = fc->nxt;
    fc->nxt->prv = fc->prv;
  }

/*   free(fc->nrm); */
  free(fc);
  --( ppd->fn );
}

/* halfedge */
HHalfedge *create_hppdhalfedge(HFace *fc)
{
  HHalfedge *he;

  he = (HHalfedge *) malloc(sizeof(HHalfedge));
  
  if (fc->shhe == (HHalfedge *) NULL) {	/* first */
    fc->shhe = he;
    he->prv  = he;
    he->nxt  = he;
  } else {
    he->prv  = fc->shhe->prv;
    he->nxt  = fc->shhe;
    fc->shhe->prv->nxt = he;
    fc->shhe->prv = he;
  }
  /* id */
  he->no = fc->hen;
  /* start vertex */
  he->vt  = (HVertex *) NULL;

  /* links to hedge */
  he->hed = (HEdge *) NULL;
  
  ++( fc->hen );

  return he;
}

HHalfedge *free_hppdhalfedge(HHalfedge *he, HFace *face)
{
  HHalfedge *newhe;
  
  if (he->nxt == he) {
    free(he);
    --(face->hen);
    return (HHalfedge *) NULL;
  } else {
    newhe = he->nxt;
    he->prv->nxt = he->nxt;
    he->nxt->prv = he->prv;
    free(he);
    --(face->hen);
    return newhe;
  }
}

void calc_hppd_boundary( HPpd *ppd )
{
  HEdge *e;
  HVertex *sv, *ev;
  
  for ( e = ppd->shed; e != (HEdge *) NULL; e = e->nxt ) {

    if ( (e->lf == (HFace *) NULL) || (e->rf == (HFace *) NULL) ) {
      e->atr     = EDGEBOUNDARY;
      e->sv->atr = VTXEXTERNAL;
      e->ev->atr = VTXEXTERNAL;
    } else {
      e->atr = EDGEINTERNAL;
    }
    
  }
}

/* "Create HVertex" functions */
void create_hvertex( void )
{
  ScreenAtr *screen1, *screen2;
  HPpd    *hppd;
  Spvt    *vt1, *vt2;
  HVertex *hvt;
  HPpd    *create_hppd(void);
  HVertex *create_hppdvertex(HPpd *);
  void    FreeSelectList(ScreenAtr *);
  
  screen1 = &(swin->screenatr[0]);
  screen2 = &(swin->screenatr[1]);

  if ( !(screen1->n_sellist) || !(screen2->n_sellist) ) return;

  vt1 = screen1->sel_first->vt;
  vt2 = screen2->sel_first->vt;

  if ( (vt1->sp_type != SP_VERTEX_NORMAL) ||
       (vt2->sp_type != SP_VERTEX_NORMAL) ) {
    return;
  }

  /* create hppd */
  if ( swin->hppd == (HPpd *) NULL ) {
    hppd = swin->hppd = create_hppd();
    
    hppd->ppd1 = screen1->current_ppd;
    hppd->ppd2 = screen2->current_ppd;
    
  } else {
    hppd = swin->hppd;
  }
    
  /* create hvertex */
  hvt = create_hppdvertex(hppd);
  hvt->vt1 = vt1;
  hvt->vt2 = vt2;
  /* change vt->sp_type */
  hvt->vt1->sp_type = SP_VERTEX_HVERTEX;
  hvt->vt2->sp_type = SP_VERTEX_HVERTEX;
  vt1->hvt = hvt;
  vt2->hvt = hvt;
  
  FreeSelectList( screen1 );
  FreeSelectList( screen2 );
}

void cancel_create_hvertex( void )
{
  ScreenAtr *screen1, *screen2;
  void    FreeSelectList(ScreenAtr *);

  screen1 = &(swin->screenatr[0]);
  screen2 = &(swin->screenatr[1]);

  FreeSelectList( screen1 );
  FreeSelectList( screen2 );
}

void delete_hvertex( void )
{
  ScreenAtr *screen1, *screen2;
  void    delete_hvertex_screen( ScreenAtr * );
  void    FreeSelectList(ScreenAtr *);
  
  screen1 = &(swin->screenatr[0]);
  screen2 = &(swin->screenatr[1]);

  delete_hvertex_screen( screen1 );
  delete_hvertex_screen( screen2 );
  
  FreeSelectList( screen1 );
  FreeSelectList( screen2 );
}

void delete_hvertex_screen( ScreenAtr *screen )
{
  SelList *sl;
  HVertex *hvt;
  void free_hppdvertex( HVertex *, HPpd * );
  void    FreeSelectList(ScreenAtr *);

  for ( sl = screen->sel_first; sl != (SelList *) NULL; sl = sl->nxt ) {
    
    if ( (hvt = sl->hvt) != (HVertex *) NULL ) {
      
      hvt->vt1->sp_type = SP_VERTEX_NORMAL;
      hvt->vt2->sp_type = SP_VERTEX_NORMAL;
      hvt->vt1->hvt = (HVertex *) NULL;
      hvt->vt2->hvt = (HVertex *) NULL;

      free_hppdvertex( hvt, swin->hppd );
    }
  }
  FreeSelectList( screen );
}


void cancel_delete_hvertex( void )
{
  ScreenAtr *screen1, *screen2;
  void    FreeSelectList(ScreenAtr *);

  screen1 = &(swin->screenatr[0]);
  screen2 = &(swin->screenatr[1]);

  FreeSelectList( screen1 );
  FreeSelectList( screen2 );
}

/* "Create HFace" functions */
void create_hface( void )
{
  ScreenAtr *screen, *screen1, *screen2;
  HPpd      *hppd;
  HVertex   *hvt, *shvt, *ehvt;
  HFace     *hfc;
  HEdge     *hed;
  HHalfedge *hhe;
  SelList   *sl;
  HFace     *create_hppdface( HPpd * );
  HEdge     *create_hppdedge( HPpd * );
  HHalfedge *create_hppdhalfedge( HFace * );
  void      FreeSelectList(ScreenAtr *);
  HEdge     *find_hppdedge( HVertex *, HVertex *, HPpd * );
  Splp      *ShortestPath_Dijkstra(Sppd *, Spvt *, Spvt *);
  int       check_hppdface( ScreenAtr *, HPpd * );
/*   void      create_hloop( HFace *, HPpd * ); */
  
  screen1 = &(swin->screenatr[0]);
  screen2 = &(swin->screenatr[1]);

  if ( (hppd = swin->hppd) == (HPpd *) NULL ) {
    FreeSelectList( screen1 );
    FreeSelectList( screen2 );
    return;
  }
    
  if ( !(screen1->n_sellist) && !(screen2->n_sellist) ) {
    FreeSelectList( screen1 );
    FreeSelectList( screen2 );
    return;
  }

  if ( screen1->n_sellist ) {
    screen = screen1;
  } else {
    screen = screen2;
  }

  /* only a triangle is generated */
  if ( screen->n_sellist < TRIANGLE ) {
    displayinfo("points must be more than 3.\n");
    FreeSelectList( screen1 );
    FreeSelectList( screen2 );
    return;
  }

  /* one of edges has more than 3 faces */

  if ( (check_hppdface( screen, hppd ) != SMD_OFF) ) {
    FreeSelectList( screen1 );
    FreeSelectList( screen2 );
    return;
  }
    
  hfc = create_hppdface( hppd );

  shvt = screen->sel_last->hvt;
  for ( sl = screen->sel_first; sl != (SelList *) NULL; sl = sl->nxt ) {

    hhe = create_hppdhalfedge( hfc );
    ehvt = hhe->vt = sl->hvt;

    /* create harmonic edge */
    if ( (hed = find_hppdedge( shvt, ehvt, hppd )) == (HEdge *) NULL ) {
      
      hed = create_hppdedge( hppd );
      hed->sv = shvt;
      hed->ev = ehvt;

      /* make ppd open loop and link PPDLoop to HEdge */
/*       hed->lp1 = ShortestPath_Dijkstra( screen1->current_ppd, shvt->vt1, ehvt->vt1 ); */
/*       hed->lp1->hed = hed; */
      
/*       hed->lp2 = ShortestPath_Dijkstra( screen2->current_ppd, shvt->vt2, ehvt->vt2 ); */
/*       hed->lp2->hed = hed; */

    } 
    /* link HHalfedge to HEdge */
    hhe->hed = hed;
    
    /* link HEdge to HFace */
    if ( hed->rf == (HFace *) NULL ) hed->rf = hfc;
    else hed->lf = hfc;

    /* next vertex */
    shvt = ehvt;
  }

  calc_hppdfnorm( hfc  );

/*   create_hloop( hfc, hppd ); */
  
  FreeSelectList( screen1 );
  FreeSelectList( screen2 );
}

void delete_hface( void )
{
  ScreenAtr *screen1, *screen2;
  void delete_hface_screen( ScreenAtr * );
  
  screen1 = &(swin->screenatr[0]);
  screen2 = &(swin->screenatr[1]);

  delete_hface_screen( screen1 );
  delete_hface_screen( screen2 );
}

void delete_hface_screen( ScreenAtr *screen )
{
  HPpd *hppd;
  SelList *sl;
  HFace *hfc;
  HHalfedge *he;
  HFace *another_hface( HEdge *, HFace * );
  void free_hppdedge( HEdge *, HPpd * );
  void free_hppdface( HFace *, HPpd * );
  void FreeSelectList(ScreenAtr *);
  void delete_hedge_face_link( HEdge *, HFace * );

  hppd = swin->hppd;
  
  for ( sl = screen->sel_first; sl != (SelList *) NULL; sl = sl->nxt ) {

    if ( (hfc = sl->hfc) != (HFace *) NULL ) {

      /* delete edges */
      he = hfc->shhe;
      do {

	if ( another_hface( he->hed, hfc ) == (HFace *) NULL ) {
	  free_hppdedge( he->hed, hppd );
	} else {
	  delete_hedge_face_link( he->hed, hfc );
	}
	
      } while ( (he = he->nxt) != hfc->shhe );
      
      free_hppdface( hfc, hppd );

      sl->hfc = (HFace *) NULL;
    }
  }
  FreeSelectList( screen );
  
}

void delete_hedge_face_link( HEdge *hed, HFace *hfc )
{
  if ( hed->rf == hfc ) {
    hed->rf = (HFace *) NULL;
    return;
  } else {
    hed->lf = (HFace *) NULL;
    return;
  }
}
  
#define LP_START 0
#define LP_END   1

void create_hloop( HFace *hfc, HPpd *hppd )
{
  int i;
  HLoop *hloop;
  Splp *nlp, *lp;
  Spvt *sv, *vt;
  Splv  *nlv, *lv, *lprv;
  Sple  *nle;
  HHalfedge *he;
  HVertex *hvt;
  HEdge   *hed;
  Sped    *ed;
  Splp    *create_splp( void );
  int     lp_start_end( Splp *, Spvt * );
  Splv    *create_ppdloopvertex( Splp * );
  Sple    *create_ppdloopedge( Splp * );
  Sped    *find_ppdedge( Spvt *, Spvt * );
  
  hloop = hfc->hloop = (HLoop *) malloc( sizeof(HLoop) );
  
  for ( i = 0; i < 2; ++i ) {

    nlp = create_splp();
    nlp->type = HLOOP;
    
    /* initialize */
    if ( !i ) hloop->lp1 = nlp; else hloop->lp2 = nlp;
    
    he = hfc->shhe;
    do {
      
      hvt = he->vt;
      sv = ( !i ) ? hvt->vt1 : hvt->vt2;
      hed = he->hed;
      lp = ( !i ) ? hed->lp1 : hed->lp2;

      if ( lp_start_end( lp, sv ) == LP_START ) {

	lv = lp->splv;
	while ( lv != lp->eplv ) {
	  
	  /* create loopvertex */
	  nlv = create_ppdloopvertex( nlp );
	  nlv->vt = lv->vt;

	  lprv = lv;
	  lv   = lv->nxt;
	  
	  /* create loopedge */
	  ed = (Sped *) NULL;
	  if ( (ed = find_ppdedge( lprv->vt, lv->vt )) != (Sped *) NULL ) {
	    nle = create_ppdloopedge( nlp );
	    nle->ed = ed;
	  }
	}
	  
      } else {
	
	lv = lp->eplv;
	while ( lv != lp->splv ) {

	  /* create loopvertex */
	  nlv = create_ppdloopvertex( nlp );
	  nlv->vt = lv->vt;

	  lprv = lv;
	  lv   = lv->prv;
	  
	  /* create loopedge */
	  ed = (Sped *) NULL;
	  if ( (ed = find_ppdedge( lprv->vt, lv->vt )) != (Sped *) NULL ) {
	    nle = create_ppdloopedge( nlp );
	    nle->ed = ed;
	  }
	  
	}
	  
      }
    } while ( (he = he->nxt) != hfc->shhe );

  }
}
    
int lp_start_end( Splp *lp, Spvt *vt )
{
  if ( lp->splv->vt == vt ) return LP_START;
  else LP_END;
}

int check_hppdface( ScreenAtr *screen, HPpd *hppd )
{
  SelList   *sl;
  HVertex *shvt, *ehvt;
  HEdge   *hed;
  HEdge   *find_hppdedge( HVertex *, HVertex *, HPpd * );
  
  if ( hppd == (HPpd *) NULL ) return SMD_ON;
  if ( screen == (ScreenAtr *) NULL ) return SMD_ON;
  
  shvt = screen->sel_last->hvt;
  for ( sl = screen->sel_first; sl != (SelList *) NULL; sl = sl->nxt ) {
    
    ehvt = sl->hvt;
    if ( (hed = find_hppdedge( shvt, ehvt, hppd )) != (HEdge *) NULL ) {

      if ( (hed->rf != (HFace *) NULL) && (hed->lf != (HFace *) NULL) )
	return SMD_ON;
    }
    shvt = ehvt;
  }

  return SMD_OFF;
}

/* "Cancel" functions */
void cancel_create_hface( void )
{
  ScreenAtr *screen1, *screen2;
  void    FreeSelectList(ScreenAtr *);

  screen1 = &(swin->screenatr[0]);
  screen2 = &(swin->screenatr[1]);

  FreeSelectList( screen1 );
  FreeSelectList( screen2 );
}

void cancel_delete_hface( void )
{
  ScreenAtr *screen1, *screen2;
  void    FreeSelectList(ScreenAtr *);

  screen1 = &(swin->screenatr[0]);
  screen2 = &(swin->screenatr[1]);

  FreeSelectList( screen1 );
  FreeSelectList( screen2 );
}

