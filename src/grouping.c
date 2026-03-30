/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

/*******************************************************************************
  STEP 1: Grouping PPD according to HPPD
*******************************************************************************/

void hppd_to_hgppd( HPpd *hppd )
{
  void subdivide_ppdedges( HPpd * );
  void cmesh_group_hppd( HPpd * );
  
  /* subdivide ppd edges */
  subdivide_ppdedges( hppd );
  /* grouping vertices, edges */
  cmesh_group_hppd( hppd );
}

/* ppd edge subdivision */
void subdivide_ppdedges( HPpd *hppd )
{
  int   i;
  Sppd  *ppd;
  HEdge *hed;
  Splp  *lp;
  Splv  *lv, *plv;
  Spvt  *vt, *sv, *ev;
  Sped  *ed, *ned, *ed1;
  Edvt  *edvt;
  void  insert_edvtx( Sped *, Spvt *, Sped *, double );
  void  free_ppdedgevertex( Sped * );
  void  free_vertexedge( Spvt *, Sped * );
  void  free_ppdedge( Sped *, Sppd * );
  Sped  *create_ppdedge( Sppd * );
  Vted  *create_vtxed( Spvt *, Sped * );
  Sped  *find_ppdedge( Spvt *, Spvt * );

  for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {

    for ( i = 0; i < 2; ++i ) {
      
      if ( !i ) lp = hed->lp1; else lp = hed->lp2;

      plv = lv = lp->splv;
      
      lv  = lv->nxt;

      while ( lv != (Splv *) NULL ) {

	if ( (ed = find_ppdedge( plv->vt, lv->vt )) != (Sped *) NULL ) {
	    
	  /* create additional edges */
	  ed->type = EDGE_ORIGINAL;
	}
	
	/* create edge->vertex links for the steiner points */
	vt = lv->vt;
	if ( vt->sp_type == SP_VERTEX_BOUNDARY ) {
	  insert_edvtx( vt->sp_ed, vt, NULL, vt->sp_val );
	}

	if ( lv == lp->eplv ) break;
	plv = lv;
	lv = lv->nxt;
      }
      
    }

  }

  for ( i = 0; i < 2; ++i ) {

/*     display("i = %d\n", i ); */
    
    if ( !i ) ppd = hppd->ppd1; else ppd = hppd->ppd2;

/*     display("en(before) %d\n", ppd->en); */
    
    ed = ppd->sped;
    while ( ed != (Sped *) NULL ) {

      ed1 = ed->nxt;

      if ( ( ed->sp_type == SP_EDGE_NORMAL ) && ( ed->evn ) ) {

/* 	display("ed %d evn %d\n", ed->no, ed->evn ); */
	/* create subidivided edge */
	sv = ed->ev;
	edvt = ed->sedvt;
	ned = NULL;
	
	while ( edvt != (Edvt *) NULL ) {
	  
	  ev = edvt->vt;
/* 	  display("val %g\n", edvt->val); */
	  
	  ned = create_ppdedge( ppd );
/* 	  display("Edge %d created.\n", ned->no); */
	  ned->sv = sv;
	  ned->ev = ev;
/* 	  display("ed sv %d ev %d created.\n", sv->no, ev->no ); */
/* 	  ned->type = EDGE_ADDED; */
	  (void) create_vtxed(sv, ned);
	  (void) create_vtxed(ev, ned);

	  edvt = edvt->nxt;
	  sv = ev;
	}

	/* last edge */
	ev = ed->sv;
	ned = create_ppdedge( ppd );
	ned->sv = sv;
	ned->ev = ev;
/* 	display("ed sv %d ev %d created.\n", sv->no, ev->no ); */
/* 	ned->type = EDGE_ADDED; */
	(void) create_vtxed(sv, ned);
	(void) create_vtxed(ev, ned);
	
	/* free vertex-edge links */
	free_vertexedge( ed->sv, ed );
	free_vertexedge( ed->ev, ed );

	/* free edge-steinervertex links */
	free_ppdedgevertex( ed );
	
/* 	display("ed sv %d ev %d deleted.\n", ed->sv->no, ed->ev->no ); */
	/* free ppdedge */
	free_ppdedge( ed, ppd );
      }
      
      ed = ed1;
    }

/*     display("en(after) %d\n", ppd->en); */
  }
}

