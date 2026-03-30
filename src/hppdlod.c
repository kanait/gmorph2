/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

void ppdtocmeshcb(Widget w, XtPointer cld, XtPointer *cad)
{
  HPpd *hppd;
  void ppdtocmesh( HPpd * );
  void clear_hppdlod( HPpd * );
  void make_shortest_path( HPpd * );
  
  if ( (hppd = swin->hppd) == (HPpd *) NULL ) {
    return;
  }
  if ( hppd->vn < 4 ) {
    displayinfo("Error: to convert ppd to hppd, \n");
    displayinfo("\t1. more than 4 hvertices are required if a ppd have no loop.\n");
    displayinfo("\t2. more than 3 hvertices are required per a loop boundary.\n");
    return;
  }

  clear_hppdlod( hppd );
  ppdtocmesh( hppd );
}

void ppdtocmesh( HPpd *hppd )
{
  Sppd *ppd, *oppd;
  /* tmp */
  Spfc *fc;
  Sped *ed;
  Sphe *he;
  Spvt *vt;
  Sppd *copy_ppd( Sppd * );
  void calc_ppd_boundary( Sppd * );
  void optlod( Sppd *, int );
  void calc_fnorm( Spfc * );
  void ppdtohppd( Sppd *, HPpd * );
  void free_ppd( Sppd * );
/*   void printppd( Sppd * ); */
  void make_shortest_path( HPpd * );
  
  displayinfo("ppd - hppd: begin... ");
  
  if ( (hppd->ppd1 == NULL) || (hppd->ppd2 == NULL) ) return;

  oppd = ( hppd->ppd1->vn < hppd->ppd2->vn ) ? hppd->ppd1 : hppd->ppd2;

  /* initialize */
  ppd = copy_ppd( oppd );

  optlod( ppd, hppd->vn );

  ppdtohppd( ppd, hppd );
  free_ppd( ppd );
  displayinfo("done. \nhppd: vertex %d face %d\n", hppd->vn, hppd->fn);
  
  /* make shortest path and make hloop */
/*   displayinfo("calculate shortest-path... \n"); */
/*   make_shortest_path( hppd ); */
/*   displayinfo("done. \n"); */
}

void optlod( Sppd *ppd, int num )
{
  Sped  *ed;
  Sped  *try_edge_operation( Sped *, Sppd * );

  ed = ppd->sped;
  if ( ed == (Sped *) NULL ) return;
  
  while (1) {

    ed = try_edge_operation( ed, ppd );

    if ( ed == (Sped *) NULL ) break;
    if ( ppd->vn == num ) break;
    
  }
}

void swap_edgevertex( Sped *ed )
{
  Spvt *tv;
  
  tv = ed->sv;
  ed->sv = ed->ev;
  ed->ev = tv;
}
  
Sped *try_edge_operation( Sped *ed, Sppd *ppd )
{
  Star   *star;
  Starvt *sv;
  Sped   *next_ed( Sped *, Sppd * );
  Star   *create_star( Sped * );
  void   free_star( Star * );
  int    collapse_condition( Star * );
  void   swap_edgevertex( Sped * );
  Sped   *edge_collapse( Sped *, Sppd * );

  if ( (ed->sv->sp_type == SP_VERTEX_HVERTEX) && (ed->ev->sp_type == SP_VERTEX_HVERTEX) )  {
    return next_ed( ed, ppd );
  }
  if ( (ed->ev->sp_type == SP_VERTEX_HVERTEX) ) {
    swap_edgevertex( ed );
  }
  
  if ( (star = create_star( ed )) == (Star *) NULL ) {
    return next_ed( ed, ppd );
  }

  if ( collapse_condition( star ) != SMD_ON ) {
    free_star( star );
    return next_ed( ed, ppd );
  }
    
  ed = edge_collapse( ed, ppd );
  free_star( star );
  
  return ed;
}
  
Sped *next_ed( Sped *ed, Sppd *ppd )
{
  if ( ed->nxt != (Sped *) NULL ) return ed->nxt;
  else return ppd->sped;
}

