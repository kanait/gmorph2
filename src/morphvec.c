/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

/*******************************************************************************
  STEP 4: Calc Morphing Vector 
*******************************************************************************/

void calc_morph_vector( HPpd *hppd, Sppd *gppd )
{
  int  i;
  Spvt *vt;
  void initialize_ppd_morph_vector( Sppd * );
  void hvertex_calc_morph_vector( HPpd *, Sppd * );
  void hedge_calc_morph_vector( HPpd *, Sppd * );
  void hface_calc_morph_vector( HPpd *, Sppd * );
  
  initialize_ppd_morph_vector( gppd );
  hvertex_calc_morph_vector( hppd, gppd );
  hedge_calc_morph_vector( hppd, gppd );
  hface_calc_morph_vector( hppd, gppd );
/*   for ( i = 0, vt = gppd->spvt; vt != (Spvt *) NULL; ++i, vt = vt->nxt ) { */
/*     display("vt %d type %d from %d\n", vt->no, vt->sp_type, vt->from); */
/*     display("\tmcalc1 %d mcalc2 %d\n", gppd->mcalc1[i], gppd->mcalc2[i]); */
/*     display("\tmvec1 %g %g %g\n\tmvec2 %g %g %g\n", */
/* 	    gppd->mvec1[i].x, gppd->mvec1[i].y, gppd->mvec1[i].z, */
/* 	    gppd->mvec2[i].x, gppd->mvec2[i].y, gppd->mvec2[i].z ); */
/*   } */
    
}

void initialize_ppd_morph_vector( Sppd *ppd )
{
  int i;
  
  ppd->mvec1  = (Vec *) malloc( ppd->vn * sizeof( Vec ) );
  ppd->mvec2  = (Vec *) malloc( ppd->vn * sizeof( Vec ) );
  ppd->mcalc1 = (unsigned short *) malloc( ppd->vn * sizeof( unsigned short ) );
  ppd->mcalc2 = (unsigned short *) malloc( ppd->vn * sizeof( unsigned short ) );

  /* initialize calculated flag */
  for ( i = 0; i < ppd->vn; ++i ) {
    ppd->mcalc1[i] = SMD_OFF;
    ppd->mcalc2[i] = SMD_OFF;
  }
}

/* vt->sp_type = SP_VERTEX_HVERTEX の頂点に対するベクトルの計算 */

void hvertex_calc_morph_vector( HPpd *hppd, Sppd *gppd )
{
  int     id;
  HVertex *hvt;
  Vec *mvec1, *mvec2;
  unsigned short *mcalc1, *mcalc2;
  
  mvec1  = gppd->mvec1;
  mvec2  = gppd->mvec2;
  mcalc1 = gppd->mcalc1;
  mcalc2 = gppd->mcalc2;

  for ( hvt = hppd->shvt; hvt != (HVertex *) NULL; hvt = hvt->nxt ) {
    id = hvt->vt1->mvt->no;
    mvec1[id].x = hvt->vt1->vec.x;
    mvec1[id].y = hvt->vt1->vec.y;
    mvec1[id].z = hvt->vt1->vec.z;
    mvec2[id].x = hvt->vt2->vec.x;
    mvec2[id].y = hvt->vt2->vec.y;
    mvec2[id].z = hvt->vt2->vec.z;
    mcalc1[id]  = SMD_ON;
    mcalc2[id]  = SMD_ON;
  }
}

/* vt->sp_type = SP_VERTEX_BOUNDARY の頂点に対するベクトルの計算 */
/* HEdge の頂点間に含まれるパラメータ(ev->val) によって算出する */
/*

  |--------------------|--------------------------------|
  v0(from_src)         v(from_target)                   v1(from_src)
  val0                 val                              val1

  vec = v0->vec + (val - val0)/(val1 - val0) * ( v1->vec - v0->vec )
  
*/  
  
