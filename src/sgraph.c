/* Copyright (c) 1997-1998 by Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

double sub_ratio = 0.5;

/* SGraph functions */

SGraph *create_sgraph( void )
{
  SGraph *sg;
  
  sg = (SGraph *) malloc( sizeof(SGraph) );

  sg->sgvtn  = 0;
  sg->sgvtid = 0;
  sg->sgvt  = (SGvt *) NULL;
  sg->egvt  = (SGvt *) NULL;
  sg->sgedn  = 0;
  sg->sgedid = 0;
  sg->sged  = (SGed *) NULL;
  sg->eged  = (SGed *) NULL;
  sg->sgfcn  = 0;
  sg->sgfcid = 0;
  sg->sgfc  = (SGfc *) NULL;
  sg->egfc  = (SGfc *) NULL;

  sg->src = sg->dist = (SGvt *) NULL;
  sg->lp  = (SGlp *) NULL;
  
  return sg;
}

void free_sgraph( SGraph *sg )
{
  void FreeSGVertex( SGraph * );
  void FreeSGEdge( SGraph * );
  void FreeSGFace( SGraph * );
  void free_sglp( SGlp * );
  
  if ( sg == NULL ) return;

  sg->src  = NULL;
  sg->dist = NULL;
  if ( sg->lp != (SGlp *) NULL )
    free_sglp( sg->lp );
  
  FreeSGEdge( sg );
  FreeSGFace( sg );
  FreeSGVertex( sg );

  free( sg );
}

/* SGvt functions */
SGvt *create_sgvt( SGraph *sg )
{
  SGvt *vt;

  vt = (SGvt *) malloc(sizeof(SGvt));

  vt->nxt = (SGvt *) NULL;
  if (sg->sgvt == (SGvt *) NULL) {
    vt->prv   = (SGvt *) NULL;
    sg->egvt = sg->sgvt = vt;
  } else {
    vt->prv   = sg->egvt;
    sg->egvt = sg->egvt->nxt = vt;
  }

  /* id */
  vt->no   = sg->sgvtid;
  vt->sid  = SMDNULL;
  /* type */
  vt->type = VERTEX_SPATH_ORIGINAL;

  /* vt - edge */
  vt->sgvtedn = 0;
  vt->svted = (SGvted *) NULL;
  vt->evted = (SGvted *) NULL;

  vt->spvt = (Spvt *) NULL;
  vt->sped = (Sped *) NULL;
  
  /* for shortest path */
  vt->pq_type = EMPTY;
  vt->pqc     = (SGPQCont *) NULL;
  
  ++( sg->sgvtn );
  ++( sg->sgvtid );

  return vt;
}

SGvt *CreateSGvtfromSGvt( SGraph *sg, SGvt *sgvt )
{
  SGvt *nsgvt;
  
  nsgvt = create_sgvt( sg );
  nsgvt->vec.x = sgvt->vec.x;
  nsgvt->vec.y = sgvt->vec.y;
  nsgvt->vec.z = sgvt->vec.z;
    
  nsgvt->type = sgvt->type;
  if ( nsgvt->type == VERTEX_SPATH_ORIGINAL ) {
    nsgvt->spvt = sgvt->spvt;
  } else {
    nsgvt->sped  = sgvt->sped;
  }

  return nsgvt;
}

void free_sgvt( SGvt *vt, SGraph *sg )
{
  void FreeSGVertexEdge( SGvt * );
  
  if (vt == (SGvt *) NULL) return;

  FreeSGVertexEdge( vt );
  
  if ( sg->sgvt == vt ) {
    if ( (sg->sgvt = vt->nxt) != (SGvt *) NULL )
      vt->nxt->prv = (SGvt *) NULL;
    else {
      sg->egvt = (SGvt *) NULL;
    }
  } else if (sg->egvt == vt) {
    vt->prv->nxt = (SGvt *) NULL;
    sg->egvt = vt->prv;
  } else {
    vt->prv->nxt = vt->nxt;
    vt->nxt->prv = vt->prv;
  }

  --( sg->sgvtn );
  free( vt );
}


SGvt *another_sgvt( SGed *ed, SGvt *vt )
{
  if ( ed->sv == vt ) return ed->ev;
  else return ed->sv;
}

SGvt *find_sgvt_from_spvt( SGraph *sg, Spvt *ppdvt )
{
  SGvt *vt;
  
  for ( vt = sg->sgvt; vt != (SGvt *) NULL; vt = vt->nxt ) {
    if ( vt->type == VERTEX_SPATH_ORIGINAL ) {
      if ( vt->spvt == ppdvt ) return vt;
    }
  }
  return (SGvt *) NULL;
}

SGvt *find_sgvt_from_spvt_number( SGraph *sg, int id )
{
  SGvt *vt;
  
  for ( vt = sg->sgvt; vt != (SGvt *) NULL; vt = vt->nxt ) {
    if ( vt->type == VERTEX_SPATH_ORIGINAL ) {
      if ( vt->spvt->no == id ) return vt;
    }
  }
  return (SGvt *) NULL;
}

SGvt *find_sgvt_from_sgfc( SGfc *fc, SGvt *vt )
{
  SGfcvt *fv;
  
  for ( fv = fc->sfcvt; fv != (SGfcvt *) NULL; fv = fv->nxt ) {
    if ( fv->vt == vt ) return vt;
  }
  return NULL;
}

void FreeSGVertex( SGraph *sg )
{
  SGvt *vt, *nvt;
  void free_sgvt( SGvt *, SGraph * );

  if ( sg == NULL ) return;
  
  for ( vt = sg->sgvt; vt != (SGvt *) NULL; vt = nvt ) {
    nvt = vt->nxt;
    free_sgvt( vt, sg );
  }
  sg->sgvtn = sg->sgvtid = 0;
  sg->sgvt = sg->egvt = (SGvt *) NULL;
}

/* SGvted functions */
SGvted *create_sgvted( SGvt *vt, SGed *ed )
{
  SGvted *ve;

  ve = (SGvted *) malloc( sizeof(SGvted) );

  ve->nxt = (SGvted *) NULL;
  if (vt->svted == (SGvted *) NULL) {
    ve->prv   = (SGvted *) NULL;
    vt->evted = vt->svted = ve;
  } else {
    ve->prv   = vt->evted;
    vt->evted = vt->evted->nxt = ve;
  }

  ve->ed = ed;
  
  ++( vt->sgvtedn );

  return ve;
}

void free_sgvted( SGvted *ve, SGvt *vt )
{
  if ( vt == (SGvt *) NULL ) return;
  if ( ve == (SGvted *) NULL ) return;

  if ( vt->svted == ve ) {
    if ( (vt->svted = ve->nxt) != (SGvted *) NULL )
      ve->nxt->prv = (SGvted *) NULL;
    else {
      vt->evted = (SGvted *) NULL;
    }
  } else if (vt->evted == ve) {
    ve->prv->nxt = (SGvted *) NULL;
    vt->evted = ve->prv;
  } else {
    ve->prv->nxt = ve->nxt;
    ve->nxt->prv = ve->prv;
  }

  --( vt->sgvtedn );
  free( ve );
}