Sped *edge_collapse( Sped *e, Sppd *ppd )
{
  int  i;
  Spvt *vi, *vj, *vk;
  Spfc *f[2], *fi, *fj;
  Sped *ei, *ej[2], *ne;
  int  side;
  Spfc *fno[2];
  /* tmp */
  Vted *ve;
  Spvt *get_lm_ppdvertex(Spfc *, Spvt *, Spvt *);
  Sped *get_lm_ppdedge(Spfc *, Spvt *, Spvt *);
  Spfc *get_lm_ppdface_rev(Spfc *, Sped *);
  void append_ppdedge(Sped *, Spfc *, Spfc *);
  void append_ppdvertex( Spfc *, Spvt *, Spvt *);
  void append_ppdface(Spfc *, Sped *, Sped *);
  void free_vtxed_num(Spvt *, Sped *);
  void change_ppdvertexlinks(Spvt *, Spvt *);
  int  change_vertexnumber(Spfc *, Spvt *, Spvt *);
  void free_ppdface( Spfc *, Sppd * );
  void free_ppdedge( Sped *, Sppd * );
  void free_ppdvertex( Spvt *, Sppd * );

  if ( e == (Sped *) NULL ) return (Sped *) NULL;
  
  /* edge collapse driver */

/*   display("e  Edge No.%d\n", e->no); */
  vi  = e->sv; vj = e->ev;
/*   display("vi Vertex No.%d\n", vi->no); */
/*   display("vj Vertex No.%d\n", vj->no); */

/*   display("Vertex No.%d links: \n", vi->no); */
/*   for ( ve = vi->svted; ve != (Vted *) NULL; ve = ve->nxt ) { */
/*     display("\tEdge No.%d\n", ve->ed->no); */
/*   } */
  if ( e->atr != EDGEBOUNDARY ) {
    side = EDGEINTERNAL;
    f[0] = e->rf;
    f[1] = e->lf;
  } else {
    side = EDGEBOUNDARY;
    f[0] = ( e->rf != (Spfc *) NULL ) ? e->rf : e->lf;
  }

  /* fj は別に NULL でもよい */
  for (i = 0; i < side; ++i) {
/*     display("side %d:\n", i); */
/*     display("f  Face No.%d\n", (f != NULL) ? f[i]->no : SMDNULL); */
    vk = get_lm_ppdvertex( f[i], vi, vj );
/*     display("vk Vertex No.%d\n", (vk != NULL) ? vk->no : SMDNULL); */
    ei = get_lm_ppdedge( f[i], vi, vk );
/*     display("ei Edge No.%d\n", (ei != NULL ) ? ei->no : SMDNULL); */
    ej[i] = get_lm_ppdedge( f[i], vj, vk );
/*     display("ej Edge No.%d\n", (ej[i] != NULL) ? ej[i]->no : SMDNULL); */
    fi = get_lm_ppdface_rev( f[i], ei );
/*     display("fi Face No.%d\n", (fi != NULL) ? fi->no : SMDNULL); */
    fj = get_lm_ppdface_rev( f[i], ej[i] );
/*     display("fj Face No.%d\n", (fj != NULL) ? fj->no : SMDNULL); */
    
    append_ppdedge( ei, f[i], fj );
    append_ppdedge( ej[i], f[i], fi );
    append_ppdface( fj, ej[i], ei );
    
    free_vtxed_num( vk, ej[i] );
    append_ppdvertex( fj, vj, vi );
    free_vtxed_num( vj, ej[i] );
    
    if ( fj == (Spfc *) NULL ) ei->atr = EDGEBOUNDARY;
  }
  /* change vertexlinks */

  change_ppdvertexlinks( vj, vi );

  for ( i = 0; i < side; ++i ) {
/*     display("Face No.%d deleted.\n", f[i]->no); */
    free_ppdface( f[i], ppd );
/*     display("Edge No.%d deleted.\n", ej[i]->no); */
    free_ppdedge( ej[i], ppd );
  }
/*   display("Vertex No.%d deleted.\n", vj->no); */
  free_ppdvertex( vj, ppd );
  
  free_vtxed_num( vi, e );
/*   display("Edge No.%d deleted.\n", e->no); */
  ne = next_ed( e, ppd );
  free_ppdedge( e, ppd );

/*   display("Vertex No.%d links: \n", vi->no); */
/*   for ( ve = vi->svted; ve != (Vted *) NULL; ve = ve->nxt ) { */
/*     display("\tEdge No.%d\n", ve->ed->no); */
/*   } */
  return ne;
}

