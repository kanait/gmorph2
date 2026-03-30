/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

/*******************************************************************************
  STEP 3: Making New PPD
*******************************************************************************/

/* make vertices from HVertex */
Sppd *hppd_to_gppd( HPpd *hppd )
{
  Sppd *gppd;
  Sppd *create_ppd( void );
  Spso *create_ppdsolid( Sppd * );
  Sppt *create_ppdpart( Sppd * );
  void create_mhgfc( HPpd * );
  void hvertex_to_gppd( HPpd *, Sppd * );
  void hedge_to_gppd( HPpd *, Sppd * );
  void hface_to_gppd( HPpd *, Sppd * );
  void hppd_uvw_to_vec( HPpd * );
  void mHGfcOrderBoundaryEdges( HPpd * );
  void HGfcMakeSign( HPpd * );

  /* create gppd */
  gppd = hppd->gppd = create_ppd();
  gppd->spso = create_ppdsolid( gppd );
  gppd->sppt = create_ppdpart( gppd );
  gppd->sppt->bpso = gppd->spso;
  
  /* hgppd initialize */
  create_mhgfc( hppd );
  HGfcMakeSign( hppd );
  hvertex_to_gppd( hppd, gppd );
  hedge_to_gppd( hppd, gppd );
  mHGfcOrderBoundaryEdges( hppd );
  hface_to_gppd( hppd, gppd );
  
  return gppd;
}

void create_mhgfc( HPpd *hppd )
{
  int   i;
  HFace *hfc;
  HGfc  *create_hgfc( void );
  
  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {
    hfc->mhgfc = create_hgfc();
    hfc->mhgfc->id = hfc->no;
    /* temp */
    hfc->hgfc1->mhgfc = hfc->mhgfc;
    hfc->hgfc2->mhgfc = hfc->mhgfc;
  }
}

void HGfcMakeSign( HPpd *hppd )
{
  int   i;
  HGvt  *vt;
  HFace *hfc;
  
  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {
    
    i = 0;
    for ( vt = hfc->hgfc1->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
      vt->sign = i;
      ++i;
    }
    for ( vt = hfc->hgfc2->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
      vt->sign = i;
      ++i;
    }
    
  }
  
}

/************************************************************************ */
/************************************************************************ */

/* make vertices from HVertex */
void hvertex_to_gppd( HPpd *hppd, Sppd *gppd )
{
  HVertex *hvt;
  Spvt *mvt;
  HFace *hfc;
  HGvt *hgvt1, *hgvt2, *mhgvt;
  Spvt *create_ppdvertex( Sppd * );
  HGvt *hgvt_in_hgfc( Spvt *, HGfc * );
  HGvt *create_hgppdvertex( HGfc * );

  for ( hvt = hppd->shvt; hvt != (HVertex *) NULL; hvt = hvt->nxt ) {

    /* create mvt */
    mvt = create_ppdvertex( gppd );
    hvt->vt1->mvt = mvt;
    hvt->vt2->mvt = mvt;
    mvt->bpso = gppd->spso;
    mvt->sp_type = SP_VERTEX_HVERTEX;
    mvt->from = FROM_HVERTEX;
    
/*     display("(1)Vertex %d created.\n", mvt->no); */
    
    /* create hgvt */
    for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {

      if ( (hgvt1 = hgvt_in_hgfc( hvt->vt1, hfc->hgfc1 )) != (HGvt *) NULL ) {
	
	mhgvt = create_hgppdvertex( hfc->mhgfc );
	mhgvt->vt = mvt;
	mhgvt->uvw.x = hgvt1->uvw.x;
	mhgvt->uvw.y = hgvt1->uvw.y;
	mhgvt->sp_type = SP_VERTEX_HVERTEX;
	mvt->from = FROM_HVERTEX;
	/* link mhgvt with hgvt1 */
	hgvt1->mhgvt = mhgvt;
	
	/* link mhgvt with hgvt2 */
	hgvt2 = hgvt_in_hgfc( hvt->vt2, hfc->hgfc2 );
	hgvt2->mhgvt = mhgvt;
	
      }
      
    }
    
  }
  
}    