void FreeSGvted( SGvt *vt, SGed *ed )
{
  SGvted *ve;
  void free_sgvted( SGvted *, SGvt * );

  if ( vt == NULL ) return;
  
  for ( ve = vt->svted; ve != (SGvted *) NULL; ve = ve->nxt ) {
    if ( ve->ed == ed ) {
      free_sgvted( ve, vt );
      return;
    }
  }
}

void FreeSGVertexEdge( SGvt *vt )
{
  SGvted *ve, *nve;
  void free_sgvted( SGvted *, SGvt * );

  if ( vt == NULL ) return;
  
  for ( ve = vt->svted; ve != (SGvted *) NULL; ve = nve ) {
    nve = ve->nxt;
    free_sgvted( ve, vt );
  }
  vt->sgvtedn = 0;
  vt->svted  = vt->evted = (SGvted *) NULL;
}
    
/* SGed functions */
SGed *create_sged( SGraph *sg )
{
  SGed *ed;

  ed = (SGed *) malloc(sizeof(SGed));

  ed->nxt = (SGed *) NULL;
  if (sg->sged == (SGed *) NULL) {
    ed->prv   = (SGed *) NULL;
    sg->eged = sg->sged = ed;
  } else {
    ed->prv   = sg->eged;
    sg->eged = sg->eged->nxt = ed;
  }

  /* id */
  ed->no   = sg->sgedid;
  ed->sid  = SMDNULL;
  /* type */
  ed->type = EDGE_SPATH_ORIGINAL;

  ed->sv = ed->ev = (SGvt *) NULL;
  ed->lf = ed->rf = (SGfc *) NULL;
  
  ed->sped = (Sped *) NULL;
  ed->spfc = (Spfc *) NULL;
  
  ++( sg->sgedn );
  ++( sg->sgedid );

  return ed;
}

SGed *CreateSGed( SGraph *sg, SGvt *sv, SGvt *ev, Sped *ed )
{
  SGed   *sged;
  double V3DistanceBetween2Points( Vec *, Vec * );
  SGvted *create_sgvted( SGvt *, SGed * );
  SGed   *create_sged( SGraph * );
  
  sged = create_sged( sg );
  sged->sped = ed;
  
  sged->sv = sv;
  sged->ev = ev;
  (void) create_sgvted( sv, sged );
  (void) create_sgvted( ev, sged );
/*   sged->length = V3DistanceBetween2Points( &(sged->sv->vec), &(sged->ev->vec) ); */
  
  return sged;
}

SGed *CreateSGedfromSGed( SGraph *sg, SGvt *sv, SGvt *ev, SGed *sged )
{
  SGed *nsged;
  SGvted *create_sgvted( SGvt *, SGed * );
  SGed *CreateSGed( SGraph *, SGvt *, SGvt *, Sped * );

  nsged = CreateSGed( sg, sv, ev, sged->sped );
  nsged->type = sged->type;
  if ( nsged->type == EDGE_SPATH_ONFACE ) nsged->spfc = sged->spfc;

  return nsged;
}

void free_sged( SGed *ed, SGraph *sg )
{
  if ( ed == (SGed *) NULL ) return;

  if ( sg->sged == ed ) {
    if ( (sg->sged = ed->nxt) != (SGed *) NULL )
      ed->nxt->prv = (SGed *) NULL;
    else {
      sg->eged = (SGed *) NULL;
    }
  } else if (sg->eged == ed) {
    ed->prv->nxt = (SGed *) NULL;
    sg->eged = ed->prv;
  } else {
    ed->prv->nxt = ed->nxt;
    ed->nxt->prv = ed->prv;
  }

  --( sg->sgedn );
  free( ed );
}

void FreeSGed( SGed *ed, SGraph *sg )
{
  void FreeSGvted( SGvt *, SGed * );
  void free_sged( SGed *, SGraph * );

  if ( ed == NULL ) return;
  FreeSGvted( ed->sv, ed );
  FreeSGvted( ed->ev, ed );
  free_sged( ed, sg );
}
  
void FreeSGEdge( SGraph *sg )
{
  SGed *ed, *ned;
  void free_sged( SGed *, SGraph * );

  if ( sg == NULL ) return;
  
  for ( ed = sg->sged; ed != (SGed *) NULL; ed = ned ) {
    ned = ed->nxt;
    free_sged( ed, sg );
  }
  sg->sgedn = sg->sgedid = 0;
  sg->sged = sg->eged = (SGed *) NULL;
}

SGed *find_sged( SGvt *sv, SGvt *ev )
{
  SGvted *ve;
  SGed   *ed;
  
  for ( ve = sv->svted; ve != (SGvted *) NULL; ve = ve->nxt ) {
    ed = ve->ed;
    if ( (ed->sv == sv) && (ed->ev == ev) ) {
      return ed;
    }
    if ( (ed->ev == sv) && (ed->sv == ev) ) {
      return ed;
    }
  }
  
  return (SGed *) NULL;
}
    
      
/* SGfc functions */
SGfc *create_sgfc( SGraph *sg )
{
  SGfc *fc;

  fc = (SGfc *) malloc(sizeof(SGfc));

  fc->nxt = (SGfc *) NULL;
  if (sg->sgfc == (SGfc *) NULL) {
    fc->prv   = (SGfc *) NULL;
    sg->egfc = sg->sgfc = fc;
  } else {
    fc->prv   = sg->egfc;
    sg->egfc = sg->egfc->nxt = fc;
  }

  /* id */
  fc->no   = sg->sgfcid; 
  fc->sid  = SMDNULL;

  /* fc - edge */
  fc->sgfcvtn = 0;
  fc->sfcvt = (SGfcvt *) NULL;
  fc->efcvt = (SGfcvt *) NULL;

  fc->spfc = (Spfc *) NULL;
  
  ++( sg->sgfcn );
  ++( sg->sgfcid );

  return fc;
}

void free_sgfc( SGfc *fc, SGraph *sg )
{
  void FreeSGFaceVertex( SGfc * );
  
  if (fc == (SGfc *) NULL) return;

  FreeSGFaceVertex( fc );
  
  if ( sg->sgfc == fc ) {
    if ( (sg->sgfc = fc->nxt) != (SGfc *) NULL )
      fc->nxt->prv = (SGfc *) NULL;
    else {
      sg->egfc = (SGfc *) NULL;
    }
  } else if (sg->egfc == fc) {
    fc->prv->nxt = (SGfc *) NULL;
    sg->egfc = fc->prv;
  } else {
    fc->prv->nxt = fc->nxt;
    fc->nxt->prv = fc->prv;
  }

  --( sg->sgfcn );
  free( fc );
}