void hedge_calc_morph_vector( HPpd *hppd, Sppd *gppd )
{
  int     id, i;
  HEdge   *hed;
  Spvt    *mvt;
  Hedvt   *ev, *temp_ev;
  Vec     *mvec1, *mvec2, *svec, *evec;
  double  tval, sval, eval;
  unsigned short *mcalc1, *mcalc2;
  void    free_hppdedgevertex( HEdge * );
  
  mvec1  = gppd->mvec1;
  mvec2  = gppd->mvec2;
  mcalc1 = gppd->mcalc1;
  mcalc2 = gppd->mcalc2;

  for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {

    i = 0;
    ev = hed->sedvt;
    
    while ( 1 ) {

      if ( ev == (Hedvt *) NULL ) break;

/*       display("%2d/%2d ", i+1, hed->evn); */
      mvt = ev->vt;
      id = mvt->no;
	
      if ( mvt->sp_type == SP_VERTEX_BOUNDARY ) {

	/* calc vector for src object */

	if ( mvt->from == FROM_SRC ) {
	  
	  mvec1[id].x = mvt->vec.x;
	  mvec1[id].y = mvt->vec.y;
	  mvec1[id].z = mvt->vec.z;
	  mcalc1[id]  = SMD_ON;

/* 	  display("src %g\n", ev->val); */
	  
	} else {

	  /* cale vector from the target by linear interpolation */

	  /* find_start vector */
	  temp_ev = ev;
	  while ( 1 ) {
	    temp_ev = temp_ev->prv;
	    if ( temp_ev == hed->sedvt ) {
	      /* あとで変える *_SP.gmh をすべて更新してから */
	      svec = &( hed->sv->vt1->vec );
/* 	      svec = &( hed->ev->vt1->vec ); */
	      sval = 0.0;
	      break;
	    }
	    if ( temp_ev->vt->from == FROM_SRC ) {
	      svec = &(temp_ev->vt->vec);
	      sval = temp_ev->val;
	      break;
	    }
	  }
	  
	  /* find end vector */
	  temp_ev = ev;
	  while ( 1 ) {
	    temp_ev = temp_ev->nxt;
	    if ( temp_ev == hed->eedvt ) {
	      /* あとで変える *_SP.gmh をすべて更新してから */
	      evec = &( hed->ev->vt1->vec );
/* 	      evec = &( hed->sv->vt1->vec ); */
	      eval = 1.0;
	      break;
	    }
	    if ( temp_ev->vt->from == FROM_SRC ) {
	      evec = &( temp_ev->vt->vec );
	      eval = temp_ev->val;
	      break;
	    }
	  }
	  tval = ( ev->val - sval ) / ( eval - sval );
	  mvec1[id].x = svec->x + tval * ( evec->x - svec->x );
	  mvec1[id].y = svec->y + tval * ( evec->y - svec->y );
	  mvec1[id].z = svec->z + tval * ( evec->z - svec->z );
	  mcalc1[id]  = SMD_ON;
/* 	  display("target val %g sval %g eval %g\n", ev->val, sval, eval ); */
	  
	}

	/* calc vector for target object */
	
	if ( mvt->from == FROM_SRC ) {
	  
	  /* cale vector from the src linear interpolation */
	  
	  /* find_start vector */
	  temp_ev = ev;
	  while ( 1 ) {
	    temp_ev = temp_ev->prv;
	    if ( temp_ev == hed->sedvt ) {
	      /* あとで変える *_SP.gmh をすべて更新してから */
	      svec = &( hed->sv->vt2->vec );
/* 	      svec = &( hed->ev->vt2->vec ); */
	      sval = 0.0;
	      break;
	    }
	    if ( temp_ev->vt->from == FROM_TARGET ) {
	      svec = &( temp_ev->vt->vec );
	      sval = temp_ev->val;
	      break;
	    }
	  }
	  
	  /* find end vector */
	  temp_ev = ev;
	  while ( 1 ) {
	    temp_ev = temp_ev->nxt;
	    if ( temp_ev == hed->eedvt ) {
	      /* あとで変える *_SP.gmh をすべて更新してから */
	      evec = &( hed->ev->vt2->vec );
/* 	      evec = &( hed->sv->vt2->vec ); */
	      eval = 1.0;
	      break;
	    }
	    if ( temp_ev->vt->from == FROM_TARGET ) {
	      evec = &( temp_ev->vt->vec );
	      eval = temp_ev->val;
	      break;
	    }
	  }

	  tval = ( ev->val - sval ) / ( eval - sval );
	  mvec2[id].x = svec->x + tval * ( evec->x - svec->x );
	  mvec2[id].y = svec->y + tval * ( evec->y - svec->y );
	  mvec2[id].z = svec->z + tval * ( evec->z - svec->z );
	  mcalc2[id]  = SMD_ON;
	  
	} else {
	  
	  mvec2[id].x = mvt->vec.x;
	  mvec2[id].y = mvt->vec.y;
	  mvec2[id].z = mvt->vec.z;
	  mcalc2[id]  = SMD_ON;
	  
	}
	
      } else {
/* 	display("hvertex\n"); */
      }
      
/*       display("%g %g %g\n", mvec1[id].x, mvec1[id].y, mvec1[id].z ); */

      ev = ev->nxt; ++i;
    }

    /* ここでフリー */
    free_hppdedgevertex( hed );
    
  }

}

/* vt->sp_type = SP_VERTEX_NORMAL の頂点に対するベクトルの計算 */

void hface_calc_morph_vector( HPpd *hppd, Sppd *gppd )
{
  HFace  *hfc;
  HGfc   *hgfc1, *hgfc2, *mhgfc;
  HGvt *hgvt;
  void hgfc_calc_morph_vector( HGfc *, HGfc *, HGfc *, Sppd *, int from );
  
  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {

    hgfc1 = hfc->hgfc1;
    hgfc2 = hfc->hgfc2;
    mhgfc = hfc->mhgfc;

    hgfc_calc_morph_vector( hgfc1, hgfc2, mhgfc, gppd, SRC );
    hgfc_calc_morph_vector( hgfc2, hgfc1, mhgfc, gppd, TARGET );

  }
}

