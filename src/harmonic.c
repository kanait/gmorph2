/* Copyright (c) 1996-1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"
#include "harmonic_eigen.h"

#include <stdint.h>

#ifdef Success
#undef Success
#endif
#ifdef Status
#undef Status
#endif

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
  int    initialize_hgfc( HGfc * );
  void   free_hgppdvertexvertex( HGvt * );
  void   FreeHGppdVertexVertex( HGfc * );
  void   exit_hgfc( HGfc * );
  Vec2d *calc_uvbprm( int );
  double *harmonic_calc_kappa( HGfc * );
  HGvtvt *find_sort_hgvtvt( HGvt *, Id );

  double *kappa;
  HGvt   *vt;
  HGvted *ve;
  HGed   *ed;
  HGvtvt *vv;
  Vec2d  *uvbprm;
  Sple   *le, *tle, *sle;
  Splv   *lv, *tlv, *slv, *elv;
  double sum, esum, val;
  Vec2d  sub;
  int    i;
  int    id;

  size_t tcap;
  size_t nt;
  int   *tri_r;
  int   *tri_c;
  double *tri_v;
  double *bx;
  double *by;
  double *xx;
  double *xy;
  int    sol;

  (void)initialize_hgfc( hgfc );

  kappa = harmonic_calc_kappa( hgfc );

  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    vt->vval = 0.0;
  }

  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    for ( ve = vt->shgve; ve != (HGvted *) NULL; ve = ve->nxt ) {
      ed = ve->ed;
      if ( ed == (HGed *) NULL ) continue;
      if ( ed->sv == (HGvt *) NULL || (uintptr_t)ed->sv < 0x10000 ) continue;
      if ( ed->ev == (HGvt *) NULL || (uintptr_t)ed->ev < 0x10000 ) continue;

      id = ed->sv->sid;
      if ( vt->sid != id ) {
        vv = find_sort_hgvtvt( vt, id );
        vv->val -= kappa[ed->sid];
      } else {
        vt->vval += kappa[ed->sid];
      }

      id = ed->ev->sid;
      if ( vt->sid != id ) {
        vv = find_sort_hgvtvt( vt, id );
        vv->val -= kappa[ed->sid];
      } else {
        vt->vval += kappa[ed->sid];
      }
    }
  }

  free( kappa );

  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    if ( vt->vt->sp_type != SP_VERTEX_NORMAL ) {
      vt->vval = 1.0;
      free_hgppdvertexvertex( vt );
    }
  }

  tcap = (size_t)hgfc->hgvn * 8u;
  if (tcap < 32u)
    tcap = 32u;
  tri_r = (int *)malloc(tcap * sizeof *tri_r);
  tri_c = (int *)malloc(tcap * sizeof *tri_c);
  tri_v = (double *)malloc(tcap * sizeof *tri_v);
  nt = 0;
  if (tri_r == NULL || tri_c == NULL || tri_v == NULL) {
    free(tri_r);
    free(tri_c);
    free(tri_v);
    exit_hgfc( hgfc );
    return;
  }

  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    if (nt >= tcap) {
      size_t ncap = tcap * 2u;
      int *nr = (int *)realloc(tri_r, ncap * sizeof *nr);
      int *nc = (int *)realloc(tri_c, ncap * sizeof *nc);
      double *nv = (double *)realloc(tri_v, ncap * sizeof *nv);
      if (nr == NULL || nc == NULL || nv == NULL) {
        free(tri_r);
        free(tri_c);
        free(tri_v);
        exit_hgfc( hgfc );
        return;
      }
      tri_r = nr;
      tri_c = nc;
      tri_v = nv;
      tcap = ncap;
    }
    tri_r[nt] = vt->sid;
    tri_c[nt] = vt->sid;
    tri_v[nt] = vt->vval;
    nt++;
    if ( vt->vt->sp_type == SP_VERTEX_NORMAL ) {
      for ( vv = vt->shgvv; vv != (HGvtvt *) NULL; vv = vv->nxt ) {
        if (nt >= tcap) {
          size_t ncap = tcap * 2u;
          int *nr = (int *)realloc(tri_r, ncap * sizeof *nr);
          int *nc = (int *)realloc(tri_c, ncap * sizeof *nc);
          double *nv = (double *)realloc(tri_v, ncap * sizeof *nv);
          if (nr == NULL || nc == NULL || nv == NULL) {
            free(tri_r);
            free(tri_c);
            free(tri_v);
            exit_hgfc( hgfc );
            return;
          }
          tri_r = nr;
          tri_c = nc;
          tri_v = nv;
          tcap = ncap;
        }
        tri_r[nt] = vt->sid;
        tri_c[nt] = vv->id;
        tri_v[nt] = vv->val;
        nt++;
      }
    }
  }

  FreeHGppdVertexVertex( hgfc );

  bx = (double *)calloc((size_t)hgfc->hgvn, sizeof *bx);
  by = (double *)calloc((size_t)hgfc->hgvn, sizeof *by);
  xx = (double *)calloc((size_t)hgfc->hgvn, sizeof *xx);
  xy = (double *)calloc((size_t)hgfc->hgvn, sizeof *xy);
  if (bx == NULL || by == NULL || xx == NULL || xy == NULL) {
    free(tri_r);
    free(tri_c);
    free(tri_v);
    XFREE(bx);
    XFREE(by);
    XFREE(xx);
    XFREE(xy);
    exit_hgfc( hgfc );
    return;
  }

  uvbprm = calc_uvbprm( hgfc->hgcn );
  lv = lp->splv;
  le = lp->sple;
  i = 0;
  while ( i < hgfc->hgcn ) {
    slv = lv;
    sle = le;
    sum = 0.0;

    while ( 1 ) {
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
    while ( 1 ) {
      val = esum / sum;
      bx[tlv->hgvt->sid] = uvbprm[i].x + sub.x * val;
      by[tlv->hgvt->sid] = uvbprm[i].y + sub.y * val;

      esum += tle->ed->length;
      tlv = tlv->nxt;
      tle = tle->nxt;

      if ( (tlv == elv) || (tlv == (Splv *) NULL) ) {
        break;
      }
    }

    ++i;
  }
  free( uvbprm );

  sol = gmorph_harmonic_bicgstab_xy(hgfc->hgvn, (int)nt, tri_r, tri_c, tri_v,
                                    bx, by, xx, xy);
  (void)sol;

  free(tri_r);
  free(tri_c);
  free(tri_v);
  free(bx);
  free(by);

  for ( vt = hgfc->shgvt; vt != (HGvt *) NULL; vt = vt->nxt ) {
    vt->uvw.x = xx[vt->sid];
    vt->uvw.y = xy[vt->sid];
  }

  free(xx);
  free(xy);

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