void FreeSGFace( SGraph *sg )
{
  SGfc *fc, *nfc;
  void free_sgfc( SGfc *, SGraph * );

  if ( sg == NULL ) return;
  
  for ( fc = sg->sgfc; fc != (SGfc *) NULL; fc = nfc ) {
    nfc = fc->nxt;
    free_sgfc( fc, sg );
  }
  sg->sgfcn = sg->sgfcid = 0;
  sg->sgfc  = sg->egfc = (SGfc *) NULL;
}

/* SGvted functions */
SGfcvt *create_sgfcvt( SGfc *fc, SGvt *vt )
{
  SGfcvt *fv;

  fv = (SGfcvt *) malloc( sizeof(SGfcvt) );

  fv->nxt = (SGfcvt *) NULL;
  if (fc->sfcvt == (SGfcvt *) NULL) {
    fv->prv   = (SGfcvt *) NULL;
    fc->efcvt = fc->sfcvt = fv;
  } else {
    fv->prv   = fc->efcvt;
    fc->efcvt = fc->efcvt->nxt = fv;
  }

  fv->vt = vt;
  
  ++( fc->sgfcvtn );

  return fv;
}

void free_sgfcvt( SGfcvt *fv, SGfc *fc )
{
  if ( fc == NULL ) return;
  if ( fv == (SGfcvt *) NULL ) return;

  if ( fc->sfcvt == fv ) {
    if ( (fc->sfcvt = fv->nxt) != (SGfcvt *) NULL )
      fv->nxt->prv = (SGfcvt *) NULL;
    else {
      fc->efcvt = (SGfcvt *) NULL;
    }
  } else if ( fc->efcvt == fv ) {
    fv->prv->nxt = (SGfcvt *) NULL;
    fc->efcvt = fv->prv;
  } else {
    fv->prv->nxt = fv->nxt;
    fv->nxt->prv = fv->prv;
  }

  --( fc->sgfcvtn );
  free( fv );
}

void FreeSGFaceVertex( SGfc *fc )
{
  SGfcvt *fv, *nfv;
  void free_sgfcvt( SGfcvt *, SGfc * );

  if ( fc == NULL ) return;
  
  for ( fv = fc->sfcvt; fv != (SGfcvt *) NULL; fv = nfv ) {
    nfv = fv->nxt;
    free_sgfcvt( fv, fc );
  }
  fc->sgfcvtn = 0;
  fc->sfcvt  = fc->efcvt = (SGfcvt *) NULL;
}

/* SGvt functions */
SGlp *create_sglp( void )
{
  SGlp *lp;

  lp = (SGlp *) malloc( sizeof(SGlp) );

  /* lp - vt */
  lp->sglpvtn = 0;
  lp->slpvt = (SGlpvt *) NULL;
  lp->elpvt = (SGlpvt *) NULL;
  
  /* lp - vt */
  lp->sglpedn = 0;
  lp->slped = (SGlped *) NULL;
  lp->elped = (SGlped *) NULL;

  lp->length = 0.0;
  
  return lp;
}

void free_sglp( SGlp *lp )
{
  void FreeSGLoopEdge( SGlp * );
  void FreeSGLoopVertex( SGlp * );
  
  if ( lp == NULL ) return;
  
  FreeSGLoopEdge( lp );
  FreeSGLoopVertex( lp );

  free( lp );
}

/* SGlpvt functions */
SGlpvt *create_sglpvt( SGlp *lp, SGvt *vt )
{
  SGlpvt *lv;

  lv = (SGlpvt *) malloc( sizeof(SGlpvt) );

  lv->nxt = (SGlpvt *) NULL;
  if (lp->slpvt == (SGlpvt *) NULL) {
    lv->prv   = (SGlpvt *) NULL;
    lp->elpvt = lp->slpvt = lv;
  } else {
    lv->prv   = lp->elpvt;
    lp->elpvt = lp->elpvt->nxt = lv;
  }

  lv->vt = vt;
  
  ++( lp->sglpvtn );

  return lv;
}

void free_sglpvt( SGlpvt *lv, SGlp *lp )
{
  if (lv == (SGlpvt *) NULL) return;

  if ( lp->slpvt == lv ) {
    if ( (lp->slpvt = lv->nxt) != (SGlpvt *) NULL )
      lv->nxt->prv = (SGlpvt *) NULL;
    else {
      lp->elpvt = (SGlpvt *) NULL;
    }
  } else if (lp->elpvt == lv) {
    lv->prv->nxt = (SGlpvt *) NULL;
    lp->elpvt = lv->prv;
  } else {
    lv->prv->nxt = lv->nxt;
    lv->nxt->prv = lv->prv;
  }

  --( lp->sglpvtn );
  free( lv );
}

void FreeSGLoopVertex( SGlp *lp )
{
  SGlpvt *lv, *nlv;
  void free_sglpvt( SGlpvt *, SGlp * );

  if ( lp == NULL ) return;
  
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = nlv ) {
    nlv = lv->nxt;
    free_sglpvt( lv, lp );
  }
  lp->sglpvtn = 0;
  lp->slpvt  = lp->elpvt = (SGlpvt *) NULL;
}
    
/* SGlped functions */
SGlped *create_sglped( SGlp *lp, SGed *ed )
{
  SGlped *le;

  le = (SGlped *) malloc( sizeof(SGlped) );

  le->nxt = (SGlped *) NULL;
  if (lp->slped == (SGlped *) NULL) {
    le->prv   = (SGlped *) NULL;
    lp->elped = lp->slped = le;
  } else {
    le->prv   = lp->elped;
    lp->elped = lp->elped->nxt = le;
  }

  le->ed = ed;
  
  ++( lp->sglpedn );

  return le;
}

void free_sglped( SGlped *le, SGlp *lp )
{
  if (le == (SGlped *) NULL) return;

  if ( lp->slped == le ) {
    if ( (lp->slped = le->nxt) != (SGlped *) NULL )
      le->nxt->prv = (SGlped *) NULL;
    else {
      lp->elped = (SGlped *) NULL;
    }
  } else if (lp->elped == le) {
    le->prv->nxt = (SGlped *) NULL;
    lp->elped = le->prv;
  } else {
    le->prv->nxt = le->nxt;
    le->nxt->prv = le->prv;
  }

  --( lp->sglpedn );
  free( le );
}

void FreeSGLoopEdge( SGlp *lp )
{
  SGlped *le, *nle;
  void free_sglped( SGlped *, SGlp * );
  
  if ( lp == NULL ) return;
  
  for ( le = lp->slped; le != (SGlped *) NULL; le = nle ) {
    nle = le->nxt;
    free_sglped( le, lp );
  }
  lp->sglpedn = 0;
  lp->slped  = lp->elped = (SGlped *) NULL;
}

/* create sgraph from ppd */