Spvt *get_lm_ppdvertex( Spfc *f, Spvt *v1, Spvt *v2 )
{
  Spvt *vt;
  Sphe *he;
  
  if ( f == (Spfc *) NULL ) return (Spvt *) NULL;

  he = f->sphe;

  do {
    vt = he->vt;
    if ( ( vt != v1 ) && ( vt != v2 ) ) return vt;
  } while ( (he = he->nxt) != f->sphe );
  
  return (Spvt *) NULL;
}

Sped *get_lm_ppdedge( Spfc *f, Spvt *v1, Spvt *v2 )
{
  Sped *e;
  Sphe *he;
  
  if ( f == (Spfc *) NULL ) return (Sped *) NULL;

  he = f->sphe;

  do {
    e = he->ed;
    if ( ((e->sv == v1) && (e->ev == v2)) ||
	 ((e->ev == v1) && (e->sv == v2)) )
      return e;
  } while ( (he = he->nxt) != f->sphe );
  
  return (Sped *) NULL;
}

Spfc *get_lm_ppdface_rev( Spfc *f, Sped *e )
{
  Spfc *rf, *lf;

  if ( e == (Sped *) NULL ) return (Spfc *) NULL;
  
  rf = e->rf; lf = e->lf;

  if ( rf == f ) return lf;
  if ( lf == f ) return rf;
  
  return (Spfc *) NULL;
}

void append_ppdedge( Sped *ei, Spfc *f, Spfc *fj )
{
  if ( ei == (Sped *) NULL ) return;

  if ( ei->rf == f ) {
    ei->rf = fj; 
    return;
  }

  if ( ei->lf == f ) {
    ei->lf = fj;
    return;
  }

}

void append_ppdvertex( Spfc *f, Spvt *v1, Spvt *v2 )
{
  Sphe *he;

  if ( f == (Spfc *) NULL ) return;

  he = f->sphe;

  do {
    if ( he->vt == v1 ) {
      he->vt = v2;
      return;
    }
  } while ( (he = he->nxt) != f->sphe );

}

void append_ppdface( Spfc *f, Sped *e1, Sped *e2 )
{
  Sphe *he;
  
  if ( f == (Spfc *) NULL ) return;

  he = f->sphe;

  do {
    if ( he->ed == e1 ) {
      he->ed = e2;
      return;
    }
  } while ( (he = he->nxt) != f->sphe );
  
}

void free_vtxed_num( Spvt *v, Sped *e )
{
  Vted *ve;
  void free_vtxed( Vted *, Spvt * );

  if (v == (Spvt *) NULL) return;
  
  ve = v->svted;
  while ( ve != (Vted *) NULL ) {
    if ( ve->ed == e ) {
      free_vtxed( ve, v );
    }
    ve = ve->nxt;
  }
  
}