void hgfc_calc_morph_vector( HGfc *hgfc, HGfc *bghgfc, HGfc *mhgfc, Sppd *gppd, int from )
{
  int id;
  HGvt *hgvt, *mhgvt, *shgvt, *ehgvt;
  HGed *hged;
  HGedvt *ev;
  HGsf *sf;
  Spvt *mvt;
  Vec *mvec1, *mvec2, *svec, *evec;
  unsigned short *mcalc1, *mcalc2;
  Vec *vec0, *vec1, *vec2;
  double p, al, be, gm;
  void VertexTriangleBarycentric( HGsf *, Vec2d *,
				  double *, double *, double * );
  double V2DistanceBetween2Points( Vec2d *, Vec2d * );
  HGsf *find_hgsf_in_hgfc( HGvt *, HGfc * );
  
  mvec1  = gppd->mvec1;
  mvec2  = gppd->mvec2;
  mcalc1 = gppd->mcalc1;
  mcalc2 = gppd->mcalc2;

  for ( hgvt = hgfc->shgvt; hgvt != (HGvt *) NULL; hgvt = hgvt->nxt ) {

    if ( hgvt->sp_type == SP_VERTEX_NORMAL ) {
      
      mhgvt = hgvt->mhgvt; mvt = mhgvt->vt; id = mvt->no;

      if ( from == SRC ) {
	
	mvec1[id].x = hgvt->vt->vec.x;
	mvec1[id].y = hgvt->vt->vec.y;
	mvec1[id].z = hgvt->vt->vec.z;
	mcalc1[id]  = SMD_ON;

	sf = hgvt->inc_sf;
	/* かなり付け焼き刃 */
	if ( sf == NULL ) {
	  sf = hgvt->inc_sf = find_hgsf_in_hgfc( hgvt, bghgfc );
	}
	vec0 = &(sf->shghe->vt->vt->vec);
	vec1 = &(sf->shghe->nxt->vt->vt->vec);
	vec2 = &(sf->shghe->nxt->nxt->vt->vt->vec);
	VertexTriangleBarycentric( sf, &(hgvt->uvw), &al, &be, &gm );
	mvec2[id].x = al * vec0->x + be * vec1->x + gm * vec2->x;
	mvec2[id].y = al * vec0->y + be * vec1->y + gm * vec2->y;
	mvec2[id].z = al * vec0->z + be * vec1->z + gm * vec2->z;
	mcalc2[id]  = SMD_ON;
	
      } else {
	
	sf = hgvt->inc_sf;
	/* かなり付け焼き刃 */
	if ( sf == NULL ) {
	  sf = hgvt->inc_sf = find_hgsf_in_hgfc( hgvt, bghgfc );
	}
	vec0 = &(sf->shghe->vt->vt->vec);
	vec1 = &(sf->shghe->nxt->vt->vt->vec);
	vec2 = &(sf->shghe->nxt->nxt->vt->vt->vec);
	VertexTriangleBarycentric( sf, &(hgvt->uvw), &al, &be, &gm );
	mvec1[id].x = al * vec0->x + be * vec1->x + gm * vec2->x;
	mvec1[id].y = al * vec0->y + be * vec1->y + gm * vec2->y;
	mvec1[id].z = al * vec0->z + be * vec1->z + gm * vec2->z;
	mcalc1[id]  = SMD_ON;
	
	mvec2[id].x = hgvt->vt->vec.x;
	mvec2[id].y = hgvt->vt->vec.y;
	mvec2[id].z = hgvt->vt->vec.z;
	mcalc2[id]  = SMD_ON;

      }
    }
  }

  /* intersection points */
  for ( hged = hgfc->shged; hged != (HGed *) NULL; hged = hged->nxt ) {

    shgvt = hged->sv;
    ehgvt = hged->ev;
    svec  = &( shgvt->vt->vec );
    evec  = &( ehgvt->vt->vec );

    for ( ev = hged->shgev; ev != (HGedvt *) NULL; ev = ev->nxt ) {
      
      hgvt = ev->vt;
      mvt  = hgvt->vt; id = mvt->no;

      p = V2DistanceBetween2Points( &(hgvt->uvw), &(shgvt->uvw) ) /
	V2DistanceBetween2Points( &(ehgvt->uvw), &(shgvt->uvw) );
      if ( from == SRC ) {
	mvec1[id].x = svec->x + p * ( evec->x - svec->x );
	mvec1[id].y = svec->y + p * ( evec->y - svec->y );
	mvec1[id].z = svec->z + p * ( evec->z - svec->z );
	mcalc1[id]  = SMD_ON;
      } else {
	mvec2[id].x = svec->x + p * ( evec->x - svec->x );
	mvec2[id].y = svec->y + p * ( evec->y - svec->y );
	mvec2[id].z = svec->z + p * ( evec->z - svec->z );
	mcalc2[id]  = SMD_ON;
      }
    }
  }
    
}

      
      