SGraph *initialize_sgraph( Sppd *ppd )
{
  int    i;
  SGraph *sg;
  Spvt   *vt;
  Spfc   *fc;
  Sped   *ed;
  Sphe   *he;
  
  SGvt   *sgvt, **tmp_sgvt, *sv, *ev;
  SGfc   *sgfc, **tmp_sgfc;
  SGed   *sged;
  SGraph *create_sgraph( void );
  SGvt   *create_sgvt( SGraph * );
  SGfc   *create_sgfc( SGraph * );
  SGfcvt *create_sgfcvt( SGfc *, SGvt * );
  SGed   *CreateSGed( SGraph *, SGvt *, SGvt *, Sped * );
  void   add_vertices_edges_sgraph( SGraph * );
  double V3DistanceBetween2Points( Vec *, Vec * );

  
  sg = create_sgraph();

/*   display("SGraph:\n"); */
  /* vertices */
  tmp_sgvt   = (SGvt **) malloc( ppd->vn * sizeof(SGvt *) );
  for ( i = 0, vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt, ++i ) {
    vt->sid = i;
    sgvt = create_sgvt( sg );
    sgvt->spvt = vt;
    sgvt->type = VERTEX_SPATH_ORIGINAL;
    sgvt->vec.x = vt->vec.x;
    sgvt->vec.y = vt->vec.y;
    sgvt->vec.z = vt->vec.z;
    tmp_sgvt[i] = sgvt;
  }
  /*   display("\tVertex:\t%d\n", sg->sgvtn ); */

  /* faces */
  tmp_sgfc = (SGfc **) malloc( ppd->fn * sizeof(SGfc *) );
  for ( i = 0, fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt, ++i ) {
    fc->sid = i;
    sgfc = create_sgfc( sg );
    sgfc->spfc = fc;
    he = fc->sphe;
    do {
      vt   = he->vt;
      sgvt = tmp_sgvt[ vt->sid ];
      (void) create_sgfcvt( sgfc, sgvt );
    } while ( (he = he->nxt) != fc->sphe );
    tmp_sgfc[i] = sgfc;
  }
/*   display("\tFace:\t%d\n", sg->sgfcn ); */

  /* edges */
  for ( ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt ) {
    sv = tmp_sgvt[ ed->sv->sid ];
    ev = tmp_sgvt[ ed->ev->sid ];
    sged = CreateSGed( sg, sv, ev, ed );
    sged->type = EDGE_SPATH_ORIGINAL;
    ed->length = V3DistanceBetween2Points( &(sged->sv->vec), &(sged->ev->vec) );
    sged->length = ed->length;
    if ( ed->lf != NULL ) sged->lf = tmp_sgfc[ ed->lf->sid ];
    if ( ed->rf != NULL ) sged->rf = tmp_sgfc[ ed->rf->sid ];
/*     display("Edge %d ppdsv %d ppdiv %d length %g\n",sged->no, */
/* 	    ed->sv->sid, ed->ev->sid, sged->length ); */
  }
/*   display("\tEdge:\t%d\n", sg->sgedn ); */

  free( tmp_sgvt );
  free( tmp_sgfc );

  sub_ratio = 0.5;
  add_vertices_edges_sgraph( sg );
  return sg;
}
    
/*
  STEP1:
  add vertices and edges in G
*/

#define NUMMIDPOINT 1

void add_vertices_edges_sgraph( SGraph *sg )
{
  int     i, j, n;
  int     vcnt, escnt, eocnt, edcnt;
  int     num_midpoint, num_midpoint1;
  SGvt    *sgvt, *sv, *ev, *sgvt1, *sgvt2, **mid_sgvt;
  SGed    *sged, *next_sged, *sub_sged;
  SGfcvt  *sgfv1, *sgfv2;
  SGfc    *sgfc;
  SGvted  *ve;
  Vec     *svec, *evec;
  double  s, t;
  Boolean isOnDifferentEdges( SGvt *, SGvt * );
  SGvt    *create_sgvt( SGraph * );
  SGed    *find_sged( SGvt *, SGvt * );
  SGed    *CreateSGed( SGraph *, SGvt *, SGvt *, Sped * );
  SGfcvt  *create_sgfcvt( SGfc *, SGvt * );
  void    FreeSGed( SGed *, SGraph * );
  double  V3DistanceBetween2Points( Vec *, Vec * );

  vcnt = escnt = eocnt = edcnt = 0;
  
  /*
    STEP1-1:
    add n vertices on the edge of M
  */

/*   display("STEP1: add Vertices and Edges.\n"); */
  /* add vertices */
  num_midpoint  = NUMMIDPOINT;
  num_midpoint1 = num_midpoint+1;
  mid_sgvt = (SGvt **) malloc( num_midpoint * sg->sgedn * sizeof(SGvt *) );
  for ( j = 0, sged = sg->sged; sged != NULL; sged = sged->nxt, ++j ) {
    
    sged->sid = j;
    svec = &(sged->sv->vec);
    evec = &(sged->ev->vec);

    if ( sged->type == EDGE_SPATH_ORIGINAL ) {
      for ( i = 0; i < num_midpoint; ++i ) {
	
	sgvt = create_sgvt( sg ); ++vcnt;
	
	s = (double) (i + 1) / (double) num_midpoint1;
	t = 1 - s;
	sgvt->vec.x = t * svec->x + s * evec->x;
	sgvt->vec.y = t * svec->y + s * evec->y;
	sgvt->vec.z = t * svec->z + s * evec->z;
	
	sgvt->type = VERTEX_SPATH_ADDED;
	sgvt->sped = sged->sped;
	
	mid_sgvt[ num_midpoint * sged->sid + i ] = sgvt;

	if ( sged->lf != NULL ) create_sgfcvt( sged->lf, sgvt );
	if ( sged->rf != NULL ) create_sgfcvt( sged->rf, sgvt );
      }
    }
    
  }
/*   display("\tVertex added:\t%d\n", vcnt); */

  /*
    STEP1-2: condition 2
  */

  n = sg->sgedn; sged = sg->sged;
  j = 0;
  while ( j < n ) {
    
    next_sged = sged->nxt;
    
    if ( sged->type != EDGE_SPATH_ORIGINAL ) {
      sged = next_sged;
      ++j;
      continue;
    }
    
    /* create subdivide edges */
    sv = sged->sv;
    for ( i = 0; i < num_midpoint; ++i ) {
      ev = mid_sgvt[ num_midpoint * sged->sid + i ];

      sub_sged = CreateSGed( sg, sv, ev, sged->sped ); ++escnt;
      sub_sged->type = EDGE_SPATH_ORIGINAL;
      sub_sged->lf = sged->lf;
      sub_sged->rf = sged->rf;
      sub_sged->length = sged->sped->length * sub_ratio;
      
      sv = ev;
    }
    ev = sged->ev;
    sub_sged = CreateSGed( sg, sv, ev, sged->sped ); ++escnt;
    sub_sged->type = EDGE_SPATH_ORIGINAL;
    sub_sged->lf = sged->lf;
    sub_sged->rf = sged->rf;
    sub_sged->length = sged->sped->length * sub_ratio;
      
    /* delete an original edge */
    FreeSGed( sged, sg ); ++edcnt;

    ++j;
    sged = next_sged;
    
  }
     
  /*
    STEP1-2: condition 1
  */

  for ( sgfc = sg->sgfc; sgfc != (SGfc *) NULL; sgfc = sgfc->nxt ) {
    
    for ( sgfv1 = sgfc->sfcvt; sgfv1 != (SGfcvt *) NULL; sgfv1 = sgfv1->nxt ) {
      sgvt1 = sgfv1->vt;
      for ( sgfv2 = sgfv1->nxt; sgfv2 != (SGfcvt *) NULL; sgfv2 = sgfv2->nxt ) {
	sgvt2 = sgfv2->vt;

	if ( ( isOnDifferentEdges( sgvt1, sgvt2 ) == True ) &&
	     ( find_sged( sgvt1, sgvt2 ) == NULL ) ) {
	  sged = CreateSGed( sg, sgvt1, sgvt2, NULL ); ++eocnt;
	  sged->type = EDGE_SPATH_ONFACE;
	  sged->spfc = sgfc->spfc;
	  sged->length = V3DistanceBetween2Points( &(sged->sv->vec), &(sged->ev->vec) );
	  
	}
      }
    }
  }

/*   display("\tEdges added:\n"); */
/*   display("\tsubdivide\t%d\n", escnt); */
/*   display("\tonface   \t%d\n", eocnt); */
/*   display("\tEdges delated:\t%d\n", edcnt); */
/*   display("\tTotal:\t%d\n", sg->sgedn ); */
	  
  free( mid_sgvt );
}