/* ppd grouping according to control meshes */
void cmesh_group_hppd( HPpd * hppd )
{
  int   i;
  Sppd  *ppd;
  Sped  *ed;
  Spvt  *vt;
  HGppd *hgppd;
  /* temp */
  HFace *hf, *hfc;
  HGfc  *hgfc;
  Splp  *lp;
  HGed  *he;
  
  HGppd *ppdgroup( Sppd *, HPpd *, int );
  void  hgppdface_to_hppdface( HGppd *, HPpd *, int );
/*   void  hloop_to_hgppd( HPpd *, int ); */
  void  HGfcOrderBoundaryEdges( HPpd *, int );
  void  hgppd_make_edge_vertex_links( HPpd *, int );
  
  for ( i = 0; i < 2; ++i ) {
    
    /* initialize */
    if (!i) ppd = hppd->ppd1; else ppd = hppd->ppd2;
    
    for ( vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt ) {
      vt->group_id = SMDNULL;
    }

    for ( ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt ) {
      ed->group_id = SMDNULL;
    }

    /* make group ppd vertices, edges, control vertices */
    hgppd = ppdgroup( ppd, hppd, i );
    
    /* correspondence between HFace and HGfc */
/*     hgppdface_to_hppdface( hgppd, hppd, i ); */

    /* hloop to hgppd */
/*     hloop_to_hgppd( hppd, i ); */

    /* make edge->vertex links */
    hgppd_make_edge_vertex_links( hppd, i );

    /* sort hgppd edges of boundary */
    /* It is important to merge two embeddings */
    HGfcOrderBoundaryEdges( hppd, i );

  }

/*   for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) { */
/*       display("hfc %d hgfc1 en %d hgfc2 en %d\n", hfc->no, hfc->hgfc1->hgen, hfc->hgfc2->hgen ); */
/*   } */
}

/* make group ppd vertices, edges, control vertices */
HGppd *ppdgroup( Sppd *ppd, HPpd *hppd, int ppdid )
{
  HGppd *hgppd;
  HFace *hfc;
  HGfc  *hgfc;
  Sped  *ed;
  Spvt *vt;
  HGed  *hged;
  HGed *create_hgppdedge( HGfc * );
  HGppd *create_hgppd(void);
  HGfc  *hloop_to_hgfc( HFace *, HGppd *, int );
  HGfc *find_ppdedge_in_hgppd( Sped *, HGppd * );
  HGfc *find_hgfc_grouping( Spvt *, Sppd *, HGppd * );
  void group_hgppd_vertex( Spvt *, HGfc * );

  hgppd = create_hgppd();

  if ( !ppdid ) hppd->hgppd1 = hgppd;
  else hppd->hgppd2 = hgppd;

  /* first, grouping from boundary */
  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {
    hgfc = hloop_to_hgfc( hfc, hgppd, ppdid );
  }

  /* find in edges */
  for ( ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt ) {

    if ( ed->sp_type == SP_EDGE_NORMAL ) {
      if ( (ed->sv->sp_type != SP_VERTEX_NORMAL) &&
	   (ed->ev->sp_type != SP_VERTEX_NORMAL) ) {
	if ( (hgfc = find_ppdedge_in_hgppd( ed, hgppd )) != (HGfc *) NULL ) {
	  hged = create_hgppdedge( hgfc );
	  hged->sp_type = SP_EDGE_NORMAL;
	  hged->ed = ed;
	  ed->group_id = hgfc->id;
	}
      }
    }
    
  }

  for ( vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt ) {
    if ( (vt->sp_type == SP_VERTEX_NORMAL) && (vt->group_id == SMDNULL) ) {

      if ( (hgfc = find_hgfc_grouping( vt, ppd, hgppd )) != (HGfc *) NULL ) {
	/* create first hgppd edge */
	group_hgppd_vertex( vt, hgfc );

      }
	
    }
  }

  return hgppd;
}

