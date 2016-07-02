/* Copyright (c) 1996-1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

#define	KAPPA	1.0

/*******************************************************************************
  STEP 2: harmonic maps (Related functions are in "harmonic.c".)
*******************************************************************************/

void hppdharmonic( HPpd *hppd )
{
  HFace *hf;
  void hgfcharmonic( HGfc *, Splp *lp );
  void edge_length( Sppd * );
  void hppd_harmonicmap_to_ps( HPpd * );
  void  time_start(void);
  void  time_stop(void);
  /* calc edge length */
  edge_length( hppd->ppd1 );
  edge_length( hppd->ppd2 );
  time_start();
  for ( hf = hppd->shfc; hf != (HFace *) NULL; hf = hf->nxt ) {
    
    hgfcharmonic( hf->hgfc1, hf->hloop->lp1 );
    hgfcharmonic( hf->hgfc2, hf->hloop->lp2 );
  }
  display("time:harmonic\n");
  time_stop();
/*   hppd_harmonicmap_to_ps( hppd ); */
  
}

/* harmonic mapping functions */
void hgfcharmonic( HGfc *hgfc, Splp *lp )
{
  int    iter;
  int    vn;
  double rsq;
  Semat  *emat;
  int    initialize_hgfc( HGfc * );
  Semat  *create_harmonic_emat( HGfc * );
  void   harmonic_initialize_vector( HGfc *, Splp *, Semat * );
  int    linbcg( Semat *, double *, double *, int, double, int, int *, double *);
  void   solvec_hgfc( HGfc *, Semat * );
  void   exit_hgfc( HGfc * );
  void   free_emat( Semat * );
  void   printemat( Semat * );

/*   display("group %d\n", hgfc->id ); */
  vn = initialize_hgfc( hgfc );
/*   display("group No.%d vn %d\n", hgfc->id, vn); */
  emat = create_harmonic_emat( hgfc );
  harmonic_initialize_vector( hgfc, lp, emat );

/*   if ( vn ) { */
    linbcg( emat, emat->bx, emat->xx, 1, SMDZEROEPS, 1000, &iter, &rsq );
    linbcg( emat, emat->by, emat->yy, 1, SMDZEROEPS, 1000, &iter, &rsq );
    solvec_hgfc( hgfc, emat );
/*   } */

  free_emat( emat );
  exit_hgfc( hgfc );
/*   display("\n"); */
}

int initialize_hgfc( HGfc *hgfc )
{
  int  i;
  int  cnt = 0;
  HGvt *hv;
  HGed *he;
  HGvted *create_hgvted( HGvt *, HGed * );

  /* id */

  for (i = 0, hv = hgfc->shgvt; hv != (HGvt *) NULL; hv = hv->nxt, ++i) {
    hv->sid  = i;
    if ( hv->vt->sp_type == SP_VERTEX_NORMAL ) {
      ++cnt;
    }
  }
  /* , and create hgvt->hged links */
  for (i = 0, he = hgfc->shged; he != (HGed *) NULL; he = he->nxt, ++i) {
    he->sid  = i;
    create_hgvted( he->sv, he );
    create_hgvted( he->ev, he );
  }
  return cnt;
}

Semat *create_harmonic_emat( HGfc *hgfc )
{
  int    i, j, vn, cnt;
  int    id;
  double *kappa;
  Semat  *emat;
  HGvt   *vt;
  HGed   *ed;
  HGvted *ve;
  HGvtvt *vv;
  Semat  *init_emat( int );
  double *harmonic_calc_kappa( HGfc * );
  HGvtvt *find_sort_hgvtvt( HGvt *, Id );
  void   free_hgppdvertexvertex( HGvt * );
  void   dsprsin( HGfc *, Semat * );
  void   FreeHGppdVertexVertex( HGfc * );
  
  emat = init_emat( hgfc->hgvn );
  
  kappa = harmonic_calc_kappa( hgfc );

  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    vt->vval = 0.0;
  }

  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    for ( ve = vt->shgve; ve != (HGvted *) NULL; ve = ve->nxt ) {
      
      ed = ve->ed;
      /* sv */
      id = ed->sv->sid;
      if (vt->sid != id) {
	vv = find_sort_hgvtvt( vt, id );
	vv->val -= kappa[ed->sid];
      } else {
	vt->vval += kappa[ed->sid];
      }
      /* ev */
      id = ed->ev->sid;
      if (vt->sid != id) {
	vv = find_sort_hgvtvt( vt, id );
	vv->val -= kappa[ed->sid];
      } else {
	vt->vval += kappa[ed->sid];
      }
    }
  }
  
  /* Boundary Points are fixed. */
  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    
    if ( vt->vt->sp_type != SP_VERTEX_NORMAL ) {
      vt->vval = 1.0;
      free_hgppdvertexvertex( vt );
    }
    
  }

  vn = 0;
  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    vn += vt->hgvvn;
  }

  emat->num = vn + hgfc->hgvn + 1;
  emat->sa  = (double *) malloc(emat->num * sizeof(double));
  for (i = 0; i < emat->num; ++i) emat->sa[i] = 0.0;
  emat->ija = (int *) malloc(emat->num * sizeof(int));

  dsprsin( hgfc, emat );
  
  free(kappa);

  FreeHGppdVertexVertex( hgfc );
  
  return emat;
}