Boolean isOnDifferentEdges( SGvt *vt1, SGvt *vt2 )
{
  Sped *ed;
  Spvt *vt;
  
  if ( (vt1->type == VERTEX_SPATH_ADDED) &&
       (vt2->type == VERTEX_SPATH_ADDED) ) {
    if ( vt1->sped != vt2->sped ) return True;
  } else if ( (vt1->type == VERTEX_SPATH_ADDED) &&
	      (vt2->type == VERTEX_SPATH_ORIGINAL) ) {
    ed = vt1->sped;
    vt = vt2->spvt;
    if ( (ed->sv != vt) && (ed->ev != vt) ) return True;
  } else if ( (vt1->type == VERTEX_SPATH_ORIGINAL) &&
	      (vt2->type == VERTEX_SPATH_ADDED) ) {
    ed = vt2->sped;
    vt = vt1->spvt;
    if ( (ed->sv != vt) && (ed->ev != vt) ) return True;
  }
  return False;
}

/*
  STEP2:
  calculating a shortest-path using Dijkstra's algorithm
*/

/* Dijkstra's algorithm using SGraph */
SGlp *ShortestPath_Dijkstra_SGraph( SGraph *sg, SGvt *src, SGvt *dist )
{
  int      i, j;
  int      num1, num2;
  SGvt     *vt, *avt, *min_tvt;
  SGed     *ed;
  SGlp     *lp;
  SGPQHeap *pqh;
  SGPQCont *pqc;
  double   min_dis, newlen;
  SGvted   *vted;
  SGVtList *vtlist;
  SGPQHeap *create_sgpqheap( int );
  void     insert_sgpqcont( double, SGPQHeap *, SGvt *, SGvt * );
  void     deletemin_sgpqcont( SGPQHeap * );
  void     adjust_sgpqheap( Id, SGPQHeap * );
  SGvt     *another_sgvt( SGed *, SGvt * );
  SGlp     *sgvtlist_to_sglp( SGVtList *, SGvt *, SGvt * );
  int      sg_stepnumber_to_src( SGVtList *, SGvt *, SGvt * );
  void     free_sgpqheap ( SGPQHeap * );
  
  /* Spvt の初期化 */

  for ( i = 0, vt = sg->sgvt; vt != (SGvt *) NULL; vt = vt->nxt, ++i ) {
    vt->sid = i;
    vt->pqc = (SGPQCont *) NULL;
    vt->pq_type = EMPTY;
    if ( vt == src )  vt->pq_type = DELETED;
    
  }

  /* initialize priority queue */
  pqh    = create_sgpqheap( sg->sgvtn );
  vtlist = (SGVtList *) malloc( sg->sgvtn * sizeof(SGVtList) );
  vtlist[src->sid].prv = src;
  vtlist[src->sid].dis = 0.0;

  for ( vted = src->svted; vted != (SGvted *) NULL; vted = vted->nxt ) {
    avt = another_sgvt( vted->ed, src );
/*     if ( avt->no == 58 ) { */
/*       display("(first) inserted. %d\n", avt->no ); */
/*     } */
    insert_sgpqcont( vted->ed->length, pqh, avt, src );
  }
  
  /* vtlist: S から 1 つのエッジで辿れるものしか値が入らない */
  
  for ( i = 0; i < sg->sgvtn - 1; ++i ) {
    /* min_tvt S に加える */
    min_dis = pqh->pqcont[0].distance;
    min_tvt = pqh->pqcont[0].tgt_vt;
/*     if ( min_tvt->no == 58 ) { */
/*       display("num %d min_tvt %d\n", pqh->last, min_tvt->no ); */
/*       for ( j = 0; j < pqh->last; ++j ) { */
/* 	display("%d vt %d\n", j,  pqh->pqcont[j].tgt_vt->no ); */
/*       } */
/*     } */
    vtlist[ min_tvt->sid ].prv = pqh->pqcont[0].prv_vt;
    vtlist[ min_tvt->sid ].dis = min_dis;

    /* min_tvt をヒープから消去 */
    deletemin_sgpqcont( pqh );
    
    for ( vted = min_tvt->svted; vted != (SGvted *) NULL; vted = vted->nxt ) {

      avt    = another_sgvt( vted->ed, min_tvt );
      newlen = min_dis + vted->ed->length;
      pqc    = avt->pqc;
/*       display("\ted %d min_tvt %d avt %d\n", vted->ed->no, min_tvt->no, avt->no ); */

      if ( (pqc == (SGPQCont *) NULL) && (avt->pq_type == EMPTY) ) {
	
/* 	if ( avt->no == 58 ) { */
/* 	  display("inserted. %d\n", avt->no ); */
/* 	} */
	insert_sgpqcont( newlen, pqh, avt, min_tvt );
	
      } else if ( pqc != (SGPQCont *) NULL ) {

	/* updated. 97/08/28. modified from original dijkstra's algorithm. */
	if ( fabs( newlen - pqc->distance ) < SMDZEROEPS ) {

/* 	  num1 = sg_stepnumber_to_src( (SGVtList *) vtlist, pqc->prv_vt, src ); */
/* 	  num2 = sg_stepnumber_to_src( (SGVtList *) vtlist, min_tvt, src ); */

	  if ( num2 < num1 ) {

	    pqc->distance = newlen;
	    pqc->prv_vt   = min_tvt;
	  
	    adjust_sgpqheap( pqc->id, pqh );
	  }
	  
	} else {
	  
	  if ( newlen < pqc->distance ) {

	    pqc->distance = newlen;
	    pqc->prv_vt   = min_tvt;
	  
	    adjust_sgpqheap( pqc->id, pqh );

	  }
	}
	
      }
    }
  }

  /* VtList to Loop */
  lp = sgvtlist_to_sglp( (SGVtList *) vtlist, src, dist );

/*   display("STEP2: calculating shortest path.\n"); */
/*   display("\tVertex %d\n", lp->sglpvtn ); */
/*   display("\tEdge   %d\n", lp->sglpedn ); */
/*   display("\tLength %g\n", lp->length ); */
  
  free_sgpqheap( pqh );
  free( vtlist );

  return lp;
}