void change_ppdvertexlinks( Spvt *ev, Spvt *sv )
{
  Spfc  *f;
  Sped  *e;
  Sphe  *he;
  Vted  *vted;
  Vted  *create_vtxed( Spvt *, Sped * );

  if ( ev == (Spvt *) NULL ) return;

/*   display("CHANGELINKS: ev %d -> sv %d\n", ev->no, sv->no); */
  vted = ev->svted;
  while ( vted != (Vted *) NULL ) {
/*     display("\te %d rf %d lf %d\n", vted->ed->no, */
/* 	    (vted->ed->rf != NULL) ? vted->ed->rf->no : SMDNULL, */
/* 	    (vted->ed->lf != NULL) ? vted->ed->lf->no : SMDNULL ); */
    /* edge->vertex links, vertex->edge links */
    e = vted->ed;
    
    if ( e->sv == ev ) {
      e->sv = sv;
      (void) create_vtxed( sv, vted->ed );
    }
    if ( e->ev == ev ) {
      e->ev = sv;
      (void) create_vtxed( sv, vted->ed );
    }

    /* edge attributes */
    if ( e->atr == EDGEBOUNDARY ) {
      e->sv->atr = VTXEXTERNAL;
      e->ev->atr = VTXEXTERNAL;
    }
    
    /* vertex->face links */
    if ( e->rf != (Spfc *) NULL ) {
/*       display("\t\tchangelinks: fc %d\n", e->rf->no); */
      he = e->rf->sphe;
      do {
/* 	display("\t\the %d vt %d\n", he->no, he->vt->no); */
	if ( he->vt == ev ) he->vt = sv;
      } while ( (he = he->nxt) != e->rf->sphe );
    }

    if ( e->lf != (Spfc *) NULL ) {
/*       display("\t\tchangelinks: fc %d\n", e->lf->no); */
      he = e->lf->sphe;
      do {
/* 	display("\t\the %d vt %d\n", he->no, he->vt->no); */
	if ( he->vt == ev ) he->vt = sv;
      } while ( (he = he->nxt) != e->lf->sphe );
    }

    vted = vted->nxt;
  }

}

int collapse_condition( Star *star )
{
  int    cnt, n_vk;
  Sped   *ppde, *e;
  Spvt   *ppdv, *ppdsv, *ppdev, **vk;
  Vted   *vted;
  Starvt *starv;
  Spvt   *find_checkvertex( int, Spvt **, Spvt * );

  /* condition (1) */

  vk   = (Spvt **) malloc( star->vn * sizeof(Spvt *) );
  ppde = star->e; ppdsv = star->sv; ppdev = star->ev;
  
  n_vk = 0;
  for ( starv = star->spvt; starv != (Starvt *) NULL; starv = starv->nxt ) {

    cnt = 0;
    ppdv = starv->vt;
    for ( vted = ppdv->svted; vted != (Vted *) NULL; vted = vted->nxt ) {
      
      e = vted->ed;
      if ( (e->sv == ppdv) && (e->ev == ppdsv) ) ++cnt;
      if ( (e->sv == ppdv) && (e->ev == ppdev) ) ++cnt;
      if ( (e->ev == ppdv) && (e->sv == ppdsv) ) ++cnt;
      if ( (e->ev == ppdv) && (e->sv == ppdev) ) ++cnt;

      if ( cnt == 2 ) {
	
	if ( (find_checkvertex( n_vk, vk, ppdv )) == (Spvt *) NULL ) {
	  vk[n_vk] = ppdv; ++n_vk;

	  if ( ppde->atr != EDGEBOUNDARY ) {
	    if ( n_vk > 2 ) {
	      free(vk);
	      return SMD_OFF;
	    }
	  } else {
	    if ( n_vk > 1 ) {
	      free(vk);
	      return SMD_OFF;
	    }
	  }
	}
      }
      
    }
    
  }

  free(vk);

  /* condition (3) */
  
  if ( (ppdsv->atr != VTXEXTERNAL) && (ppdev->atr != VTXEXTERNAL) ) {
    
    if (star->vn < 5) return SMD_OFF;
    
  } else {
    
    if (star->vn < 4) return SMD_OFF;
    
  }

  /* boundary cases */
  /* condition (2) */

/*   if ( (ppdsv->atr == VTXEXTERNAL) && (ppdev->atr == VTXEXTERNAL) ) { */
  if ( (ppdsv->atr == VTXEXTERNAL) || (ppdev->atr == VTXEXTERNAL) ) {
/*   if ( (ppdev->atr == VTXEXTERNAL) ) { */
    
    if ( ppde->atr != EDGEBOUNDARY ) return SMD_OFF;
    
  }

  
  return SMD_ON;
}

Spvt *find_checkvertex( int n, Spvt **vs, Spvt *vt )
{
  int i;

  if ( vs == (Spvt **) NULL ) return (Spvt *) NULL;
  
  for ( i = 0; i < n; ++i ) {
    if ( vs[i] == vt ) return vt;
  }
  return (Spvt *) NULL;
}