#define TMP_CVN 128

HGfc *find_hgfc_grouping( Spvt *vt, Sppd *ppd, HGppd *hgppd )
{
  int gn, cn;
  int *gid, *vf;
  HGfc *hgfc;
  Spvt *v;
  Spvt **cvt;
  void find_hgfc_vertex( Spvt *, int *, int *, int *, int *, Spvt ** );
  HGfc *find_hgfc( HGppd *, int );
  HGfc *find_hgfc_cvt( HGppd *, int, Spvt ** );

/*   display("vt %d\n", vt->no); */
  /* max number of gn = 2 */
  /* grouping id */
  gn  = 0;
  gid = (int *) malloc( hgppd->nhgfc * sizeof( int ) );

  cn = 0;
  cvt = (Spvt **) malloc( TMP_CVN * sizeof( Spvt * ) );
  
  /* 頂点探索用フラッグ */
  vf = (int *) malloc( ppd->vn * sizeof( int ) );
  for ( v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {
    vf[v->no] = SMD_OFF;
  }
  
  find_hgfc_vertex( vt, &gn, gid, vf, &cn, cvt );

/*   display("gn %d\n", gn); */
  
  /* find! */
  if ( gn == 1 ) {
    hgfc = find_hgfc( hgppd, gid[0] );
    free(gid);
    free(vf);
    free(cvt);
    return hgfc;
  } else {
    if ( cn <= 2 ) {
/*       display("Warning: can't find hgfc.\n"); */
      free(gid);
      free(vf);
      free(cvt);
      return (HGfc *) NULL;
    } else {
      hgfc = find_hgfc_cvt( hgppd, cn, cvt );
      free(gid);
      free(vf);
      free(cvt);
      return hgfc;
    }
  }
}
HGfc *find_hgfc_cvt( HGppd *hgppd, int cn, Spvt **cvt )
{
  HGfc *hgfc;
  int find_cvt_in_hgfc( int, Spvt **, HGfc * );
  
  for ( hgfc = hgppd->shgfc; hgfc != (HGfc *) NULL; hgfc = hgfc->nxt ) {
    if ( find_cvt_in_hgfc( cn, cvt, hgfc ) == SMD_ON ) {
      return hgfc;
    }
  }
  return (HGfc *) NULL;
}
int find_cvt_in_hgfc( int cn, Spvt **cvt, HGfc *hgfc )
{
  int i;
  int find_vt_in_hgppdcntrlvertex( Spvt *, HGfc * );
  
  for ( i = 0; i < cn; ++i ) {
    if ( find_vt_in_hgppdcntrlvertex( cvt[i], hgfc ) == SMD_OFF ) {
      return SMD_OFF;
    }
  }
  return SMD_ON;
}

int find_vt_in_hgppdcntrlvertex( Spvt *vt, HGfc *hgfc )
{
  HGcv *c;
  
  for ( c = hgfc->shgcv; c != (HGcv *) NULL; c = c->nxt ) {
    if ( c->vt == vt ) return SMD_ON;
  }
  return SMD_OFF;
}
void find_hgfc_vertex( Spvt *vt, int *gn, int *gid, int *vf, int *cn, Spvt **cvt )
{
  Vted *vted;
  int i, cnt;
  int tid[2];
  HEdge *hed;
  int rid, lid;
  Splp *lp;
  void find_hgfc_vertex( Spvt *, int *, int *, int *, int *, Spvt ** );
  Spvt *another_vt( Sped *, Spvt * );
  int  check_gid( int, int, int * );
  int find_cvt( Spvt *, int, Spvt ** );
  
  if ( vt == (Spvt *) NULL ) return;
  if ( vf[vt->no] == SMD_ON ) return;

  vf[vt->no] = SMD_ON;

/*   display("\tgn %d\n", gn); */
  switch ( vt->sp_type ) {
  case SP_VERTEX_NORMAL:
/*     display("\tnormal.\n"); */
    /* create hgppd edge */
    for ( vted = vt->svted; vted != (Vted *) NULL; vted = vted->nxt ) {
      find_hgfc_vertex( another_vt( vted->ed, vt ), gn, gid, vf, cn, cvt );
    }
    break;
  case SP_VERTEX_BOUNDARY:
/*     display("\tboundary.\n"); */

    lp = vt->lp;
    if ( lp == (Splp *) NULL ) return;
    hed = lp->hed;
    if ( hed == (HEdge *) NULL ) return;
    
    if ( !(*gn) ) {
      if ( hed->rf != (HFace *) NULL) {
/* 	display("aa\n"); */
	gid[(*gn)++] = hed->rf->no;
      }
      if ( hed->lf != (HFace *) NULL) {
/* 	display("bb\n"); */
	gid[(*gn)++] = hed->lf->no;
      }
/*       display("gn %d %d %d\n", *gn, gid[0], gid[1]); */
    } else {
      cnt = 0;
      if ( hed->rf != (HFace *) NULL) {
	rid = hed->rf->no;
/* 	display("rid %d gn %d gid %d %d\n", rid, *gn, gid[0], gid[1]); */
	if ( check_gid( rid, *gn, gid ) == SMD_ON ) {
	  tid[cnt++] = rid;
	}
      }
      if ( hed->lf != (HFace *) NULL ) {
	lid = hed->lf->no;
/* 	display("lid %d gn %d gid %d %d\n", lid, *gn, gid[0], gid[1]); */
	if ( check_gid( lid, *gn, gid ) == SMD_ON ) {
	  tid[cnt++] = lid;
	}
      }
      if ( cnt != *gn ) {
	*gn = cnt;
	for ( i = 0; i < cnt; ++i ) gid[i] = tid[i];
      }
    }
    break;
  case SP_VERTEX_HVERTEX:
    if ( find_cvt( vt, *cn, cvt ) == SMD_OFF ) {
      cvt[(*cn)++] = vt;
    }
    break;
  }
}

int find_cvt( Spvt *vt, int cn, Spvt **cvt )
{
  int i;
  
  for ( i = 0; i < cn; ++i ) {
    if ( cvt[i] == vt ) return SMD_ON;
  }
  return SMD_OFF;
}

int check_gid( int id, int n, int *gid )
{
  int i;
  
  for ( i = 0; i < n; ++i ) {
    if ( gid[i] == id ) return SMD_ON;
  }
  return SMD_OFF;
}

HGfc *hloop_to_hgfc( HFace *hf, HGppd *hgppd, int ppdid )
{
  int   i;
  HLoop *hl;
  Splp  *lp;
  HGfc  *hgfc;
  HGvt  *hgvt;
  HGed  *hged;
  HGcv  *hgcv;
  Splv  *lv;
  Sple  *le;
  HGvt *create_hgppdvertex( HGfc * );
  HGed *create_hgppdedge( HGfc * );
  HGcv *create_hgppdctrlvertex( HGfc * );
  HGfc *create_hgppdface( HGppd * );
  
  if ( hf == (HFace *) NULL ) return (HGfc *) NULL;
  
  hl   = hf->hloop;
  lp   = (!ppdid) ? hl->lp1   : hl->lp2;
  
  hgfc = create_hgppdface( hgppd );
  hgfc->id = hf->no;
  
  if (!ppdid) hf->hgfc1 = hgfc; else hf->hgfc2 = hgfc;

/*   display("hgfc %d ppdid %d\n", hgfc->id, ppdid ); */
  /* create control vertices */
  
/*   hgfc->cvn = hf->hen; */
/*   hgfc->cvt = (Spvt **) malloc( hgfc->cvn * sizeof(Spvt * ) ); */

  /* loopvertex to hgppdvertex */
  i = 0;
  for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    hgvt = create_hgppdvertex( hgfc );
    hgvt->vt = lv->vt;
    /* used in "hgfcharmonic" */
    lv->hgvt = hgvt;
    if ( lv->vt->sp_type == SP_VERTEX_HVERTEX ) {
/*       hgfc->cvt[i++] = lv->vt; */
      hgcv = create_hgppdctrlvertex( hgfc );
      hgcv->vt = lv->vt;
      hgvt->sp_type = SP_VERTEX_HVERTEX;
    } else {
      hgvt->sp_type = SP_VERTEX_BOUNDARY;
    }
  }
    
  /* loopedge to hgppdedge */
  for ( le = lp->sple; le != (Sple *) NULL; le = le->nxt ) {
    hged = create_hgppdedge( hgfc );
    hged->ed = le->ed;
    hged->sp_type = SP_EDGE_BOUNDARY;
/*     display("edge %d created.\n", hged->no ); */
  }

/*   display("--hgfc %d en %d\n", hgfc->id, hgfc->hgen ); */
  return hgfc;
}