HGvt *hgvt_in_hgfc( Spvt *vt, HGfc *hgfc )
{
  HGvt *hgvt;
  
  for ( hgvt = hgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {
    if ( hgvt->vt == vt ) return hgvt;
  }
/*   display("(in hgvt_in_hgfc) sp_type %d\n", vt->sp_type ); */
  return (HGvt *) NULL;
}

HGed *hged_in_hgfc( Spvt *sv, Spvt *ev, HGfc *hgfc )
{
  HGed *hged;

  if ( hgfc == (HGfc *) NULL ) return (HGed *) NULL;
  for ( hged = hgfc->shged; hged != (HGed *) NULL; hged = hged->nxt ) {
    /* ??????????????????????????? */
    if ( hged->sv == (HGvt *) NULL || hged->ev == (HGvt *) NULL )
      continue;
    if ( (hged->sv->vt == sv) && (hged->ev->vt == ev) ) 
      return hged;
    if ( (hged->sv->vt == ev) && (hged->ev->vt == sv) )
      return hged;
  }
  return (HGed *) NULL;
}
/************************************************************************ */
/************************************************************************ */

/* Making vertices, edges from HEdge */
void hedge_to_gppd( HPpd *hppd, Sppd *gppd )
{
  int    i, n;
  HEdge  *hed;
  double *prm1, *prm2;
  Splp   *lp1, *lp2;
  Splv   *lv;
  Spvt   *mvt;
  Sped   *med;
  HGvt   *hgvt, *mhgvt;
  Hedvt  *ev, *pev;
  HGed   *mhged;
  HGfc   *rhgfc1, *lhgfc1, *rhgfc2, *lhgfc2, *rmhgfc, *lmhgfc;
  void   HGvtIncludeHGed( HEdge *, Splp *, Splp *, HGfc *, HGfc *, int );
  void   lpprm( Splp *, int, double * );
  void   lpprm_hgfc( Splp *, int, double *, HGfc * );
  Spvt   *create_ppdvertex( Sppd * );
  Sped   *create_ppdedge( Sppd * );
  Splp   *create_splp( void );
  Splv   *create_ppdloopvertex( Splp * );
  Hedvt  *insert_hedvt( HEdge *, Spvt *, double );
  HGvt   *hgvt_in_hgfc( Spvt *, HGfc * );
  HGed   *hged_in_hgfc( Spvt *, Spvt *, HGfc * );
  HGvt   *create_hgppdvertex( HGfc * );
  HGed   *create_hgppdedge( HGfc * );
  
  for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {

/*     display("hed %d\n", hed->no); */
    
    lp1 = hed->lp1;
    lp2 = hed->lp2;

    if ( hed->rf != (HFace *) NULL ) {
      rhgfc1 = hed->rf->hgfc1;
      rhgfc2 = hed->rf->hgfc2;
      rmhgfc = hed->rf->mhgfc;
/*       display("rhgfc %d\n", hed->rf->no ); */
/*       display("(1) sv %d ev %d\n", */
/* 	      hgvt_in_hgfc( lp1->splv->vt, rhgfc1 )->no, */
/* 	      hgvt_in_hgfc( lp1->eplv->vt, rhgfc1 )->no ); */

    }
    if ( hed->lf != (HFace *) NULL ) {
      lhgfc1 = hed->lf->hgfc1;
      lhgfc2 = hed->lf->hgfc2;
      lmhgfc = hed->lf->mhgfc;
/*       display("lhgfc %d\n", hed->lf->no ); */
/*       display("(1) sv %d ev %d\n", */
/* 	      hgvt_in_hgfc( lp1->splv->vt, lhgfc1 )->no, */
/* 	      hgvt_in_hgfc( lp1->eplv->vt, lhgfc1 )->no ); */
    }

    prm1 = (double *) malloc( lp1->lvn * sizeof( double ) );
    prm2 = (double *) malloc( lp2->lvn * sizeof( double ) );
    
    if ( hed->rf != (HFace *) NULL ) {
      lpprm_hgfc( lp1, lp1->lvn, prm1, rhgfc1 );
      lpprm_hgfc( lp2, lp2->lvn, prm2, rhgfc2 );
    } else {
      lpprm_hgfc( lp1, lp1->lvn, prm1, lhgfc1 );
      lpprm_hgfc( lp2, lp2->lvn, prm2, lhgfc2 );
    }

    /* loop 1 */
    for ( i = 0, lv = lp1->splv; lv != (Splv *) NULL; lv = lv->nxt, ++i ) {

      if ( (lv != lp1->splv) && (lv != lp1->eplv) ) {
	
	/* creating mvt */
	mvt = create_ppdvertex( gppd );
	lv->vt->mvt = mvt;
	mvt->bpso = gppd->spso;
	mvt->sp_type = SP_VERTEX_BOUNDARY;
	mvt->from  = FROM_SRC;
	mvt->vec.x = lv->vt->vec.x;
	mvt->vec.y = lv->vt->vec.y;
	mvt->vec.z = lv->vt->vec.z;

	/* creating mhgvt */
	if ( hed->rf != (HFace *) NULL ) {
	  mhgvt = create_hgppdvertex( rmhgfc );
	  mhgvt->vt = mvt;
	  mhgvt->sp_type = SP_VERTEX_BOUNDARY;
	  mhgvt->from = FROM_SRC;
	  hgvt = hgvt_in_hgfc( lv->vt, rhgfc1 );
	  mhgvt->uvw.x = hgvt->uvw.x;
	  mhgvt->uvw.y = hgvt->uvw.y;
	  hgvt->mhgvt = mhgvt;
	}
	  
	if ( hed->lf != (HFace *) NULL ) {
	  mhgvt = create_hgppdvertex( lmhgfc );
	  mhgvt->vt = mvt;
	  mhgvt->sp_type = SP_VERTEX_BOUNDARY;
	  mhgvt->from = FROM_SRC;
	  hgvt = hgvt_in_hgfc( lv->vt, lhgfc1 );
	  mhgvt->uvw.x = hgvt->uvw.x;
	  mhgvt->uvw.y = hgvt->uvw.y;
	  hgvt->mhgvt  = mhgvt;
	}
	
	insert_hedvt( hed, mvt, prm1[i] );
	
      }
      else {
	
	insert_hedvt( hed, lv->vt->mvt, prm1[i] );
	
      }
    }
    
    /* loop 2 */
    for ( i = 0, lv = lp2->splv; lv != (Splv *) NULL; lv = lv->nxt, ++i ) {
      
      if ( (lv != lp2->splv) && (lv != lp2->eplv) ) {
	
	/* creating mvt */
	mvt = create_ppdvertex( gppd );
	lv->vt->mvt = mvt;
	mvt->bpso = gppd->spso;
	mvt->sp_type = SP_VERTEX_BOUNDARY;
	mvt->from = FROM_TARGET;
	mvt->vec.x = lv->vt->vec.x;
	mvt->vec.y = lv->vt->vec.y;
	mvt->vec.z = lv->vt->vec.z;
	
	/* creating mhgvt */
	if ( hed->rf != (HFace *) NULL ) {
	  mhgvt = create_hgppdvertex( rmhgfc );
	  mhgvt->vt = mvt;
	  mhgvt->sp_type = SP_VERTEX_BOUNDARY;
	  mhgvt->from = FROM_TARGET;
	  hgvt = hgvt_in_hgfc( lv->vt, rhgfc2 );
	  mhgvt->uvw.x = hgvt->uvw.x;
	  mhgvt->uvw.y = hgvt->uvw.y;
	  hgvt->mhgvt  = mhgvt;
	}
	  
	if ( hed->lf != (HFace *) NULL ) {
	  mhgvt = create_hgppdvertex( lmhgfc );
	  mhgvt->vt = mvt;
	  mhgvt->sp_type = SP_VERTEX_BOUNDARY;
	  mhgvt->from = FROM_TARGET;
	  hgvt = hgvt_in_hgfc( lv->vt, lhgfc2 );
	  mhgvt->uvw.x = hgvt->uvw.x;
	  mhgvt->uvw.y = hgvt->uvw.y;
	  hgvt->mhgvt  = mhgvt;
	}
	
	insert_hedvt( hed, mvt, prm2[i] );
	
      }
      
    }

    /* create merging loop */
    hed->mlp = create_splp();
    
    ev = hed->sedvt;
    while ( 1 ) {

      /* create merging loop */
      lv = create_ppdloopvertex( hed->mlp );
      lv->vt = ev->vt;
      
      pev = ev;
      ev = ev->nxt;
      if ( ev == (Hedvt *) NULL ) break;

      /* creating med */
      med = create_ppdedge( gppd );
      med->sv = pev->vt;
      med->ev = ev->vt;
      med->sp_type = SP_EDGE_BOUNDARY;
      
      /* creating mhged */
      if ( hed->rf != (HFace *) NULL ) {
	mhged = create_hgppdedge( rmhgfc );
	mhged->sv = hgvt_in_hgfc( med->ev, rmhgfc );
	mhged->ev = hgvt_in_hgfc( med->sv, rmhgfc );
	mhged->ed = med;
	mhged->sp_type = SP_EDGE_BOUNDARY;
      }
      
      if ( hed->lf != (HFace *) NULL ) {
	mhged = create_hgppdedge( lmhgfc );
	mhged->sv = hgvt_in_hgfc( med->sv, lmhgfc );
	mhged->ev = hgvt_in_hgfc( med->ev, lmhgfc );
	mhged->ed = med;
	mhged->sp_type = SP_EDGE_BOUNDARY;
      }
      
    }
      
    free(prm1); free(prm2);

    /* hged->sp_type == SP_EDGE_BOUNDARY ?????? */
    /* ???????????????????????????? */
    
/*     for ( lv = lp1->splv; lv->nxt != (Splv *) NULL; lv = lv->nxt ) { */
/*       if ( lhgfc1 != NULL ) { */
/* 	hged = hged_in_hgfc( lv->vt, lv->nxt->vt, lhgfc1 ); */
/* 	if ( hged->sv->vt != lv->vt ) { */
/* 	  tmp_vt = hged->sv; */
/* 	  hged->sv = hged->ev; */
/* 	  hged->ev = tmp_vt; */
/* 	} */
/*       } */
/*       if ( rhgfc1 != NULL ) { */

/* 	hged = hged_in_hgfc( lv->vt, lv->nxt->vt, rhgfc1 ); */
/* 	if ( hged->ev->vt != lv->vt ) { */
/* 	  tmp_vt = hged->sv; */
/* 	  hged->sv = hged->ev; */
/* 	  hged->ev = tmp_vt; */
/* 	} */
/*       } */
/*     } */
    
/*     for ( lv = lp2->splv; lv->nxt != (Splv *) NULL; lv = lv->nxt ) { */
/*       if ( lhgfc2 != NULL ) { */
/* 	hged = hged_in_hgfc( lv->vt, lv->nxt->vt, lhgfc2 ); */
/* 	if ( hged->sv->vt != lv->vt ) { */
/* 	  tmp_vt   = hged->sv; */
/* 	  hged->sv = hged->ev; */
/* 	  hged->ev = tmp_vt; */
/* 	} */
/*       } */
/*       if ( rhgfc2 != NULL ) { */
/* 	hged = hged_in_hgfc( lv->vt, lv->nxt->vt, rhgfc2 ); */
/* 	if ( hged->ev->vt != lv->vt ) { */
/* 	  tmp_vt   = hged->sv; */
/* 	  hged->sv = hged->ev; */
/* 	  hged->ev = tmp_vt; */
/* 	} */
/*       } */
/*     } */
    
    /* hgvt1 ?? hged2 ???????? inc_ed ?????? */
    /* hgvt2 ?? hged1 ???????? inc_ed ?????? */
    /* ?????????????????????????????????? */

    if ( hed->rf != (HFace *) NULL ) {
/*       display("lp1->lp2.\n"); */
      HGvtIncludeHGed( hed, lp1, lp2, rhgfc1, rhgfc2, FROM_TARGET );
/*       display("lp2->lp1.\n"); */
      HGvtIncludeHGed( hed, lp2, lp1, rhgfc2, rhgfc1, FROM_SRC );
/*       display("end.\n"); */
    }
    if ( hed->lf != (HFace *) NULL ) {
/*       display("lp1->lp2.\n"); */
      HGvtIncludeHGed( hed, lp1, lp2, lhgfc1, lhgfc2, FROM_TARGET );
/*       display("lp2->lp1.\n"); */
      HGvtIncludeHGed( hed, lp2, lp1, lhgfc2, lhgfc1, FROM_SRC );
/*       display("end.\n"); */
    }
   
    /* for finding hgvt->inc_ed */
/*     if ( rhgfc2 != NULL ) { */
/*       rharray2 = (HGedArray *) malloc( lp2->lvn * sizeof( HGedArray ) ); */
/*     } */
/*     if ( lhgfc2 != NULL ) { */
/*       lharray2 = (HGedArray *) malloc( lp2->lvn * sizeof( HGedArray ) ); */
/*     } */

/*     i = 0; */
/*     for ( lv = lp2->splv; lv->nxt != (Splv *) NULL; lv = lv->nxt ) { */
/*       if ( rhgfc2 != NULL ) { */
/* 	rharray2[i].ed = hged_in_hgfc( lv->vt, lv->nxt->vt, rhgfc2 ); */
/*       } */
/*       if ( lhgfc2 != NULL ) { */
/* 	lharray2[i].ed = hged_in_hgfc( lv->vt, lv->nxt->vt, lhgfc2 ); */
/*       } */
/*       ++i; */
/*     } */
    /* rhgfc */
/*     if ( rhgfc2 != NULL ) { */
/*       i = 0; */
/*       ed2 = rharray2[i].ed; */
/*       for ( ev = hed->sedvt; ev != (Hedvt *) NULL; ev = ev->nxt ) { */

/* 	if ( ev->vt->from != FROM_TARGET ) { */
/* 	  vt1 = spvt_from_mvt_in_lp1( ev->vt, lp1 ); */
/* 	  hgvt1 = hgvt_in_hgfc( vt1, rhgfc1 ); */
/* 	  hgvt1->inc_ed = ed2; */
/* 	} else { */
/* 	  ++i; */
/* 	  ed2 = rharray2[i].ed; */
/* 	} */
/*       } */
/*       free( rharray2 ); */
/*     } */
    
    /* lhgfc */
/*     if ( lhgfc2 != NULL ) { */
/*       i = 0; */
/*       ed2 = lharray2[i].ed; */
/*       for ( ev = hed->sedvt; ev != (Hedvt *) NULL; ev = ev->nxt ) { */

/* 	if ( ev->vt->from != FROM_TARGET ) { */
/* 	  vt1 = spvt_from_mvt_in_lp1( ev->vt, lp1 ); */
/* 	  hgvt1 = hgvt_in_hgfc( vt1, lhgfc1 ); */
/* 	  hgvt1->inc_ed = ed2; */
/* 	} else { */
/* 	  ++i; */
/* 	  ed2 = lharray2[i].ed; */
/* 	} */
/*       } */
/*       free( lharray2 ); */
/*     } */
    
  }
  
  /* ????? 3D vector ???????????? Hedvt ??????????? */
}

/* hgvt1 ?? hged2 ???????? inc_ed ?????? */
void HGvtIncludeHGed( HEdge *hed, Splp *lp1, Splp *lp2,
		      HGfc *hgfc1, HGfc *hgfc2, int from )
{
  int  i;
  HGedArray *harray;
  Splv *lv;
  Spvt *vt;
  HGed *ed;
  HGvt *hgvt;
  Hedvt *ev;
  HGvt *hgvt_in_hgfc( Spvt *, HGfc * );
  HGed *hged_in_hgfc( Spvt *, Spvt *, HGfc * );
  Spvt *spvt_from_mvt_in_lp( Spvt *, Splp * );
  
  harray = (HGedArray *) malloc( lp2->lvn * sizeof( HGedArray ) );
  i = 0;
  for ( lv = lp2->splv; lv->nxt != (Splv *) NULL; lv = lv->nxt ) {
    harray[i].ed = hged_in_hgfc( lv->vt, lv->nxt->vt, hgfc2 );
    ++i;
  }
  
  i = 0;
  ed = harray[i].ed;
  for ( ev = hed->sedvt; ev != (Hedvt *) NULL; ev = ev->nxt ) {

    if ( ev->vt->from != from ) {
      vt   = spvt_from_mvt_in_lp( ev->vt, lp1 );
      hgvt = hgvt_in_hgfc( vt, hgfc1 );
      hgvt->inc_ed = ed;

/*       if ( hgvt->no == 20 ) { */
/* 	display("hgvt %d inc_ed %d\n", hgvt->no, */
/* 		( hgvt->inc_ed != NULL ) ? hgvt->inc_ed->no : SMDNULL ); */
/*       } */
      
	
/*       if ( hgvt->inc_ed == NULL ) { */
/* 	display("nai!. vt %d\n", hgvt->no ); */
/*       } */
      
      
    } else {
      ++i;
      ed = harray[i].ed;
    }
  }
  
  free( harray );
}
  
/* find ppdvertex from morph ppd vertex in loop */
Spvt *spvt_from_mvt_in_lp( Spvt *mvt, Splp *lp )
{
  Splv *lv;
  
  for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    if ( lv->vt->mvt == mvt ) return lv->vt;
  }
  return (Spvt *) NULL;
}