double *harmonic_calc_kappa( HGfc *hgfc )
{
  double *kappa;
  HGed   *ed;
  
  /* initialize length, area, kappa */
  kappa   = (double *) malloc( hgfc->hgen * sizeof(double));

  for ( ed = hgfc->shged; ed != (HGed *) NULL; ed = ed->nxt ) {
    kappa[ed->sid] = 1.0;
  }
  return kappa;
}

static double uvprmsx[4] = {
  1.0, 0.0, 0.0, 1.0
};
  
static double uvprmsy[4] = {
  0.0, 1.0, 0.0, 0.0
};
  
void harmonic_initialize_vector( HGfc *hgfc, Splp *lp, Semat *emat )
{
  int    i;
  HGvt   *vt;
  Vec2d  *uvbprm;
  Sple   *le, *tle, *sle;
  Splv   *lv, *tlv, *slv, *elv;
  double sum, esum, val;
  Vec2d  sub;
  Vec2d *calc_uvbprm( int );

  /* initialize vector */
  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    
    emat->bx[vt->sid] = emat->xx[vt->sid] = 0.0;
    emat->by[vt->sid] = emat->yy[vt->sid] = 0.0;
    
  }

  uvbprm = calc_uvbprm( hgfc->hgcn );

  /* calculate boundary values */

  lv = lp->splv;
  le = lp->sple;

  i = 0;
  while ( i < hgfc->hgcn ) {

    slv = lv;
    sle = le;
    sum = 0.0;

    while ( 1 ) {
/*       display("(%d) vt %d type %d\n", i, lv->vt->no, lv->vt->sp_type); */
      sum += le->ed->length;
      lv = lv->nxt;
      le = le->nxt;
      if ( lv == (Splv *) NULL ) {
	elv = lp->splv;
	break;
      }
      if ( lv->vt->sp_type == SP_VERTEX_HVERTEX ) {
	elv = lv;
	break;
      }
    }

    esum = 0.0;
    tlv = slv;
    tle = sle;
    sub.x = uvbprm[i+1].x - uvbprm[i].x;
    sub.y = uvbprm[i+1].y - uvbprm[i].y;
/*     display("\n"); */
/*     display("sub %g %g\n", sub.x, sub.y); */
    while ( 1 ) {
      
      val = esum / sum;
/*       display("val %g\n", val ); */
      emat->bx[ tlv->hgvt->sid ] = uvbprm[i].x + sub.x * val;
      emat->by[ tlv->hgvt->sid ] = uvbprm[i].y + sub.y * val;
/*       display("bx[%d] %g %g\n", tlv->hgvt->sid, */
/* 	      emat->bx[ tlv->hgvt->sid ], */
/* 	      emat->by[ tlv->hgvt->sid ]); */
	      
      esum += tle->ed->length;

      tlv = tlv->nxt;
      tle = tle->nxt;

      if ( (tlv == elv) || (tlv == (Splv *) NULL) ) {
/* 	display("val %g\n", esum / sum ); */
	/* temp */
/* 	if ( tlv == (Splv *) NULL ) tlv = lp->splv; */
/* 	display("bx[%d] %g %g\n", tlv->hgvt->sid, */
/* 	      emat->bx[ tlv->hgvt->sid ], */
/* 	      emat->by[ tlv->hgvt->sid ]); */
	break;
      }
    }
    
    ++i;
  }