int sg_stepnumber_to_src( SGVtList *vtlist, SGvt *start, SGvt *goal )
{
  int cnt = 0;
  SGvt *vt;

  vt = start; 

  while ( vt != goal ) {
    ++cnt;
    vt = vtlist[vt->sid].prv;
  }
    
  return cnt;
}

/* for testing */
SGlp *sgvtlist_to_sglp( SGVtList *vtlist, SGvt *src, SGvt *dist )
{
  SGvt *sgvt, *sv, *ev;
  SGlpvt *lv;
/*   SGlped *le; */
  SGlp *lp, *nlp;
  SGed *ed;
  SGlp *create_sglp( void );
  void free_sglp( SGlp * );
  SGlp *reverse_sglp( SGlp * );
  SGed *find_sged( SGvt *, SGvt * );
  SGlpvt *create_sglpvt( SGlp *, SGvt * );
/*   SGlped *create_sglped( SGlp *, SGed * ); */

  if ( vtlist == (SGVtList *) NULL ) return (SGlp *) NULL;

  lp = create_sglp();

  /* create loop vertices */
  /* lp includes src and dist. */
/*   display("\t\tbegin. dist %d src %d\n", dist->no, src->no); */
  sgvt = dist;
  while ( sgvt != src ) {
/*     display("\t\tsgvt no %d sid %d\n", sgvt->no, sgvt->sid ); */
    lv = create_sglpvt( lp, sgvt );
    sgvt = vtlist[ sgvt->sid ].prv;
  }
/*   display("\t\tsgvt no %d sid %d\n", sgvt->no, sgvt->sid ); */

  /* include a last vertex(src) */
  lv = create_sglpvt( lp, sgvt );
/*   display("\t\tend.\n"); */
  
  /* create loop edges */
  lp->length = 0.0;
  for ( lv = lp->slpvt; lv->nxt != (SGlpvt *) NULL; lv = lv->nxt ) {
    sv = lv->vt;
    ev = lv->nxt->vt;
    ed = find_sged( sv, ev );
    lp->length += ed->length;
  }
  
  /* (dist -> src) -->> (src -> dist) */
/*   nlp = reverse_sglp( lp ); */
/*   free_sglp( lp ); */
  
  return lp;
}

/*
  STEP3:
  create graph G' from SGlp
*/

SGraph *sglp_to_sgraph( SGlp *lp, SGraph *sg )
{
  int     i, n;
  SGlpvt  *lv;
  SGraph  *nsg;
  SGvt    *sgvt, *egvt, *new_sv, *new_ev, *sv, *ev, *next_sgvt;
  SGed    *sged, *new_sged;
  SGfc    *sgfc;
  SGvted  *ve;
  SGlp    *nlp;
  Boolean *sgvtsearch, *sgedsearch, *sgfcsearch;
  SGvt    **new_sgvt;
  SGfc    **new_sgfc;
  SGraph  *create_sgraph( void );
  SGvt    *CreateSGvtfromSGvt( SGraph *, SGvt * );
  SGed    *CreateSGedfromSGed( SGraph *, SGvt *, SGvt *, SGed * );
  SGvt    *find_sgvt_from_sgfc( SGfc *, SGvt * );
  SGlp    *create_sglp( void );
  SGlpvt  *create_sglpvt( SGlp *, SGvt * );
  void    free_sgvt( SGvt *, SGraph * );
  void    FreeSGed( SGed *, SGraph * );

  /* initialize search functions */
  new_sgvt   = (SGvt **) malloc( sg->sgvtn * sizeof(SGvt *) );
  sgvtsearch = (Boolean *) malloc( sg->sgvtn * sizeof(Boolean) );
  for ( i = 0, sgvt = sg->sgvt; i < sg->sgvtn; ++i, sgvt = sgvt->nxt ) {
    sgvt->sid = i;
    sgvtsearch[i] = False;
    new_sgvt[i] = NULL;
  }
  sgedsearch = (Boolean *) malloc( sg->sgedn * sizeof(Boolean) );
  for ( i = 0, sged = sg->sged; i < sg->sgedn; ++i, sged = sged->nxt ) {
    sged->sid = i;
    sgedsearch[i] = False;
  }
  sgfcsearch = (Boolean *) malloc( sg->sgfcn * sizeof(Boolean) );
  new_sgfc   = (SGfc **) malloc( sg->sgfcn * sizeof(SGfc *) );
  for ( i = 0, sgfc = sg->sgfc; i < sg->sgfcn; ++i, sgfc = sgfc->nxt ) {
    sgfc->sid = i;
    sgfcsearch[i] = False;
    new_sgfc[i] = NULL;
  }
  
  nsg = create_sgraph();
  nlp = create_sglp();

  /* STEP3-1:
    create vertices in G'
  */
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = lv->nxt ) {
    sgvt  = lv->vt;
    new_sv = CreateSGvtfromSGvt( nsg, sgvt );
    sgvtsearch[ sgvt->sid ] = True;
    new_sgvt[ sgvt->sid ] = new_sv;
    (void) create_sglpvt( nlp, new_sv );
  }
  
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = lv->nxt ) {
    /* create sgvt from loop vertices */
    sgvt  = lv->vt;
    new_sv = new_sgvt[ sgvt->sid ];

    /* create neighbor vertices */
    for ( ve = sgvt->svted; ve != (SGvted *) NULL; ve = ve->nxt ) {
      sged = ve->ed;
      if ( sged->type == EDGE_SPATH_ORIGINAL ) {
	egvt = another_sgvt( sged, sgvt );
	if ( sgvtsearch[ egvt->sid ] == False ) {
	  new_ev = CreateSGvtfromSGvt( nsg, egvt );
	  sgvtsearch[ egvt->sid ] = True;
	  new_sgvt[ egvt->sid ]   = new_ev;
	} else {
	  new_ev = new_sgvt[ egvt->sid ];
	}
	
	if ( sged->lf != (SGfc *) NULL ) {
	  if ( sgfcsearch[ sged->lf->sid ] == False ) {
	    sgfc = create_sgfc( nsg );
	    sgfc->spfc = sged->lf->spfc;
	    sgfcsearch[ sged->lf->sid ] = True;
	    new_sgfc[ sged->lf->sid ] = sgfc;
	  } else {
	    sgfc = new_sgfc[ sged->lf->sid ];
	  }
	  if ( find_sgvt_from_sgfc( sgfc, new_sv ) == NULL )
	    create_sgfcvt( sgfc, new_sv );
	  if ( find_sgvt_from_sgfc( sgfc, new_ev ) == NULL )
	    create_sgfcvt( sgfc, new_ev );
	}
	
	if ( sged->rf != (SGfc *) NULL ) {
	  if ( sgfcsearch[ sged->rf->sid ] == False ) {
	    sgfc = create_sgfc( nsg );
	    sgfc->spfc = sged->rf->spfc;
	    sgfcsearch[ sged->rf->sid ] = True;
	    new_sgfc[ sged->rf->sid ] = sgfc;
	  } else {
	    sgfc = new_sgfc[ sged->rf->sid ];
	  }
	  if ( find_sgvt_from_sgfc( sgfc, new_sv ) == NULL )
	    create_sgfcvt( sgfc, new_sv );
	  if ( find_sgvt_from_sgfc( sgfc, new_ev ) == NULL )
	    create_sgfcvt( sgfc, new_ev );
	}
      }
    }
  }

  /* STEP3-2:
     create edges in G'
  */

  sgvt = sg->sgvt; n = sg->sgvtn; i = 0;
  while ( i < n ) {

    if ( sgvtsearch[ sgvt->sid ] == True ) {
      /* create neighbor vertices */
      for ( ve = sgvt->svted; ve != (SGvted *) NULL; ve = ve->nxt ) {
	sged = ve->ed;
	sv = sged->sv;
	ev = sged->ev;
	if ( (sgvtsearch[ sv->sid ] == True) &&
	     (sgvtsearch[ ev->sid ] == True) &&
	     (sgedsearch[ sged->sid ] == False) ) {
	  new_sv = new_sgvt[ sv->sid ];
	  new_ev = new_sgvt[ ev->sid ];
	  new_sged = CreateSGedfromSGed( nsg, new_sv, new_ev, sged );
	  new_sged->length = sged->length;
	  if ( sged->lf != NULL ) new_sged->lf = new_sgfc[ sged->lf->sid ];
	  if ( sged->rf != NULL ) new_sged->rf = new_sgfc[ sged->rf->sid ];
	}
	sgedsearch[ sged->sid ] = True;
      }
    }
    sgvt = sgvt->nxt; ++i; 
 }

  /* STEP3-3:
     delete vertices and edges in G'
  */

  sgvt = nsg->sgvt; n = nsg->sgvtn;
  i = 0;
  while ( i < n ) {
    next_sgvt = sgvt->nxt;

    if ( sgvt->sgvtedn < 2 ) {
      if ( sgvt->svted != NULL ) {
	sged = sgvt->svted->ed;
	FreeSGed( sged, nsg );
      }
      display("Vertex %d deleted.\n", sgvt->no);
      free_sgvt( sgvt, nsg );
    }

    sgvt = next_sgvt;
    ++i;
  }

  free( sgvtsearch );
  free( sgedsearch );
  free( sgfcsearch );
  free( new_sgvt );
  free( new_sgfc );

  nlp->length = lp->length;
  nsg->lp = nlp;
  nsg->src  = nlp->elpvt->vt;
  nsg->dist = nlp->slpvt->vt;