void lpprm( Splp *lp, int vn, double *prm )
{
  int    i = 0;
  double sum = 0.0;
  Splv   *plv, *lv;
  double V3DistanceBetween2Points( Vec *, Vec * );
  
  lv = lp->splv;
  while (1) {
    prm[i] = sum; ++i;
    plv = lv;
    lv = lv->nxt;
    if ( lv == (Splv *) NULL ) break;
    sum += V3DistanceBetween2Points( &(plv->vt->vec), &(lv->vt->vec) );
  }

  for ( i = 0; i < vn; ++i ) {
    prm[i] /= sum;
  }
}
  
void lpprm_hgfc( Splp *lp, int vn, double *prm, HGfc *hgfc )
{
  int    i = 0;
  double sum = 0.0;
  HGvt   *hv, *phv;
  Splv   *plv, *lv;
/*   Vec2d  uvw1, uvw2; */
  double V2DistanceBetween2Points( Vec2d *, Vec2d * );
  HGvt   *hgvt_in_hgfc( Spvt *, HGfc * );
  
  lv = lp->splv;
  hv = hgvt_in_hgfc( lv->vt, hgfc );
  while ( 1 ) {
    prm[i] = sum; ++i;
    if ( i == vn ) break;
    plv = lv; phv = hv;
    lv = lv->nxt;
    hv = hgvt_in_hgfc( lv->vt, hgfc );
/*     uvw1.x = phv->uvw.x; uvw1.y = phv->uvw.y; */
/*     uvw2.x = hv->uvw.x; uvw2.y = hv->uvw.y; */
    sum += ( V2DistanceBetween2Points( &(phv->uvw), &(hv->uvw) ) );
  }

  for ( i = 0; i < vn; ++i ) {
    prm[i] /= sum;
  }
}

/************************************************************************ */
/************************************************************************ */

void mHGfcOrderBoundaryEdges( HPpd *hppd )
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

/*   display("Ordering --\n"); */
  for ( hf = hppd->shfc; hf != (HFace *) NULL; hf = hf->nxt ) {
/*     display("hfc %d\n", hf->no); */
    hgfc = hf->mhgfc;

    he = hf->shhe;
    do {

      hed  = he->hed;
      shvt = he->vt;
      ehvt = he->nxt->vt;

      lp   = hed->mlp;

      svt = shvt->vt1->mvt;
      evt = ehvt->vt1->mvt;
      shgvt = hgvt_in_hgfc( svt, hgfc );
      ehgvt = hgvt_in_hgfc( evt, hgfc );

      if ( shvt == hed->sv ) reverse = SMD_OFF;
      else                   reverse = SMD_ON;
/*       display("(hgvt) start %d end %d reverse %d\n", shgvt->no, ehgvt->no, reverse ); */

/*       display("\thed %d\n", hed->no); */
      for ( lv = lp->splv; lv->nxt != (Splv *) NULL; lv = lv->nxt ) {
	
	hged = hged_in_hgfc( lv->vt, lv->nxt->vt, hgfc );
	
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

	/* for create HGvted */
	hged->sv->startedge = hged;
	hged->ev->endedge   = hged;
/* 	display("\thged %d sv %d ev %d\n", hged->no, hged->sv->no, hged->ev->no ); */
      }
            
    } while ( (he = he->nxt) != hf->shhe );
  }
  
}
      
/************************************************************************ */
/************************************************************************ */

/* make vertices, edges, faces from HFace */
void hface_to_gppd( HPpd *hppd, Sppd *gppd )
{
  HFace *hfc;
  HGfc  *hgfc1, *hgfc2, *mhgfc;
  void  hgppdface_to_hgppdsurface( HGfc * );
  void  hgfc_to_mhgfc( HGfc *, HGfc *, HGfc * );
  void  mhgfc_to_gppd( HGfc *, Sppd * );
  char  str[BUFSIZ];
  void  hgfc_harmonicmap_to_ps( char *, HGfc * );
  void  tmp_hgfc_harmonicmap_to_ps( char *, HGfc *, HGfc *, HGfc * );
  void  printhgfc( HGfc *, int );
  void  HGfcTriangulation_noEdge( HGfc * );
  
  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {

/*     display("hfc %d\n", hfc->no); */
    hgfc1 = hfc->hgfc1;
    hgfc2 = hfc->hgfc2;
    mhgfc = hfc->mhgfc;

    /* make harmonic map */
    if ( swin->savehmtops ) {
      sprintf( str, "hgfc1hrm%d.ps", hfc->no );
      hgfc_harmonicmap_to_ps( str, hgfc1 );
      sprintf( str, "hgfc2hrm%d.ps", hfc->no );
      hgfc_harmonicmap_to_ps( str, hgfc2 );
    }

    /* make hgsf  */
/*     display("hgfc1\n"); */
    hgppdface_to_hgppdsurface( hgfc1 );
/*     display("hgfc2\n"); */
    hgppdface_to_hgppdsurface( hgfc2 );

/*     printhgfc( hgfc1, SRC ); */
/*     printhgfc( hgfc2, TARGET ); */
    
    /* ?????? hgfc ????? */
    hgfc_to_mhgfc( hgfc1, hgfc2, mhgfc );
    
    HGfcTriangulation_noEdge( mhgfc );

    if ( swin->savehmtops ) {
      sprintf( str, "mhgfchrm%d.ps", hfc->no );
/*       tmp_hgfc_harmonicmap_to_ps( str, hgfc1, hgfc2, mhgfc ); */
      hgfc_harmonicmap_to_ps( str, mhgfc );
    }

    
    /* mhgfc ?????????????????? gppd ???????????? */
    mhgfc_to_gppd( mhgfc, gppd );
    
  }
  
}

/************************************************************************ */