HGfc *find_ppdedge_in_hgppd( Sped *ed, HGppd *hgppd )
{
  HGfc *hgfc;
  int  find_ppdvertex_in_hgfc( Spvt *, HGfc * );
  
  for ( hgfc = hgppd->shgfc; hgfc != (HGfc *) NULL; hgfc = hgfc->nxt ) {

    if ( (find_ppdvertex_in_hgfc( ed->sv, hgfc ) == SMD_ON) &&
	 (find_ppdvertex_in_hgfc( ed->ev, hgfc ) == SMD_ON) ) {
      return hgfc;
    }
  }
  return (HGfc *) NULL;
}
    
int find_ppdvertex_in_hgfc( Spvt *vt, HGfc *hgfc )
{
  HGvt *hgvt;
  
  for ( hgvt = hgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {
    if ( hgvt->vt == vt ) return SMD_ON;
  }
  return SMD_OFF;
}

Sped *next_ungrouped_edge( Sped *ed )
{
  while ( ed != (Sped *) NULL ) {
    if ( (ed->sp_type == SP_EDGE_NORMAL) && (ed->group_id == SMDNULL) )
      return ed;
    ed = ed->nxt;
    
  }
  return (Sped *) NULL;
}
  
void group_hgppd_vertex( Spvt *vt, HGfc *hgfc )
{
  Vted *vted;
  HGvt *hgvt;
  HGed *hged;
  Sped *ed;
  Spvt *another_vt( Sped *, Spvt * );
  void group_hgppd_vertex( Spvt *, HGfc * );
  HGvt *create_hgppdvertex( HGfc * );
  HGed *create_hgppdedge( HGfc * );

  if ( vt == (Spvt *) NULL ) return;
  if ( vt->group_id != SMDNULL ) return;
  if ( vt->sp_type != SP_VERTEX_NORMAL ) return;
  
  vt->group_id = hgfc->id;
  hgvt = create_hgppdvertex( hgfc );
  hgvt->vt = vt;

  /* create hgppd edge */
  for ( vted = vt->svted; vted != (Vted *) NULL; vted = vted->nxt ) {

    ed = vted->ed;
    if ( ed->group_id == SMDNULL ) {

      ed->group_id = hgfc->id;
      hged = create_hgppdedge( hgfc );
      hged->ed = ed;
      hged->sp_type = SP_EDGE_NORMAL;
      
    }
    group_hgppd_vertex( another_vt( ed, vt ), hgfc );
  }
  
}

void hgppdface_to_hppdface( HGppd *hgppd, HPpd *hppd, int ppdid )
{
  HGfc *hgfc;
  HFace *cface;
  HEdge *cedge;
  void coincident_hface_hgface( HPpd *, HGfc *, int );

  for ( hgfc = hgppd->shgfc; hgfc != (HGfc *) NULL; hgfc = hgfc->nxt ) {

    /* decide hgfc->hf */
    coincident_hface_hgface( hppd, hgfc, ppdid );
  }

}

/* Changed */
void coincident_hface_hgface( HPpd *hppd, HGfc *hgfc, int ppdid )
{
  int  i;
  Spvt **cvt, **hgvt;
  HGcv  *hc;
  HFace *hf;
  HGfc  *ahgfc;
  HHalfedge *he;
  int coincidence_hface_hgfc( int, Spvt **, Spvt ** );
  
  if ( hgfc == NULL ) return;
  if ( hppd == NULL ) return;

  hgvt = (Spvt **) malloc( hgfc->hgcn * sizeof(Spvt *) );
  cvt  = (Spvt **) malloc( hgfc->hgcn * sizeof(Spvt *) );
  
  for ( hc = hgfc->shgcv, i = 0; hc != (HGcv *) NULL; hc = hc->nxt, ++i )
    hgvt[i] = hc->vt;
  
  /* HFace are only triangle shapes */
  for ( hf = hppd->shfc; hf != (HFace *) NULL; hf = hf->nxt ) {

    if ( !ppdid ) ahgfc = hf->hgfc1; else ahgfc = hf->hgfc2;
    if ( ahgfc != (HGfc *) NULL ) continue;
    if ( hgfc->hgcn != hf->hen ) continue;
    
    /* store hvertices */
    i = 0; he = hf->shhe;
    do {
      if ( !ppdid ) cvt[i] = he->vt->vt1; else cvt[i] = he->vt->vt2;
      ++i;
    } while ( (he = he->nxt) != hf->shhe );

    if ( coincidence_hface_hgfc( hf->hen, cvt, hgvt ) == SMD_ON ) {
      
	if ( !ppdid ) hf->hgfc1 = hgfc; else hf->hgfc2 = hgfc;
	hgfc->hf = hf;
	free(cvt); free(hgvt);
	return;

    }
  }
  free(cvt); free(hgvt);
}    

int coincidence_hface_hgfc( int n, Spvt **cvt, Spvt **hgvt )
{
  int flag;
  int i, j;

  for ( i = 0; i < n; ++i ) {
    
    flag = SMD_OFF; j = 0;
    while ( !flag ) {
      if ( cvt[i] == hgvt[j] ) {
	flag = SMD_ON;
      }
      ++j;
      if ( j == n ) break;
    }
    if ( !flag ) return SMD_OFF;
    
  }
  
  return SMD_ON;
  
}
  

/* void hloop_to_hgppd( HPpd *hppd, int ppdid ) */
/* { */
/*   int   i; */
/*   HFace *hf; */
/*   HLoop *hl; */
/*   Splp  *lp; */
/*   HGfc  *hgfc; */
/*   HGvt  *hgvt; */
/*   HGed  *hged; */
/*   Splv  *lv; */
/*   Sple  *le; */
/*   HGvt *create_hgppdvertex( HGfc * ); */
/*   HGed *create_hgppdedge( HGfc * ); */
  
/*   for ( hf = hppd->shfc; hf != (HFace *) NULL; hf = hf->nxt ) { */

/*     hl   = hf->hloop; */
/*     lp   = (!ppdid) ? hl->lp1   : hl->lp2; */
/*     hgfc = (!ppdid) ? hf->hgfc1 : hf->hgfc2; */
    
/*     hgfc->cvn = hf->hen; */
/*     hgfc->cvt = (Spvt **) malloc( hgfc->cvn * sizeof(Spvt * ) ); */

/*     i = 0; */
/*     for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) { */
/*       hgvt = create_hgppdvertex( hgfc ); */
/*       hgvt->vt = lv->vt; */
/*       lv->hgvt = hgvt; */
/*       if ( lv->vt->sp_type == SP_VERTEX_HVERTEX ) { */
/* 	hgfc->cvt[i++] = lv->vt; */
/*       } */
/*     } */
    
/*     for ( le = lp->sple; le != (Sple *) NULL; le = le->nxt ) { */
/*       hged = create_hgppdedge( hgfc ); */
/*       hged->ed = le->ed; */
/*     } */
    
/*   } */
/* } */

/* hged->sp_type == SP_EDGE_BOUNDARY の整列 */
void HGfcOrderBoundaryEdges( HPpd *hppd, int ppdid )
{
  int   reverse;
  HFace *hf;
  HHalfedge *he;
  HEdge *hed;
  HVertex *shvt, *ehvt;
  HGfc  *hgfc;
  HGed  *hged;
  Spvt  *svt, *evt;
  HGvt  *hgvt, *shgvt, *ehgvt;
  Splp  *lp;
  Splv  *lv;
  HGvt  *hgvt_in_hgfc( Spvt *, HGfc * );
  HGed  *hged_in_hgfc( Spvt *, Spvt *, HGfc * );

/*   display("*********(%d)*********\n", ppdid ); */
  for ( hf = hppd->shfc; hf != (HFace *) NULL; hf = hf->nxt ) {

    hgfc = (!ppdid) ? hf->hgfc1 : hf->hgfc2;

    he = hf->shhe;
    do {

      hed  = he->hed;
      shvt = he->vt;
      ehvt = he->nxt->vt;

      lp   = (!ppdid) ? hed->lp1 : hed->lp2;

      svt = (!ppdid) ? shvt->vt1 : shvt->vt2;
      evt = (!ppdid) ? ehvt->vt1 : ehvt->vt2;
      shgvt = hgvt_in_hgfc( svt, hgfc );
      ehgvt = hgvt_in_hgfc( evt, hgfc );

      
      if ( shvt == hed->sv ) reverse = SMD_OFF;
      else                   reverse = SMD_ON;
/*       display("(hgvt) start %d end %d reverse %d\n", shgvt->no, ehgvt->no, reverse ); */
      
      for ( lv = lp->splv; lv->nxt != (Splv *) NULL; lv = lv->nxt ) {

	hged = hged_in_hgfc( lv->vt, lv->nxt->vt, hgfc );
	GMORPH_ASSERT(hged != (HGed *) NULL);
	GMORPH_ASSERT(hged->sv != (HGvt *) NULL);
	GMORPH_ASSERT(hged->ev != (HGvt *) NULL);
/* 	display("\thged %d sv %d ev %d\n", hged->no, hged->sv->no, hged->ev->no ); */
	if ( !reverse ) {
	  if ( hged->sv->vt != lv->vt ) {
	    hgvt     = hged->sv;
	    hged->sv = hged->ev;
	    hged->ev = hgvt;
	  }
	} else {
	  if ( hged->ev->vt != lv->vt ) {
	    hgvt     = hged->sv;
	    hged->sv = hged->ev;
	    hged->ev = hgvt;
	  }
	}
	
      }

    } while ( (he = he->nxt) != hf->shhe );
  }
  
}
      
void hgppd_make_edge_vertex_links( HPpd *hppd, int ppdid )
{
  HFace *hf;
  HGvt  *hgvt;
  HGed  *hged;
  HGfc  *hgfc;
  
  for ( hf = hppd->shfc; hf != (HFace *) NULL; hf = hf->nxt ) {

/*     display("group No.%d ppdid %d\n", hf->no, ppdid); */
    hgfc = (!ppdid) ? hf->hgfc1 : hf->hgfc2;

    /* initialize */
    for ( hgvt = hgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {
      hgvt->vt->hgvt = hgvt;
    }
    
    /* make links */
    for ( hged = hgfc->shged; hged != (HGed *) NULL; hged = hged->nxt ) {
      hged->sv = hged->ed->sv->hgvt;
      hged->ev = hged->ed->ev->hgvt;
/*       display("ed %d sv %d ev %d\n", */
/* 	      hged->no, */
/* 	      hged->sv->no, */
/* 	      hged->ev->no ); */
    }
    
    /* end */
    for ( hgvt = hgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {
      hgvt->vt->hgvt = (HGvt *) NULL;
    }
  }
  
}