/*   for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) { */
/*     display("vt %d type %d bx[%d] %g %g\n", lv->hgvt->sid, */
/* 	    lv->hgvt->vt->sp_type, */
/* 	    lv->hgvt->sid,  */
/* 	    emat->bx[ lv->hgvt->sid ], */
/* 	    emat->by[ lv->hgvt->sid ]); */
/*   } */
/*   lv = lp->splv; */
/*   display("vt %d type %d bx[%d] %g %g\n", lv->hgvt->sid, */
/* 	  lv->hgvt->vt->sp_type, */
/* 	  lv->hgvt->sid,  */
/* 	  emat->bx[ lv->hgvt->sid ], */
/* 	  emat->by[ lv->hgvt->sid ]); */
    
  free(uvbprm);
}

Vec2d *calc_uvbprm( int cn )
{
  int i;
  double angle;
  Vec2d *uvbprm;

  uvbprm = (Vec2d *) malloc( (cn+1) * sizeof(Vec2d) );

  for ( i = 0; i <= cn; ++i ) {
    angle = 2 * (double) i * SMDPI / (double) cn;
    uvbprm[i].x = cos( angle );
    uvbprm[i].y = sin( angle );
/*     display("uvbprm %g %g\n", uvbprm[i].x, uvbprm[i].y); */
  }
  return uvbprm;
}
    
void solvec_hgfc( HGfc *hgfc, Semat *emat )
{
  HGvt *vt;
  
  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    
    vt->uvw.x = emat->xx[vt->sid];
    vt->uvw.y = emat->yy[vt->sid];
/*     vt->uvw.z = 1.0 - emat->xx[vt->sid] - emat->yy[vt->sid]; */
  }

/*     vt->vec.x = ( vt->uvw.x * hgfc->cvt[0]->vec.x + */
/* 		  vt->uvw.y * hgfc->cvt[1]->vec.x + */
/* 		  vt->uvw.z * hgfc->cvt[2]->vec.x ); */
      
/*     vt->vec.y = ( vt->uvw.x * hgfc->cvt[0]->vec.y + */
/* 		  vt->uvw.y * hgfc->cvt[1]->vec.y + */
/* 		  vt->uvw.z * hgfc->cvt[2]->vec.y ); */
      
/*     vt->vec.z = ( vt->uvw.x * hgfc->cvt[0]->vec.z + */
/* 		  vt->uvw.y * hgfc->cvt[1]->vec.z + */
/* 		  vt->uvw.z * hgfc->cvt[2]->vec.z ); */
    
/*     display("vt %d type %d (uvw) %g %g %g (vec) %g %g %g\n", */
/* 	    vt->sid, */
/* 	    vt->vt->sp_type, */
/* 	    vt->uvw.x, vt->uvw.y, vt->uvw.z, */
/* 	    vt->vec.x, vt->vec.y, vt->vec.z); */
}

void exit_hgfc( HGfc *hgfc )
{
  HGvt *hv;
  HGed *he;
  void free_hgppdvertexedge(HGvt *);
  void hgppdface_to_hgppdsurface( HGfc * );
  
  for ( hv = hgfc->shgvt; hv != (HGvt *) NULL; hv = hv->nxt ) {
    hv->sid  = SMDNULL;
    hv->vval = 0.0;
    free_hgppdvertexedge( hv );
  }
  
  /* create hgfc surface */
/*   hgppdface_to_hgppdsurface( hgfc ); */
  
  for ( he = hgfc->shged; he != (HGed *) NULL; he = he->nxt ) {
    he->sid  = SMDNULL;
  }
  
}

/* Semat functions */
Semat *init_emat( int num )
{
  Semat *emat;
  
  emat = (Semat *) malloc(sizeof(Semat));
  emat->rnum = num;
  emat->cnum = num;
  /* constant vectors initialize */
  emat->bx = (double *) malloc(num * sizeof(double));
  emat->by = (double *) malloc(num * sizeof(double));
  /* solution vectors initialize */
  emat->xx = (double *) malloc(num * sizeof(double));
  emat->yy = (double *) malloc(num * sizeof(double));

  return emat;
}

void free_emat(Semat *emat)
{
  free(emat->sa);
  free(emat->ija);
  free(emat->bx);
  free(emat->by);
  free(emat->xx);
  free(emat->yy);
  free(emat);
}

void printemat(Semat *emat)
{
  int i;
  
  for (i = 0; i < emat->cnum; ++i) {
    display("%d (b) %f %f\n", i, emat->bx[i], emat->by[i]);
    display("%d (x) %f %f\n", i, emat->xx[i], emat->yy[i]);
  }
  for (i = 0; i < emat->num; ++i) 
    display("(%d) (sa) %f (ija) %d\n", i, emat->sa[i], emat->ija[i]);
}


  