void ppdtohppd( Sppd *ppd, HPpd *hppd )
{
  Spfc      *fc;
  Sphe      *he;
  HVertex   *shvt, *ehvt;
  HFace     *hfc;
  HEdge     *hed;
  HHalfedge *hhe;
  void      calc_hppdfnorm( HFace * );
  HFace     *create_hppdface( HPpd * );
  HEdge     *create_hppdedge( HPpd * );
  HHalfedge *create_hppdhalfedge( HFace * );
  HEdge     *find_hppdedge( HVertex *, HVertex *, HPpd * );
  void      calc_hppd_boundary( HPpd * );
  
  for ( fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt ) {
/*     display("fc %d\n", fc->no); */
    /* create_hface */
    hfc = create_hppdface( hppd );

    he = fc->sphe;
    do {
      hhe = create_hppdhalfedge( hfc );
      hhe->vt = he->vt->hvt;
/*       display("hhe %d hvt %d vt %d\n", he->no, hhe->vt->no, he->vt->no); */
    } while ( ( he = he->nxt ) != fc->sphe );

    /* make edges */
    hhe = hfc->shhe;
    do {
      shvt = hhe->vt;
      ehvt = hhe->nxt->vt;
      if ( (hed = find_hppdedge( shvt, ehvt, hppd )) == (HEdge *) NULL ) {
	hed = create_hppdedge( hppd );
	hed->sv = shvt;
	hed->ev = ehvt;
	hed->rf = hfc;
      } else {
	hed->lf = hfc;
      }
      hhe->hed = hed;
    } while ( ( hhe = hhe->nxt ) != hfc->shhe );

    calc_hppdfnorm( hfc  );
  }

  calc_hppd_boundary( hppd );
}

    

/* star functions */

Star *create_star( Sped *ed )
{
  Star   *star;
  Vted   *vted;
  Spvt   *vt;
  Starvt *starvt;
  Star   *init_star( void );
  Starvt *find_starvertex( Star *, Spvt * );
  Spvt   *another_vt( Sped *, Spvt * );
  Starvt *create_starvt( Star * );
    
  star = init_star();
  
  star->e  = ed;
  star->sv = ed->sv;
  star->ev = ed->ev;

  /* sv */
  for ( vted = ed->sv->svted; vted != (Vted *) NULL; vted = vted->nxt ) {
    vt = another_vt( vted->ed, ed->sv );
    if ( find_starvertex( star, vt ) == (Starvt *) NULL ) {
      starvt = create_starvt( star );
      starvt->vt = vt;
    }
  }
  /* ev */
  for ( vted = ed->ev->svted; vted != (Vted *) NULL; vted = vted->nxt ) {
    vt = another_vt( vted->ed, ed->ev );
    if ( find_starvertex( star, vt ) == (Starvt *) NULL ) {
      starvt = create_starvt( star );
      starvt->vt = vt;
    }
  }

  return star;
}

Star *init_star( void )
{
  Star *star;

  star     = (Star *) malloc( sizeof( Star ) );
  star->e  = (Sped *) NULL;
  star->sv = (Spvt *) NULL;
  star->ev = (Spvt *) NULL;

  star->vn = 0;
  star->spvt = star->epvt = (Starvt *) NULL;
  
  return star;
}

Starvt *find_starvertex( Star *star, Spvt *vt )
{
  Starvt *sv;
  
  for ( sv = star->spvt; sv != (Starvt *) NULL; sv = sv->nxt ) {
    if ( sv->vt == vt ) return sv;
  }
  return (Starvt *) NULL;
}

Starvt *create_starvt( Star *star )
{
  Starvt *vt;
  
  vt = (Starvt *) malloc( sizeof(Starvt) );

  vt->nxt = (Starvt *) NULL;
  if (star->spvt == (Starvt *) NULL) {
    vt->prv   = (Starvt *) NULL;
    star->epvt = star->spvt = vt;
  } else {
    vt->prv   = star->epvt;
    star->epvt = star->epvt->nxt = vt;
  }
  vt->vt = (Spvt *) NULL;

  vt->no = star->vn;
  ++( star->vn );

  return vt;
}