/*   display("old: \n"); */
/*   display("\tsrc  %d - %d\n", sg->src->sid, sg->src->spvt->sid ); */
/*   display("\tdist %d - %d\n", sg->dist->sid, sg->dist->spvt->sid ); */
/*   display("STEP3: Create Graphs. v %d e %d f %d\n", nsg->sgvtn, nsg->sgedn, nsg->sgfcn ); */
  
  return nsg;
}

/* Shortest-Path between two Vertices Widget */
#define TWO_VERTICES 2

void create_shortest_path( ScreenAtr *screen )
{
  int  i, iter;
  Boolean isCalculated;
  Sppd *ppd;
  Spvt *src, *dist;
  SGraph *sg, *org_sg, *new_sg;
  SGlp *org_lp, *tmp_lp;
  SGvt *sgsrc, *sgdist, *v;
  void FreeSelectList( ScreenAtr * );
  void free_sglp( SGlp * );
  SGvt *find_sgvt_from_spvt( SGraph *, Spvt * );
  SGlp *ShortestPath_Dijkstra_SGraph( SGraph *, SGvt *, SGvt * );
  void drawwindow( int );
  
  if ( screen->current_ppd == (Sppd *) NULL ) return;
  if ( screen->n_sellist != TWO_VERTICES ) {
    FreeSelectList(screen);
    return;
  }

  ppd  = screen->current_ppd;
  src  = screen->sel_first->vt;
  dist = screen->sel_first->nxt->vt;

  FreeSelectList( screen );
  
  if ( (src == NULL) || (dist == NULL) ) return;
  if ( src == dist ) return;

  free_sglp( screen->lp );

  sg = screen->sg; if ( sg == NULL ) return;

/*   sg->src   = find_sgvt_from_spvt_number( sg, 104 ); */
/*   sg->dist  = find_sgvt_from_spvt_number( sg, 144 ); */

  sg->src  = find_sgvt_from_spvt( sg, src );
  sg->dist = find_sgvt_from_spvt( sg, dist );
  
  if ( (sg->src == NULL) || (sg->dist == NULL) ) return;

  i = 0;
  iter = 20;
  org_sg = sg;
  org_lp = NULL;
  new_sg = NULL;
  tmp_lp = NULL;
  isCalculated = False;
  sub_ratio = 0.5;

  while ( (i < iter) && (isCalculated == False) ) {
    if ( org_sg != sg ) {
      add_vertices_edges_sgraph( org_sg );
/*       display("vt %d fc %d ed %d\n", org_sg->sgvtn, org_sg->sgfcn, org_sg->sgedn ); */
/*       display("src %d dist %d\n", org_sg->src->no, org_sg->dist->no ); */
/*       for ( v = new_sg->sgvt; v != NULL; v = v->nxt ) { */
/* 	display("\tv %d\n", v->no ); */
/*       } */
    }

    tmp_lp = ShortestPath_Dijkstra_SGraph( org_sg, org_sg->src, org_sg->dist );
    display("i = %d graph v %d f %d e %d length %g\n", i,
	    org_sg->sgvtn, org_sg->sgfcn, org_sg->sgedn, tmp_lp->length );
    new_sg = sglp_to_sgraph( tmp_lp, org_sg );

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
    screen->nsg = org_sg;
    screen->lp  = org_lp;
    drawwindow( screen->no );
/*     sleep(100); */
  }
  display("calculation finished. Length = %g\n", org_sg->lp->length );
  
  drawwindow( screen->no );
}