void hgppdface_to_hgppdsurface( HGfc *hgfc )
{
  HGvt *hgvt;
  HGvted *ve;
  
  void makehgvtedlink( HGfc * );
  void HGfcMakeHGsfFromHGvt( HGvt *, HGfc * );
  void HGfcTriangulation( HGfc *hgfc );
  void printhgfc( HGfc *, int );

/*   display("hgfc %d\n", hgfc->id ); */
  /* ?????????????? */
  makehgvtedlink( hgfc );

  for ( hgvt = hgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {

/*     if ( (hgvt->no == 17) || (hgvt->no == 19) ){ */
      
/*       display("hgvt %d en %d\n", hgvt->no, hgvt->hgven ); */

/*       for ( ve = hgvt->shgve; ve != (HGvted *) NULL; ve = ve->nxt ) { */
/* 	display("\ted %d type %d\n", ve->ed->no, ve->ed->sp_type); */
/*       } */
      
/*     } */
    

    HGfcMakeHGsfFromHGvt( hgvt, hgfc );
    
  }
/*   display("(b) sfn %d\n", hgfc->hgsn ); */
/*   printhgfc( hgfc, 0 ); */
  
  HGfcTriangulation( hgfc );
/*   display("(a) sfn %d\n", hgfc->hgsn ); */
/*   printhgfc( hgfc, 0 ); */
}

void makehgvtedlink( HGfc *hgfc )
{
  HGed   *hged;
  void   insert_hgvted( HGvt *, HGed * );
  
  for ( hged = hgfc->shged; hged != (HGed *) NULL; hged = hged->nxt ) {
    
    insert_hgvted( hged->sv, hged );
    insert_hgvted( hged->ev, hged );
    
  }
}

void HGfcMakeHGsfFromHGvt( HGvt *vt, HGfc *hgfc )
{
  HGvted *hgve;
  Boolean HGsfCreateCheck( HGvt *, HGed * );
  void    HGsfCreate( HGvt *, HGed *, HGfc * );

  for ( hgve = vt->shgve; hgve != (HGvted *) NULL; hgve = hgve->nxt ) {

    if ( True == HGsfCreateCheck( vt, hgve->ed ) ) {

      (void) HGsfCreate( vt, hgve->ed, hgfc );

    }
      
  }

}

Boolean HGsfCreateCheck( HGvt *start_vertex, HGed *start_edge )
{
  HGvt *vt, *nvt;
  HGed *ed, *ned;
  HGsf *sf;
  HGsf *HGedLeftFace( HGed *, HGvt * );
  HGvt *HGedAnotherVertex( HGed *, HGvt * );
  HGed *HGvtedNextCCWEdge( HGvt *, HGed * );
  
  vt = start_vertex;
  ed = start_edge;

  if ( (vt == NULL) || (ed == NULL) ) return False;
  while (1) {

/*     display("vt %d ed %d ( sv %d ev %d )\n", vt->no, ed->no, */
/* 	    ed->sv->no, ed->ev->no ); */
    
    if ( ( ed->sp_type == SP_EDGE_BOUNDARY ) && ( ed->ev == vt ) ) {
      /* boundary rotation */
      return False;
    }

    sf = HGedLeftFace( ed, vt );
    if ( sf != (HGsf *) NULL ) {
      /* already exist HGsf */
      return False;
    }
      
    nvt = HGedAnotherVertex( ed, vt );
    ned = HGvtedNextCCWEdge( nvt, ed );

    if ( nvt == start_vertex ) {
      return True;
    }
    vt = nvt;
    ed = ned;
  }
  
}
  
void HGsfCreate( HGvt *start_vertex, HGed *start_edge, HGfc *hgfc )
{
  HGvt *vt, *nvt;
  HGed *ed, *ned;
  HGsf *sf;
  HGhe *he;
  HGsf *create_hgppdsurface( HGfc * );
  HGhe *create_hgppdhalfedge( HGsf * );
  void HGedAppendLeftFace( HGed *, HGvt *, HGsf * );
  HGvt *HGedAnotherVertex( HGed *, HGvt * );
  HGed *HGvtedNextCCWEdge( HGvt *, HGed * );

  sf = create_hgppdsurface( hgfc );
/*   display("sf %d\n", sf->no); */
  
  vt = start_vertex;
  ed = start_edge;

  while (1) {

    he = create_hgppdhalfedge( sf );
    he->vt = vt;
    he->ed = ed;
    HGedAppendLeftFace( ed, vt, sf );
/*     display("\tvt %d ed %d\n", vt->no, ed->no); */
    
    nvt = HGedAnotherVertex( ed, vt );
    ned = HGvtedNextCCWEdge( nvt, ed );

    if ( nvt == start_vertex ) {
      return;
    }
    vt = nvt;
    ed = ned;
  }
  
}
  
/************************************************************************ */

/* Merge Two hgfcs */
void hgfc_to_mhgfc( HGfc *hgfc1, HGfc *hgfc2, HGfc *mhgfc )
{
  HGvt *hgvt;
  HGvted *ve;
  void hgvt_to_mhgvt( HGfc *, HGfc *, int );
  void hged_to_mhged( HGfc *, HGfc *, HGfc * );
  void mhgfc_create_mhgsf( HGfc * );
  void CalcHGsfNorm( HGfc * );
  
  /* hgvt ????????? mhgfc ????? */
  hgvt_to_mhgvt( hgfc1, mhgfc, SRC );
  hgvt_to_mhgvt( hgfc2, mhgfc, TARGET );

  /* ????????????? */
  hged_to_mhged( hgfc1, hgfc2, mhgfc );

/*   for ( hgvt = mhgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) { */

/*     display("hgvt %d type %d\n", hgvt->no, hgvt->sp_type ); */

/*     for ( ve = hgvt->shgve; ve != (HGvted *) NULL; ve = ve->nxt ) { */

/*       display("\ted %d type %d\n", ve->ed->no, ve->ed->sp_type ); */
/*     } */
/*   } */
  /* mhged ??????????????mhgsf ?????? */
  mhgfc_create_mhgsf( mhgfc );

/*   CalcHGsfNorm( mhgfc ); */
}  

/***********************************/

/* create mvt from hgvt */

void hgvt_to_mhgvt( HGfc *hgfc, HGfc *mhgfc, int from )
{
  HGvt *hgvt, *mhgvt;
  HGvt *create_hgppdvertex( HGfc * );
  
  for ( hgvt = hgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {

    if ( hgvt->vt->sp_type == SP_VERTEX_NORMAL ) {
      
      /* create mhgvt */
      mhgvt = create_hgppdvertex( mhgfc );
      mhgvt->uvw.x = hgvt->uvw.x;
      mhgvt->uvw.y = hgvt->uvw.y;
      mhgvt->from  = from;
      hgvt->mhgvt = mhgvt;
    }
    
  }
}

/***********************************/

void hged_to_mhged( HGfc *hgfc1, HGfc *hgfc2, HGfc *mhgfc )
{
  char  str[BUFSIZ];
  void hgfc_edge_edge_intersection( HGfc *, HGfc *, HGfc * );
  void mhgfc_create_mhged( HGfc *, HGfc *, HGfc * );
  void  tmp_hgfc_harmonicmap_to_ps( char *, HGfc *, HGfc *, HGfc * );
  
  /** ????????????????????????????????????????? **/
  /* ??????????????????????mhgfc ????????????????? */
/*   display("edge_edge_intersection\n"); */
  hgfc_edge_edge_intersection( hgfc1, hgfc2, mhgfc );
/*   display("ee end.\n"); */

/*   if ( swin->savehmtops ) { */
/*     sprintf( str, "mhgfchrm%dee.ps", hgfc1->id ); */
/*     tmp_hgfc_harmonicmap_to_ps( str, hgfc1, hgfc2, mhgfc ); */
/*     hgfc_harmonicmap_to_ps( str, mhgfc ); */
/*   } */
  /* hged ????????????????????mhgfc ??????????????? */
/*   display("create_mhged\n"); */
  mhgfc_create_mhged( hgfc1, hgfc2, mhgfc );
/*   display("cm end.\n"); */
}

/* Edge-Edge Intersection */
/*
  This algorithm is based on:
  James R. Kent, Wayne E. Carlson and Richard E. Parent,
  "Shape Transformation for Polyhedral Objects", in SIGGRAPH'92
  - 5.2 The Merging Algorithm -
*/
/* 
  isLineSegmentCrossing ???????????????
*/

/*
  (sv->sp_type == SP_VERTEX_BOUNDARY) || (ev->sp_type != SP_VERTEX_BOUNDARY)  
  ??????????????????????
  -> ??????????
*/

void hgfc_edge_edge_intersection( HGfc *hgfc1, HGfc *hgfc2, HGfc *mhgfc )
{
  int  flag;
  HGed *ed, *e1, *e2;
  HGsf *sf, *nf, *f2;
  HGvt *sv, *ev, *mhgvt, *tmp_vt;
  HGhe *he;
  HGvted *vted;
  HGedvt *hgedvt1, *hgedvt2;
  HGedList *edgelist1, *edgelist2;
  HGedList *create_hgedlist( void );
  void     free_hgedlist( HGedList * );
  HGedNode *create_hgednode( HGed *, HGedList * );
  void     free_start_hgednode( HGedList * );
  HGvt *create_hgppdvertex( HGfc * );
  HGsf *another_hgsf( HGed *, HGsf * );
  HGsf *find_hgsf_in_hgfc( HGvt *, HGfc * );
  HGsf   *includeHGsf( HGvt * );
  HGedvt *create_hgedvt(HGvt *, HGed *);
  Boolean isHGvtLineSegmentCrossing( HGvt *, HGvt *, HGvt *, HGvt * );
  Boolean isHGvtLeftSide( HGvt *, HGvt *, HGvt * );
/*   Boolean isLineSegmentCrossing( Vec2d *, Vec2d *, Vec2d *, Vec2d * ); */
/*   Boolean isLeftSide( Vec2d *, Vec2d *, Vec2d * ); */
  Boolean isEdgeValidforCandidate( HGed *, HGed * );
  void    LineIntersectionPoint( Vec2d *, Vec2d *, Vec2d *, Vec2d *, Vec2d * );

  /* initialize hged in hgfc1 */
  for ( ed = hgfc1->shged; ed != (HGed *) NULL; ed = ed->nxt ) {
    if ( ed->sp_type != SP_EDGE_BOUNDARY ) ed->pass = SMD_ON;
    else ed->pass = SMD_OFF;
  }

  for ( ed = hgfc1->shged; ed != (HGed *) NULL; ed = ed->nxt ) {

    if ( ed->pass == SMD_ON ) {

      edgelist1 = create_hgedlist();
      (void) create_hgednode( ed, edgelist1 );
      ed->pass = SMD_OFF;

      flag = SMD_OFF;
      if ( (sf = find_hgsf_in_hgfc( ed->sv, hgfc2 )) != (HGsf *) NULL ) {
	if ( ed->sv->sp_type == SP_VERTEX_NORMAL ) {
	  ed->sv->inc_sf = sf;
	}
	flag = SMD_ON;
      }
      if ( (sf = find_hgsf_in_hgfc( ed->ev, hgfc2 )) != (HGsf *) NULL ) {
	if ( ed->ev->sp_type == SP_VERTEX_NORMAL ) {
	  ed->ev->inc_sf = sf;
	}
	flag = SMD_ON;
      }

      if ( !flag ) {
	free_hgedlist( edgelist1 );
	continue;
      }
      
/*       display("ed1 %d sv %d type %d inc_sf %d ev %d type %d inc_sf %d\n", */
/* 	      ed->no, */
/* 	      ed->sv->no, */
/* 	      ed->sv->sp_type, */
/* 	      (ed->sv->inc_sf != NULL) ? ed->sv->inc_sf->no : SMDNULL, */
/* 	      ed->ev->no, */
/* 	      ed->ev->sp_type, */
/* 	      (ed->ev->inc_sf != NULL) ? ed->ev->inc_sf->no : SMDNULL */
/* 	      ); */

/*       display("sf found. %d\n", sf->no ); */
      
      while ( edgelist1->en ) {

	e1 = edgelist1->senode->ed;

	
	if ( (f2 = includeHGsf( e1->sv )) != (HGsf *) NULL ) {
	  sv = e1->sv;
	  ev = e1->ev;
	} else if ( (f2 = includeHGsf( e1->ev )) != (HGsf *) NULL ) {
	  sv = e1->ev;
	  ev = e1->sv;
	  /* swap */
	  tmp_vt = e1->sv;
	  e1->sv = e1->ev;
	  e1->ev = tmp_vt;
	} else {
	  free_start_hgednode( edgelist1 );
	  continue;
	}
	
/* 	display("\ttest begin. e1 %d sv %d inc_sf %d ev %d inc_sf %d (f2 %d) \n", */
/* 		e1->no, */
/* 		sv->no, */
/* 		(sv->inc_sf != NULL) ? sv->inc_sf->no : SMDNULL, */
/* 		ev->no, */
/* 		(ev->inc_sf != NULL) ? ev->inc_sf->no : SMDNULL, */
/* 		f2->no */
/* 		); */

	/* add the edges originating at v1 to edgelist */
	if ( sv->sp_type != SP_VERTEX_HVERTEX ) {
	  for ( vted = sv->shgve; vted != (HGvted *) NULL; vted = vted->nxt ) {
	    if ( vted->ed->pass == SMD_ON ) {
/* 	      display("(sv)\ted1 candidate %d sv %d inc_sf %d ev %d inc_sf %d\n", */
/* 		      vted->ed->no, */
/* 		      vted->ed->sv->no,	 */
/* 		      (vted->ed->sv->inc_sf != NULL) ? vted->ed->sv->inc_sf->no : SMDNULL, */
/* 		      vted->ed->ev->no, */
/* 		      (vted->ed->ev->inc_sf != NULL) ? vted->ed->ev->inc_sf->no : SMDNULL */
/* 		      ); */

	      (void) create_hgednode( vted->ed, edgelist1 );
	      vted->ed->pass = SMD_OFF;
	    }
	  }
	}

	edgelist2 = create_hgedlist();
	he = f2->shghe;
	do {
/* 	  display("ed %d\n", he->ed->no); */
	  if ( True == isEdgeValidforCandidate( e1, he->ed ) ) {
	    (void) create_hgednode( he->ed, edgelist2 );
	  }
	} while ( (he = he->nxt) != f2->shghe );

	/* ??????????????????????? */
	while ( edgelist2->en ) {

	  e2 = edgelist2->senode->ed;
/* 	  display("\ttest: e1 %d e2 %d\n", e1->no, e2->no ); */

/* 	  display("e1 (sv) %g %g (ev) %g %g\n", */
/* 		  e1->sv->uvw.x, e1->sv->uvw.y, e1->ev->uvw.x, e1->ev->uvw.y ); */
/* 	  display("e2 (sv) %g %g (ev) %g %g\n", */
/* 		  e2->sv->uvw.x, e2->sv->uvw.y, e2->ev->uvw.x, e2->ev->uvw.y ); */
/* 	  if ( isLineSegmentCrossing( &(sv->uvw), &(ev->uvw), */
/* 				      &(e2->sv->uvw), &(e2->ev->uvw) ) == True ) { */

	  if ( isHGvtLineSegmentCrossing( sv, ev, e2->sv, e2->ev) == True ) {

/* 	    display("\tintersection found.\n"); */
	    /* create hgppdvertex */
	    mhgvt = create_hgppdvertex( mhgfc );
/* 	    display("mhgvt %d\n", mhgvt->no ); */
	    /* ?????????????? */
/* 	    mhgvt->vt = mvt; */
	    mhgvt->sp_type = SP_VERTEX_NORMAL;
	    mhgvt->from = FROM_INTSEC;
	    LineIntersectionPoint( &(sv->uvw), &(ev->uvw),
				   &(e2->sv->uvw), &(e2->ev->uvw),
				   &(mhgvt->uvw) );

	    /* insert hgedvt in e1, e2 */
	    /* hged2 ???????hged1 ?? rv ?? lv ??????????? */
	    /* create vertex->edge in counter-clockwise */
	    hgedvt1 = create_hgedvt( mhgvt, e1 );
	    hgedvt1->intsec_ed = e2;
	    hgedvt1->rf = e2->rf;
	    hgedvt1->lf = e2->lf;
	    /* set edge rotation */
/* 	    if ( True == isLeftSide( &(e1->sv->uvw), &(e1->ev->uvw), &(e2->sv->uvw) ) ) { */
	    if ( True == isHGvtLeftSide( e1->sv, e1->ev, e2->sv ) ) {
	      hgedvt1->rotation = EDGE_END;
	    } else {
	      hgedvt1->rotation = EDGE_START;
	    }
	      
	    hgedvt2 = create_hgedvt( mhgvt, e2 );
	    hgedvt2->intsec_ed = e1;
	    hgedvt2->rf = e1->rf;
	    hgedvt2->lf = e1->lf;
	    /* set edge rotation */
/* 	    if ( True == isLeftSide( &(e2->sv->uvw), &(e2->ev->uvw), &(e1->sv->uvw) ) ) { */
	    if ( True == isHGvtLeftSide( e2->sv, e2->ev, e1->sv ) ) {
	      hgedvt2->rotation = EDGE_END;
	    } else {
	      hgedvt2->rotation = EDGE_START;
	    }

	    hgedvt1->mate = hgedvt2;
	    hgedvt2->mate = hgedvt1;

	    nf = another_hgsf( e2, f2 );

	    if ( nf == (HGsf *) NULL ) break;
	    
	    he = nf->shghe;
	    do {
	      if ( he->ed != e2 ) {
		if ( True == isEdgeValidforCandidate( e1, he->ed ) ) {
		  (void) create_hgednode( he->ed, edgelist2 );
		}
	      }
	    } while ( (he = he->nxt) != nf->shghe );

	    f2 = nf;
	  }
	  free_start_hgednode( edgelist2 );
	}


	if ( ev->sp_type == SP_VERTEX_NORMAL ) {
	  ev->inc_sf = f2;
/* 	  display("\tattach: ev %d inc_sf %d\n", ev->no, ev->inc_sf->no ); */
	}
	if ( ev->sp_type != SP_VERTEX_HVERTEX ) {
	  for ( vted = ev->shgve; vted != (HGvted *) NULL; vted = vted->nxt ) {
	    if ( vted->ed->pass == SMD_ON ) {
/* 	      display("(ev)\ted1 candidate %d sv %d inc_sf %d ev %d inc_sf %d\n", */
/* 		      vted->ed->no, */
/* 		      vted->ed->sv->no,	 */
/* 		      (vted->ed->sv->inc_sf != NULL) ? vted->ed->sv->inc_sf->no : SMDNULL, */
/* 		      vted->ed->ev->no, */
/* 		      (vted->ed->ev->inc_sf != NULL) ? vted->ed->ev->inc_sf->no : SMDNULL */
/* 		      ); */
	      (void) create_hgednode( vted->ed, edgelist1 );
	      vted->ed->pass = SMD_OFF;
	    }
	  }
	}

	free_hgedlist( edgelist2 );
	free_start_hgednode( edgelist1 );


      } /* while ( edgelist1->en ) */

      free_hgedlist( edgelist1 );
      
    } /* if ( ed->pass == SMD_ON ) */


  }
  
  /* ????? 3D vector ???????????? HGedvt ??????????? */
}

Boolean isEdgeValidforCandidate( HGed *e1, HGed *e2 )
{
  
  if ( e2->sp_type == SP_EDGE_BOUNDARY ) return False;
  
  if ( e2->sv->sp_type == SP_VERTEX_HVERTEX ) {
    if ( e1->sv->sp_type == SP_VERTEX_HVERTEX ) {
      if ( e2->sv->mhgvt == e1->sv->mhgvt ) return False;
    }
    if ( e1->ev->sp_type == SP_VERTEX_HVERTEX ) {
      if ( e2->sv->mhgvt == e1->ev->mhgvt ) return False;
    }
  }
    
  if ( e2->ev->sp_type == SP_VERTEX_HVERTEX ) {
    if ( e1->sv->sp_type == SP_VERTEX_HVERTEX ) {
      if ( e2->ev->mhgvt == e1->sv->mhgvt ) return False;
    }
    if ( e1->ev->sp_type == SP_VERTEX_HVERTEX ) {
      if ( e2->ev->mhgvt == e1->ev->mhgvt ) return False;
    }
  }

  return True;
}

HGsf *includeHGsf( HGvt *vt )
{

  HGed *hged;
  
  if ( vt->sp_type == SP_VERTEX_NORMAL ) {

    if ( vt->inc_sf != (HGsf *) NULL ) return vt->inc_sf;
    else return (HGsf *) NULL;

  } else if ( vt->sp_type == SP_VERTEX_BOUNDARY ) {

    hged = vt->inc_ed;
/*     if ( hged == NULL ) return NULL; */
    if ( hged->rf != (HGsf *) NULL ) return hged->rf;
    else return hged->lf;

  } 

  return (HGsf *) NULL;

}
    
HGsf *find_hgsf_in_hgfc( HGvt *hgvt, HGfc *hgfc )
{
  HGsf    *sf;
  HGed    *hged;
  Boolean isPointinHGsf( HGsf *, Vec2d * );
  Boolean isHGvtinHGsf( HGsf *, HGvt * );

/*   if ( hgvt->no == 57 ) { */
/*     display("(find_hgsf_in_hgfc) hgvt %d\n", hgvt->no ); */
/*   } */
  if ( hgvt->sp_type == SP_VERTEX_NORMAL ) {

    /* Kent ?????????? */
    if ( hgvt->inc_sf != (HGsf *) NULL ) return hgvt->inc_sf;
    
    for ( sf = hgfc->shgsf; sf != (HGsf *) NULL; sf = sf->nxt ) {
      
/*       if ( isPointinHGsf( sf, &(hgvt->uvw) ) == True ) return sf; */
      if ( isHGvtinHGsf( sf, hgvt ) == True ) return sf;
	  
    }
    return (HGsf *) NULL;

  } else if ( hgvt->sp_type == SP_VERTEX_BOUNDARY ) {

    /* use boundary information */
    /* ?????????????????????????????? */
    /* hedge_to_gppd ???????????????????????????????? */
    
    hged = hgvt->inc_ed;
/*     display("\tinc ed found. %d\n", hged->no ); */
/*     if ( hged == NULL ) { */
      
/*       display("nai!. vt %d\n", hgvt->no ); */
/*     } */
    
    if ( hged->rf != (HGsf *) NULL ) return hged->rf;
    else return hged->lf;

  } else {

    return (HGsf *) NULL;

  }

}

/* HGedvtSort frees and rebuilds hgfc2 HGedvt nodes; hgfc1 still points at the
 * old mates. Restore symmetric links via intsec_ed and the shared mhgvt. */
static void mhgfc_relink_cross_mesh_mates( HGfc *hgfc1 )
{
  HGed *hged;
  HGedvt *hgev, *hgev2;

  for ( hged = hgfc1->shged; hged != (HGed *) NULL; hged = hged->nxt ) {
    for ( hgev = hged->shgev; hgev != (HGedvt *) NULL; hgev = hgev->nxt ) {
      HGed *e2;
      if ( hgev->intsec_ed == (HGed *) NULL ) continue;
      e2 = hgev->intsec_ed;
      for ( hgev2 = e2->shgev; hgev2 != (HGedvt *) NULL; hgev2 = hgev2->nxt ) {
        if ( hgev2->intsec_ed == hged && hgev2->vt == hgev->vt ) {
          hgev->mate = hgev2;
          hgev2->mate = hgev;
          break;
        }
      }
    }
  }
}

void mhgfc_create_mhged( HGfc *hgfc1, HGfc *hgfc2, HGfc *mhgfc )
{
  HGvt *hgvt;
  HGed *hged;
  void HGedvtSort( HGed * );
  void HGfcCreatemHGed( HGfc *, HGfc * );
  void mHGfcCreateHGvted( HGfc *, HGfc *, HGfc * );
  void HVertexCreateHGvted( HGfc * );
  HGsf *find_hgsf_in_hgfc( HGvt *, HGfc * );
  
  /* sort H2's intersection points */
  /* H1's points have already sorted. */
  /* decide hgvt1's inc_sf ( used in "hface_calc_morph_vector"(morphvec.c) ) */
  /* ?????????????????? */

/*   display("aa\n"); */
  
  for ( hged = hgfc2->shged; hged != (HGed *) NULL; hged = hged->nxt ) {
/*     display("(sort) hged %d sv %d ev %d evn %d\n", */
/* 	    hged->no, */
/* 	    hged->sv->no, */
/* 	    hged->ev->no, */
/* 	    hged->hgevn ); */
    HGedvtSort( hged );
/*     display("(a) hged %d evn %d\n", hged->no, hged->hgevn ); */
  }
  mhgfc_relink_cross_mesh_mates( hgfc1 );

/*   display("bb\n"); */
/*   for ( hgvt = hgfc2->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) { */
/*     if ( (hgvt->sp_type == SP_VERTEX_NORMAL) && (hgvt->inc_sf == NULL) ) { */
/*       hgvt->inc_sf = find_hgsf_in_hgfc( hgvt, hgfc1 ); */
/*     } */
/*   } */
  
/*   display("cc\n"); */
  /* create mhged */
  HGfcCreatemHGed( hgfc1, mhgfc );
  HGfcCreatemHGed( hgfc2, mhgfc );

/*   display("dd\n"); */
  /* create mhgvt->mhged links */
  mHGfcCreateHGvted( hgfc1, hgfc2, mhgfc );

/*   display("ee\n"); */
  /* re-order hgvted */
  /* ?????????????? */
  HVertexCreateHGvted( mhgfc );
/*   display("ff\n"); */
}

void HGedvtSort( HGed *hged )
{
  int        i;
  Boolean    reverse;
  HGsf       *sf, *ef;
  HGedvt     *hgev, *nhgev;
  HGedvtList *hgedvtlist;
  HGedvtNode *node, *anode;
  HGedvtNode *create_start_hgedvtnode( HGedvtList * );
  HGedvtNode *create_end_hgedvtnode( HGedvtList * );
  HGedvt *create_hgedvt( HGvt *, HGed * );
  void free_hgedvt( HGedvt *, HGed * );
  HGedvtList *create_hgedvtlist( void );
  Boolean HGedvtDecideOrder( HGvt *, HGvt *, HGsf *, HGsf * );
  void free_hgedvtlist( HGedvtList * );

  if ( !(hged->hgevn) ) return;
  
  hgedvtlist = create_hgedvtlist();
  /* insert first hgedvt to hgedvtlist */
  hgev = hged->shgev;
  node = create_end_hgedvtnode( hgedvtlist );
  node->intsec_ed = hgev->intsec_ed;
  node->vt = hgev->vt;
  node->sf = hgev->lf;
  node->ef = hgev->rf;
  node->mate = hgev->mate;
  free_hgedvt( hgev, hged );

  sf = node->sf;
  ef = node->ef;

/*   display("\nhged %d sv %d type %d ev %d type %d\n", hged->no, */
/* 	  hged->sv->no, hged->sv->sp_type, hged->ev->no, hged->ev->sp_type); */
  
  while ( hged->hgevn ) {

/*     display("hged->hgevn %d \n", hged->hgevn); */

/*     for ( anode = hgedvtlist->senode; anode != (HGedvtNode *) NULL; anode = anode->nxt ) { */
/*       display("node %vt %d sf %d ef %d\n", anode->vt->no, anode->sf->no, anode->ef->no); */
/*     } */
/*     display("hgev->rf %d hgev->lf %d\n", hged->shgev->rf->no, hged->shgev->lf->no ); */
      
    
    for ( hgev = hged->shgev; hgev != (HGedvt *) NULL; hgev = nhgev ) {

      nhgev = hgev->nxt;
	
      if ( sf == hgev->lf ) {
	/* change surface */
	sf = hgev->rf;
	node = create_start_hgedvtnode( hgedvtlist );
	node->intsec_ed = hgev->intsec_ed;
	node->vt = hgev->vt;
	node->sf = hgev->rf;
	node->ef = hgev->lf;
	node->mate = hgev->mate;
	node->rotation = hgev->rotation;
	free_hgedvt( hgev, hged );
	continue;
      } else if ( sf == hgev->rf ) {
	/* change surface */
	sf = hgev->lf;
	node = create_start_hgedvtnode( hgedvtlist );
	node->intsec_ed = hgev->intsec_ed;
	node->vt = hgev->vt;
	node->sf = hgev->lf;
	node->ef = hgev->rf;
	node->mate = hgev->mate;
	node->rotation = hgev->rotation;
	free_hgedvt( hgev, hged );
	continue;
      }
      if ( ef == hgev->lf ) {
	/* change surface */
	ef = hgev->rf;
	node = create_end_hgedvtnode( hgedvtlist );
	node->intsec_ed = hgev->intsec_ed;
	node->vt = hgev->vt;
	node->sf = hgev->lf;
	node->ef = hgev->rf;
	node->mate = hgev->mate;
	node->rotation = hgev->rotation;
	
	free_hgedvt( hgev, hged );
	continue;
      } else if ( ef == hgev->rf ) {
	/* change surface */
	ef = hgev->lf;
	node = create_end_hgedvtnode( hgedvtlist );
	node->intsec_ed = hgev->intsec_ed;
	node->vt = hgev->vt;
	node->sf = hgev->rf;
	node->ef = hgev->lf;
	node->mate = hgev->mate;
	node->rotation = hgev->rotation;
	free_hgedvt( hgev, hged );
	continue;
      }
    }
    
  } /*  while ( hged->hgevn ) */

  /* decide order */
  /* ????????????????? */
  reverse = HGedvtDecideOrder( hged->sv, hged->ev, sf, ef );
  
/*   if ( (hged->sv->no == 57) ||  (hged->ev->no == 57) ) { */
/*     display("sv %d ev %d sf %d ef %d\n", */
/* 	    hged->sv->no, hged->ev->no, */
/* 	    sf->no, ef->no ); */
/*   } */

  /* decide include sf */
  if ( reverse == False ) {
    if ( hged->sv->sp_type == SP_VERTEX_NORMAL ) {
      hged->sv->inc_sf = sf;
    }
    if ( hged->ev->sp_type == SP_VERTEX_NORMAL ) {
      hged->ev->inc_sf = ef;
    }
  } else {
    if ( hged->sv->sp_type == SP_VERTEX_NORMAL ) {
      hged->sv->inc_sf = ef;
    }
    if ( hged->ev->sp_type == SP_VERTEX_NORMAL ) {
      hged->ev->inc_sf = sf;
    }
  }

  /* re-create hgedvt */
/*   display("hged %d reverse %d\n", hged->no, reverse ); */
  if ( reverse == False ) {
    for ( node = hgedvtlist->senode; node != (HGedvtNode *) NULL; node = node->nxt ) {
      hgev = create_hgedvt( node->vt, hged );
      hgev->lf = node->sf;
      hgev->rf = node->ef;
      hgev->intsec_ed = node->intsec_ed;
      hgev->mate = node->mate;
      hgev->rotation = node->rotation;
/*       display("\tvt %d sf %d ef %d\n", hgev->vt->no, hgev->lf->no, hgev->rf->no ); */
    }
  } else {
    for ( node = hgedvtlist->eenode; node != (HGedvtNode *) NULL; node = node->prv ) {
      hgev = create_hgedvt( node->vt, hged );
      hgev->lf = node->ef;
      hgev->rf = node->sf;
      hgev->intsec_ed = node->intsec_ed;
      hgev->mate = node->mate;
      hgev->rotation = node->rotation;
/*       display("\tvt %d sf %d ef %d\n", hgev->vt->no, hgev->lf->no, hgev->rf->no ); */
    }
  }
  
  free_hgedvtlist( hgedvtlist );
  
}

Boolean HGedvtDecideOrder( HGvt *sv, HGvt *ev, HGsf *sf, HGsf *ef )
{
  HGed *inc_ed;
  HGsf *fc;
  Boolean isPointinHGsf( HGsf *, Vec2d * );
  Boolean isHGvtinHGsf( HGsf *, HGvt * );
  
  if ( sf == ef ) return False;
  
  if ( sv->sp_type == SP_VERTEX_NORMAL ) {
    if ( isHGvtinHGsf( sf, sv ) == True  ) return False;
    if ( isHGvtinHGsf( ef, sv ) == True  ) return True;
  }
    
  if ( ev->sp_type == SP_VERTEX_NORMAL ) {
    if ( isHGvtinHGsf( ef, ev ) == True  ) return False;
    if ( isHGvtinHGsf( sf, ev ) == True  ) return True;
  }
    
  if ( sv->sp_type == SP_VERTEX_BOUNDARY ) {
    inc_ed = sv->inc_ed;
    fc = ( inc_ed->lf != (HGsf *) NULL ) ? inc_ed->lf : inc_ed->rf;
    if ( fc == sf ) return False;
    if ( fc == ef ) return True;
  }
    
  if ( ev->sp_type == SP_VERTEX_BOUNDARY ) {
    inc_ed = ev->inc_ed;
    fc = ( inc_ed->lf != (HGsf *) NULL ) ? inc_ed->lf : inc_ed->rf;
    if ( fc == ef ) return False;
    if ( fc == sf ) return True;
  }
    
  /* ??????????????????????????? */
/*   display("okasii! in HGedvtDecideOrder.\n"); */
  return False;
}    
    
void HGfcCreatemHGed( HGfc *hgfc, HGfc *mhgfc )
{
  HGed *hged;
  void HGedCreatemHGed( HGed *, HGfc * );
  
  for ( hged = hgfc->shged; hged != (HGed *) NULL; hged = hged->nxt ) {
/*     display("(b) hged %d type %d evn %d\n", hged->no, hged->sp_type, hged->hgevn ); */
    HGedCreatemHGed( hged, mhgfc );
/*     display("(a) hged %d type %d een %d\n", hged->no, hged->sp_type, hged->mhgedn ); */

  }
}

void HGedCreatemHGed( HGed *hged, HGfc *mhgfc )
{
  HGed *mhged, *prv_mhged;
  HGeded *hgee;
  HGedvt *hgev, *mate_hgev;
  HGvt *hgvt_in_hgfc( Spvt *, HGfc * );
  HGed *create_hgppdedge( HGfc * );
  HGeded *create_hgeded( HGed * );
  
  if ( hged->sp_type != SP_EDGE_NORMAL ) return;

  mhged = create_hgppdedge( mhgfc );
/*   mhged->sv = hgvt_in_hgfc( hged->sv->mhgvt, mhgfc ); */
  mhged->sv = hged->sv->mhgvt;
  hgee = create_hgeded( hged );
  hgee->mhged = mhged;
/*   display("create.\n"); */

  for ( hgev = hged->shgev; hgev != (HGedvt *) NULL; hgev = hgev->nxt ) {

    prv_mhged = mhged;
    prv_mhged->ev = hgev->vt;
    mhged = create_hgppdedge( mhgfc );
    mhged->sv = hgev->vt;
    hgee = create_hgeded( hged );
    hgee->mhged = mhged;
/*     display("create.\n"); */

    hgev->mhged[0] = prv_mhged;
    hgev->mhged[2] = mhged;

    mate_hgev = hgev->mate;

    if ( mate_hgev->rotation == EDGE_START ) {
      mate_hgev->mhged[1] = prv_mhged;
      mate_hgev->mhged[3] = mhged;
    } else {
      mate_hgev->mhged[1] = mhged;
      mate_hgev->mhged[3] = prv_mhged;
    }
  }
  mhged->ev = hged->ev->mhgvt;
}

void mHGfcCreateHGvted( HGfc *hgfc1, HGfc *hgfc2, HGfc *mhgfc )
{
  int  i;
  HGed *hged;
  HGedvt *hgev;
  HGvted *mvted;
  HGvt *mhgvt;
  void HGfcCreateHGvted( HGfc *, HGfc * );
  HGvted *create_hgvted(HGvt *, HGed *);
  
/*   display("bb-a\n"); */
  HGfcCreateHGvted( hgfc1, mhgfc );
/*   display("bb-b\n"); */
  HGfcCreateHGvted( hgfc2, mhgfc );
/*   display("bb-c\n"); */

  for ( hged = hgfc1->shged; hged != (HGed *) NULL; hged = hged->nxt ) {
    
    for (hgev = hged->shgev; hgev != (HGedvt *) NULL; hgev = hgev->nxt ) {

      mhgvt = hgev->vt;

      for ( i = 0; i < NUM_INTERSECTION_EDGE; ++i ) {
	mvted = create_hgvted( mhgvt, hgev->mhged[i] );
      }
    }
  }
/*   display("bb-d\n"); */
}

void HGfcCreateHGvted( HGfc *hgfc, HGfc *mhgfc )
{
  HGvt   *hgvt, *mhgvt;
  HGed   *start_hged, *end_hged, *hged;
  HGvted *mvted, *ve, *start_ve, *end_ve;
  HGvted *create_hgvted(HGvt *, HGed *);
  HGvted *find_hgvted( HGvt *, HGed * );
  HGed   *find_hgvted_startedge_boundary( HGvt * );
  HGed   *find_hgvted_endedge_boundary( HGvt * );

  for ( hgvt = hgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {

    /* ???????????????????????????? */
    mhgvt = hgvt->mhgvt;
    
/*     display("hgvt %d type %d\n", hgvt->no, hgvt->sp_type ); */
      
    if ( hgvt->sp_type != SP_VERTEX_NORMAL ) {

      /* startedge, endedge are created in "mHGfcOrderBoundaryEdges" */
      
      /* store start edge */
      /* consider in the case of duplicate edges */
      if ( hgvt->sp_type == SP_VERTEX_HVERTEX ) {
	if ( (mvted = find_hgvted( mhgvt, mhgvt->startedge )) == NULL ) {
	  mvted = create_hgvted( mhgvt, mhgvt->startedge );
	}
      } else {
	mvted = create_hgvted( mhgvt, mhgvt->startedge );
      }

      start_hged = find_hgvted_startedge_boundary( hgvt );
      start_ve   = find_hgvted( hgvt, start_hged );
      end_hged   = find_hgvted_endedge_boundary( hgvt );
      end_ve     = find_hgvted( hgvt, end_hged );

/*       display("hged start %d end %d\n", start_hged->no, end_hged->no ); */
      
      ve = start_ve;
      
      while ( 1 ) {

	ve = ve->nxt;
	if ( ve == (HGvted *) NULL ) ve = hgvt->shgve;
	if ( ve == end_ve ) break;

	/* store normal edge */
	hged = ve->ed;
	if ( hged->sv == hgvt ) {
	  mvted = create_hgvted( mhgvt, hged->smhged->mhged );
	} else {
	  mvted = create_hgvted( mhgvt, hged->emhged->mhged );
	}

      }
      
      /* store end edge */
      /* consider in the case of duplicate edges */
      if ( hgvt->sp_type == SP_VERTEX_HVERTEX ) {
	if ( (mvted = find_hgvted( mhgvt, mhgvt->endedge )) == NULL ) {
	  mvted = create_hgvted( mhgvt, mhgvt->endedge );
	}
      } else {
	mvted = create_hgvted( mhgvt, mhgvt->endedge );
      }

    } else {

      for ( ve = hgvt->shgve; ve != (HGvted *) NULL; ve = ve->nxt ) {

	hged = ve->ed;
	if ( hged->sv == hgvt ) {
	  mvted = create_hgvted( mhgvt, hged->smhged->mhged );
	} else {
	  mvted = create_hgvted( mhgvt, hged->emhged->mhged );
	}

      }
      
    }
  }
	
}

HGed *find_hgvted_startedge_boundary( HGvt *hgvt )
{
  HGed   *ed;
  HGvted *ve;
  
  for ( ve = hgvt->shgve; ve != (HGvted *) NULL; ve = ve->nxt ) {
    ed = ve->ed;
    if ( ed->sp_type == SP_EDGE_BOUNDARY ) {
      if ( ed->sv == hgvt ) return ed;
    }
  }
  return NULL;
}

HGed *find_hgvted_endedge_boundary( HGvt *hgvt )
{
  HGed   *ed;
  HGvted *ve;
  
  for ( ve = hgvt->shgve; ve != (HGvted *) NULL; ve = ve->nxt ) {
    ed = ve->ed;
    if ( ed->sp_type == SP_EDGE_BOUNDARY ) {
      if ( ed->ev == hgvt ) return ed;
    }
  }
  return NULL;
}

void HVertexCreateHGvted( HGfc *hgfc )
{
  int n, i;
  HGvted *ve;
  HGvt *hgvt, *v;
  HGed  *ed;
  HGedArray *harray;
  void free_hgppdvertexedge( HGvt * );
  HGvted *create_hgvted( HGvt *, HGed * );
  HGvted *InsertHGvted( HGvt *, HGvted *, HGed * );
  HGvted *find_right_hgvted( HGvt *, HGvt * );
  
  for ( hgvt = hgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {
    
    if ( hgvt->sp_type == SP_VERTEX_HVERTEX ) {

      harray = (HGedArray *) malloc( hgvt->hgven * sizeof( HGedArray ) );

      n = hgvt->hgven;
      i = 0;
      for ( ve = hgvt->shgve; ve != (HGvted *) NULL; ve = ve->nxt ) {
/* 	display("ed %d\n", ve->ed->no ); */
	if ( ve->ed->sp_type == SP_EDGE_NORMAL ) {
	  harray[i++].ed = ve->ed;
	}
      }
      free_hgppdvertexedge( hgvt );

      /* store start and end (boundary) edges */
      (void) create_hgvted( hgvt, hgvt->startedge );
      (void) create_hgvted( hgvt, hgvt->endedge );

/*       for ( ve = hgvt->shgve; ve != (HGvted *) NULL; ve = ve->nxt ) { */

/* 	display("\t(????) ed %d type %d\n", ve->ed->no, ve->ed->sp_type ); */
/*       } */
      
      for ( i = 0; i < n - 2; ++i ) {
	ed = harray[i].ed;
	if ( ed->sv == hgvt ) v = ed->ev;
	else                  v = ed->sv;
	ve = find_right_hgvted( v, hgvt );

	InsertHGvted( hgvt, ve, ed );

      }
      free( harray );
    }
  }
}

HGvted *find_right_hgvted( HGvt *v, HGvt *hgvt )
{
  HGvted *ve;
  HGvt *sv, *ev;
  Boolean isLeftSide( Vec2d *, Vec2d *, Vec2d * );
  Boolean isHGvtLeftSide( HGvt *, HGvt *, HGvt * );

  for ( ve = hgvt->shgve; ve != (HGvted *) NULL; ve = ve->nxt ) {
    if ( ve->ed->sv == hgvt ) {
      sv = ve->ed->sv;
      ev = ve->ed->ev;
    } else {
      sv = ve->ed->ev;
      ev = ve->ed->sv;
    }
/*     if ( False == isLeftSide( &(sv->uvw), &(ev->uvw), &(v->uvw) ) ) { */
    if ( False == isHGvtLeftSide( sv, ev, v ) ) {
      return ve;
    }
  }
  return hgvt->ehgve;
}
/***********************************/

void mhgfc_create_mhgsf( HGfc *hgfc )
{
  HGvt *hgvt;
  HGsf *hgsf;
  HGhe *hghe;
  void HGfcMakeHGsfFromHGvt( HGvt *, HGfc * );
  
  for ( hgvt = hgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {

    HGfcMakeHGsfFromHGvt( hgvt, hgfc );
    
  }

/*   for ( hgsf = hgfc->shgsf; hgsf != (HGsf *) NULL; hgsf = hgsf->nxt ) { */
/*     display("hgsf %d hn %d\n", hgsf->no, hgsf->hen ); */
/*     hghe = hgsf->shghe; */
/*     do { */
/*       display("\tvt %d ed %d\n", hghe->vt->no, hghe->ed->no ); */
/*     } while ( (hghe = hghe->nxt) != hgsf->shghe ); */
/*   } */
/*   display("\n"); */
}

/************************************************************************ */

/* vt->mvt ???????????????? ? */
void mhgfc_to_gppd ( HGfc *mhgfc, Sppd *gppd )
{
  HGvt *hgvt;
  HGsf *hgsf;
  HGhe *hghe;
  Spvt *mvt;
  Spfc *mfc;
  Sphe *mhe;
  Sped *ed, *ned;
  Spvt *create_ppdvertex( Sppd * );
  Spfc *create_ppdface( Sppd * );
  Sphe *create_ppdhalfedge( Spfc * );
  void free_ppdedge( Sped *, Sppd * );
  
  for ( hgvt = mhgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {
    
    if ( hgvt->sp_type == SP_VERTEX_NORMAL ) {
      mvt = create_ppdvertex( gppd );
      hgvt->vt = mvt;
      mvt->bpso = gppd->spso;
      mvt->sp_type = SP_VERTEX_NORMAL;
      mvt->from = hgvt->from;
    }
  }

  for ( hgsf = mhgfc->shgsf; hgsf != (HGsf *) NULL; hgsf = hgsf->nxt ) {

    mfc = create_ppdface( gppd );
    mfc->bpso = gppd->spso;
    mfc->bppt = gppd->sppt;

    hghe = hgsf->shghe;
    do {
      mhe = create_ppdhalfedge( mfc );
      mhe->vt = hghe->vt->vt;
    } while ( (hghe = hghe->nxt) != hgsf->shghe );
    
  }

  /* free edges (not needed) */
  for ( ed = gppd->sped; ed != (Sped *) NULL; ed = ned ) {
    ned = ed->nxt;
    free_ppdedge( ed, gppd );
  }


    
}