void free_starvertex( Starvt *vt, Star *star )
{
  if (vt == (Starvt *) NULL) return;
  
  if ( star->spvt == vt ) {
    if ( (star->spvt = vt->nxt) != (Starvt *) NULL )
      vt->nxt->prv = (Starvt *) NULL;
    else {
      star->epvt = (Starvt *) NULL;
    }
  } else if (star->epvt == vt) {
    vt->prv->nxt = (Starvt *) NULL;
    star->epvt = vt->prv;
  } else {
    vt->prv->nxt = vt->nxt;
    vt->nxt->prv = vt->prv;
  }

  free(vt);
  
  --( star->vn );
}

void free_star( Star *star )
{
  Starvt *vt, *nvt;
  void free_starvertex( Starvt *, Star * );

  /* vertex */
  for (vt = star->spvt; vt != (Starvt *) NULL; vt = nvt) {
    nvt = vt->nxt;
    free_starvertex(vt, star);
  }

  free(star);
}

/* clear functions */
void clear_hppdlod( HPpd *hppd )
{
  HVertex *hvt;
  HFace *hfc, *nhfc;
  void FreeHppdEdge( HPpd * );
  void FreeHppdFace( HPpd * );
  void clear_ppd( Sppd * );

  
  if ( hppd == (HPpd *) NULL ) return;

  FreeHppdEdge( hppd );
  FreeHppdFace( hppd );

  /* clear ppds */
  clear_ppd( hppd->ppd1 );
  clear_ppd( hppd->ppd2 );

  for ( hvt = hppd->shvt; hvt != (HVertex *) NULL; hvt = hvt->nxt ) {
    hvt->vt1->sp_type = SP_VERTEX_HVERTEX;
    hvt->vt2->sp_type = SP_VERTEX_HVERTEX;
  }
    
}

void clear_ppd( Sppd *ppd )
{
  int  i;
  Spvt *vt, *nvt;
  Sped *ed, *ned;
  Spfc *fc;
  Vted *ve, *nve;
  void free_vtxed(Vted *, Spvt *);
  void free_ppdvertex( Spvt *, Sppd * );
  void free_ppdvertexedge( Spvt * );
  void free_ppdedge( Sped *, Sppd * );
  Vted *create_vtxed( Spvt *, Sped * );
  double V3DistanceBetween2Points( Vec *, Vec * );
  void edge_length( Sppd * );

  if ( ppd == (Sppd *) NULL ) return;

  /* delete vertices */
  for ( i = 0, vt = ppd->spvt; vt != (Spvt *) NULL; vt = nvt ) {

    /* delete vertex-edge links */
    nvt = vt->nxt;

    free_ppdvertexedge(vt);
    
    vt->nvt = (Spvt *) NULL;
    
    vt->pq_type = EMPTY;
    vt->pqc     = (PQCont *) NULL;
    vt->sp_ed   = (Sped *) NULL;
    vt->sp_val  = 0.0;
    vt->sp_type = SP_VERTEX_NORMAL;

    if ( vt->type != VERTEX_ORIGINAL ) {
      free_ppdvertex( vt, ppd );
    } else {
      vt->no = i;
      ++i;
    }

  }
  ppd->vn = i; ppd->vid = i;
  
  /* delete edges */
  for ( i = 0, ed = ppd->sped; ed != (Sped *) NULL; ed = ned ) {
  
    ned = ed->nxt;
    ed->ned = (Sped *) NULL;
    
    ed->cal_subgraph = SMD_OFF;
    ed->sp_type = SP_EDGE_NORMAL;

    if ( ed->type != EDGE_ORIGINAL ) {
      free_ppdedge( ed, ppd );
    } else {
      ed->no = i;
      ++i;
    }
  }
  ppd->en = ppd->eid = i;
  
  for ( fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt ) {
    fc->nfc = (Spfc *) NULL;
  }

  /* create vertex-edge links */
  for ( ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt ) {
    create_vtxed( ed->sv, ed );
    create_vtxed( ed->ev, ed );
  }
  
  edge_length( ppd );

/*   printppd( ppd ); */
}