void free_splp_shortest_path( Splp *lp, Sppd *ppd )
{
  Sped  *ed;
  Sple  *le;
  Splv  *lv, *plv;
  Spvt  *vt;
  HEdge *hed;
  void  FreeSped( Sped *, Sppd * );
  void  free_ppdvertex( Spvt *, Sppd * );
  void  free_splp( Splp * );
  Sped  *find_ppdedge( Spvt *, Spvt * );

  /* delete loopedges */
/*   for ( le = lp->sple; le != (Sple *) NULL; le = le->nxt ) { */

  plv = lv = lp->splv;
  lv  = lv->nxt;

  while ( lv != (Splv *) NULL ) {

    if ( (ed = find_ppdedge( plv->vt, lv->vt )) != (Sped *) NULL ) {
	    
      /*
	If there is no link to the faces( ed->type == EDGE_ADDED ), an
	edge is SP_EDGE_STEINER, thus it's deleted. else an edge is
	SP_EDGE_NORMAL, thus it should be left.
      */
/*       display("ed %d sp_type %d\n", ed->no, ed->sp_type ); */
      if ( (ed->type == EDGE_ADDED ) ) {
	FreeSped( ed, ppd );
      } else {
	ed->sp_type = SP_EDGE_NORMAL;
      }

    }

    if ( lv == lp->eplv ) break;
    plv = lv;
    lv = lv->nxt;

  }

  /* delete loopedges */
  for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    /*
      If there is no link to the edges, a vertex is SP_VERTEX_STEINER,
      thus it's deleted. else if a vertex is SP_VERTEX_NORMAL, thus it
      should be left.
    */
    vt = lv->vt;
    vt->lp = NULL;
    --( vt->spn );
    
/*     display("vt %d ven %d\n", vt->no, vt->ven ); */
/*     if ( !(vt->ven) ) { */
    /* steiner points */
    if ( vt->sp_ed != NULL ) {
      
      vt->sp_ed = NULL;
      free_ppdvertex( vt, ppd );
      display("vt %d deleted.\n", vt->no );
      
    } else {

      if ( vt->sp_type != SP_VERTEX_HVERTEX ) {
	vt->sp_type = SP_VERTEX_NORMAL;
      }
      
    }
  }

  if ( lp->hed != NULL ) {

    hed = lp->hed;
    if ( hed->lp1 == lp ) hed->lp1 = NULL;
    if ( hed->lp2 == lp ) hed->lp2 = NULL;

  }

  free_splp( lp );

}
    
void delete_shortest_path( ScreenAtr *screen )
{
  Sppd *ppd;
  Splp *lp;
  SelList *sl;
  void free_splp_shortest_path( Splp *, Sppd * );
  void FreeSelectList( ScreenAtr * );
  
  ppd = screen->current_ppd;
  for ( sl = screen->sel_first; sl != (SelList *) NULL; sl = sl->nxt ) {

    lp = sl->lp;
    free_splp_shortest_path( lp, ppd );
    sl->lp = (Splp *) NULL;
    
  }
  FreeSelectList(screen);
}

void manual_shortest_path( ScreenAtr *screen )
{
  Sppd  *ppd;
  Splp  *lp;
  Spvt  *sv, *ev;
  HEdge *hed;
  SelList *sl;
  void FreeSelectList( ScreenAtr * );
  HEdge *find_hppdedge_from_spvt( Spvt *, Spvt *, ScreenAtr *, HPpd * );
  Splp  *sellist_to_splp( ScreenAtr * );
  
  ppd = screen->current_ppd;

  /* check whether shortest path can be created or not. */

  if ( screen->n_sellist < 2 ) {
    displayinfo("Too few vertices are selected. \n");
    FreeSelectList( screen );
    return;
  }

  sv = screen->sel_first->vt;
  ev = screen->sel_last->vt;
  hed = find_hppdedge_from_spvt( sv, ev, screen, swin->hppd );

  if ( hed == NULL ) {
    displayinfo("Both start and end vertices should be CVs. \n");
    FreeSelectList( screen );
    return;
  }

  if ( screen->no == SCREEN1 ) {
    if ( (lp = hed->lp1) != NULL ) {
      displayinfo("SPath has already exist between two vertices. \n");
      FreeSelectList( screen );
      return;
    }
  } else {
    if ( (lp = hed->lp2) != NULL ) {
      displayinfo("SPath has already exist between two vertices. \n");
      FreeSelectList( screen );
      return;
    }
  }
      
  lp = sellist_to_splp( screen );
  if ( screen->no == SCREEN1 ) hed->lp1 = lp;
  else hed->lp2 = lp;
  lp->hed = hed;
  
  displayinfo("A SPath at HEdge No.%d of Screen No.%d is generated.\n",
	      hed->no, screen->no+1);
  
  FreeSelectList( screen );
}

Splp *sellist_to_splp( ScreenAtr *screen )
{
  SelList *sl;
  Splp    *lp;
  Spvt    *vt, *sv, *ev;
  Splv    *lv;
  Sped    *ed;
  Splp    *create_splp( void );
  Splv    *create_ppdloopvertex( Splp * );
  Sped    *find_ppdedge( Spvt *, Spvt * );

  lp = create_splp();
  lp->type  = SHORTESTPATH;

  for ( sl = screen->sel_first; sl != (SelList *) NULL; sl = sl->nxt ) {
    vt = sl->vt;
    lv = create_ppdloopvertex( lp );
    lv->vt = sl->vt; 
    ++( vt->spn );
    if ( (sl != screen->sel_first) && (sl != screen->sel_last) ) {
      vt->sp_type = SP_VERTEX_BOUNDARY;
      vt->lp = lp;
    }
  }

  lv = lp->splv;
  while ( lv != lp->eplv ) {
    sv = lv->vt;
    ev = lv->nxt->vt;
    if ( (ed = find_ppdedge( sv, ev )) != (Sped *) NULL ) {
      ed->sp_type = SP_EDGE_BOUNDARY;
    }
    lv = lv->nxt;
  }

  return lp;
}
    
/* sgraph initialize */
void screen_initialize_sgraph( ScreenAtr *screen )
{
  SGraph *initialize_sgraph( Sppd * );

  if ( screen->current_ppd == NULL ) return;
  
  screen->sg = initialize_sgraph( screen->current_ppd );
  screen->lp  = NULL;
  screen->nsg = NULL;
  
}

/* sgraph exit */
void screen_exit_sgraph( ScreenAtr *screen )
{
  void free_sgraph( SGraph * );

  if ( screen->sg == NULL ) return;
  
  free_sgraph( screen->sg );
  screen->sg = NULL;
  if ( screen->nsg != NULL ) {
    free_sgraph( screen->nsg );
    screen->nsg = NULL;
  }
}

