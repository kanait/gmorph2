/* Copyright (c) 1995-1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"
#include "mtl.h"
#include "color.h"

/* #define SPHERE_RADIUS  0.1 */
#define SPHERE_RADIUS  0.01
#define SMALL_SPHERE_RADIUS  0.008
/* #define SMALL_SPHERE_RADIUS  0.05 */
/* #define TUBE_RADIUS    0.05 */
#define TUBE_RADIUS    0.005
#define NUM_GROUP_EDGE 5

Material line_mat[] = {
  0.187004, 0.138930, 0.138930, 1.000000,
  0.748016, 0.75721,  0.155721, 1.000000,
  0.000000, 0.000000, 0.000000, 1.000000,
  0.836364, 0.836364, 0.836364, 1.000000,
  77.575806
};

Material pointred_mat[] = {
  0.187004, 0.000000, 0.000000, 1.000000,
  0.748016, 0.000000, 0.000000, 1.000000,
  0.000000, 0.000000, 0.000000, 1.000000,
  0.672727, 0.672727, 0.672727, 1.000000,
  29.478785
};

Material point_mat[] = {
  0.005183, 0.084911, 0.084911, 1.000000,
  0.020732, 0.339645, 0.339645, 1.000000,
  0.000000, 0.000000, 0.000000, 1.000000,
  0.903030, 0.903030, 0.903030, 1.000000,
  118.690941
};

Material vertex_mat[] = {
  0.000000, 0.187004, 0.187004, 1.000000, 
  0.000000, 0.748016, 0.748016, 1.000000, 
  0.000000, 0.000000, 0.000000, 1.000000, 
  0.981818, 0.981818, 0.981818, 1.000000, 
  92.315140
};

Material edge_mat[] = {
  0.038095, 0.036285, 0.036285, 1.000000, 
  0.152381, 0.145141, 0.145141, 1.000000, 
  0.000000, 0.000000, 0.000000, 1.000000, 
  0.745455, 0.745455, 0.745455, 1.000000, 
  122.569725
};

void drawppd_vertex( Sppd *ppd, ScreenAtr *screen )
{
  int  i;
  Spvt *v;
  Vec  *vec;
  char buf[BUFSIZ];
  
  if ( ppd == NULL ) return;
  
  glPointSize(4.0);
  
  for ( v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {

    glColor3dv( greenvec );
    
    if ( v->col == PNTRED ) glColor3dv( redvec );
    
    glBegin(GL_POINTS);
    vec = &(v->vec);
    glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
    glEnd();

  }
}

void drawppd_vertex_enhanced( Sppd *ppd, ScreenAtr *screen )
{
  Spvt *v;
  double radius;
  void glu_sphere( Vec *, double );
  void  material_binding( Material * );

  if ( ppd == NULL ) return;
  
  glEnable(GL_LIGHTING);

  radius = SPHERE_RADIUS * ppd->scale;
  
  for ( v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {

    if ( v->type == VERTEX_ORIGINAL ) {
      
      switch ( v->sp_type ) {
      case SP_VERTEX_HVERTEX:
	material_binding( tropical20 );
	break;
      default:
	material_binding( point_mat );
	break;
      }
      if ( v->col == PNTRED ) material_binding( pointred_mat );
    
      glu_sphere( &(v->vec), SPHERE_RADIUS );
    
    }

  }
  
  glDisable(GL_LIGHTING);
}

void drawppd_edge( Sppd *ppd, ScreenAtr *screen )
{
  int  i;
  Sped *e;
  Spvt *vt;
  Vec  *svec, *evec;
  char buf[BUFSIZ];
  int printstring(ScreenAtr *, char *);
  
  if ( ppd == NULL ) return;
  
  glLineWidth((GLfloat) 2.0);
 
  /* edge */

  for (e = ppd->sped; e != (Sped *) NULL; e = e->nxt) {

    if ( e->type == EDGE_ORIGINAL ) {

/*       glColor3dv( blackvec ); */
      glColor3dv( gray50vec );
      if ( e->fn == 1 ) {
	if ( e->used_loop == SMD_OFF ) {
	  glColor3dv( bluevec );
	}
      }
      if ( e->fn > 2 ) {
	glColor3dv( redvec );
      }

      svec = &(e->sv->vec);
      evec = &(e->ev->vec);
      glBegin(GL_LINE_STRIP);
      glVertex3d( (GLdouble) svec->x, (GLdouble) svec->y, (GLdouble) svec->z );
      glVertex3d( (GLdouble) evec->x, (GLdouble) evec->y, (GLdouble) evec->z );
      glEnd();
    }
    
  }
}

void drawppd_hiddenline( Sppd *ppd, ScreenAtr *screen )
{
  Spfc  *f;
  Sphe  *he;
  Vec   *vec, *nrm;
  void  material_binding( Material * );
  
  glLineWidth( (GLfloat) 1.0 );
  glColor3dv( blackvec );

  glEnable(GL_STENCIL_TEST);
  glClear(GL_STENCIL_BUFFER_BIT);
  glStencilFunc(GL_ALWAYS, 0, 1);
  glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);

  for ( f = ppd->spfc; f != NULL; f = f->nxt ) {

    he = f->sphe;
    glBegin(GL_LINE_LOOP);
    do {
      vec = &(he->vt->vec);
      glVertex3d((GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z);
    } while ((he = he->nxt) != f->sphe);
    glEnd();
      
    /*** polygon shading (by material or background color) ***/
/*     if (swin->dis3d.shading) { */
      
      material_binding(swin->material);
      glEnable(GL_LIGHTING);
      glStencilFunc(GL_EQUAL, 0, 1);
      glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
      
      glBegin( GL_POLYGON );
      he = f->sphe;
      nrm = &(f->nrm);
      do {
	glNormal3d((GLdouble) nrm->x, (GLdouble) nrm->y, (GLdouble) nrm->z);
	vec = &(he->vt->vec);
	glVertex3d((GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z);
      } while ((he = he->nxt) != f->sphe);
      glEnd();
      
      glDisable(GL_LIGHTING);
      
/*     } else { */
/*       glColor3f( screen->bgrgb[0], screen->bgrgb[1], screen->bgrgb[2] ); */
      
/*       glShadeModel(GL_FLAT); */
/*       glStencilFunc(GL_EQUAL, 0, 1); */
/*       glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); */
/*       glBegin(GL_POLYGON); */
/*       do { */
/* 	vec = &(he->vt->vec); */
/* 	glVertex3d((GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z); */
/*       } while ((he = he->nxt) != f->sphe); */
/*       glEnd(); */
/*     } */
      
    /*** edge drawing (2nd stencil) ***/      
    glStencilFunc(GL_ALWAYS, 0, 1);
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
    
    he = f->sphe;
    glBegin(GL_LINE_LOOP);
    do {
      vec = &(he->vt->vec);
      glVertex3d((GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z);
    } while ((he = he->nxt) != f->sphe);
    glEnd();
    
  }
}


void drawppd_sged( SGraph *sg, ScreenAtr *screen, GLdouble *color, GLfloat width )
{
  SGvt *v;
  SGed *e;
  Vec  *svec, *evec;
  char buf[BUFSIZ];
  int  printstring( ScreenAtr *, char * );
  
  if ( sg == NULL ) return;
  
/*   glPointSize(7.0); */
/*   for ( v = sg->sgvt; v != (SGvt *) NULL; v = v->nxt ) { */
/*     svec = &(v->vec); */

/*     glBegin( GL_POINTS ); */
/*     glColor3dv( lightbluevec ); */
/*     glVertex3d( (GLdouble) svec->x, (GLdouble) svec->y, (GLdouble) svec->z ); */
/*     glEnd(); */
    
/*     glColor3dv( redvec ); */
/*     glRasterPos3d( (GLdouble) svec->x, (GLdouble) svec->y, (GLdouble) svec->z ); */
/*     sprintf(buf, "%d", v->no ); */
/*     (void) printstring( screen, buf ); */

/*   } */
  
  glLineWidth((GLfloat) width );

  /* edge */
  for ( e = sg->sged; e != (SGed *) NULL; e = e->nxt ) {

    glColor3dv( color );
    svec = &(e->sv->vec);
    evec = &(e->ev->vec);
    glBegin( GL_LINE_STRIP );
    glVertex3d( (GLdouble) svec->x, (GLdouble) svec->y, (GLdouble) svec->z );
    glVertex3d( (GLdouble) evec->x, (GLdouble) evec->y, (GLdouble) evec->z );
    glEnd();

/*     glColor3dv( redvec ); */
/*     glRasterPos3d( (GLdouble) (svec->x+evec->x) /2.0, */
/* 		   (GLdouble) (svec->y+evec->y) /2.0, */
/* 		   (GLdouble) (svec->z+evec->z) /2.0 ); */
/*     sprintf(buf, "%d", e->no ); */
/*     (void) printstring( screen, buf ); */

  }
}

void drawppd_edge_enhanced(Sppd *ppd, ScreenAtr *screen)
{
  int  i;
  unsigned short id;
  Sped *e;
  Spvt *vt;
  double radius;
  char buf[BUFSIZ];
  int printstring(ScreenAtr *, char *);
  void glu_cylinder( Vec *, Vec *, double );
  void  material_binding(Material *);
  
  if ( ppd == NULL ) return;
  
  glEnable(GL_LIGHTING);
  material_binding( edge_mat );
  
  /* edge */

  radius = TUBE_RADIUS * ppd->scale;
  
  for (e = ppd->sped; e != (Sped *) NULL; e = e->nxt) {

    if ( e->type == EDGE_ORIGINAL ) {
      glu_cylinder( &(e->sv->vec), &(e->ev->vec) , radius );
    }

  }
  
  glDisable(GL_LIGHTING);

}

void drawppd_shading(Sppd *ppd, ScreenAtr *screen)
{
  int   i, j;
  Spfc  *f;
  Sphe  *he;
  Splv  *lv;
  Splp  *lp;
  Spvt  *vt;
  Vec   *vec;
  void  material_binding(Material *);

  if ( ppd == NULL ) return;

  material_binding( swin->material );
  
  for (f = ppd->spfc; f != (Spfc *) NULL;f = f->nxt) {
    
    if (f->col == FACEBLUE) {
      
      glEnable(GL_LIGHTING);
      glBegin(GL_POLYGON);
      he = f->sphe;
      do {
	
	if ( (he->nm != (Spnm *) NULL) && (swin->smooth_shading == SMD_ON) ) {
	  vec = &(he->nm->vec);
	  glNormal3d((GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
	} else {
	  glNormal3d((GLdouble) f->nrm.x, (GLdouble) f->nrm.y, (GLdouble) f->nrm.z);
	}
	vec = &(he->vt->vec);
	glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
	
      } while ((he = he->nxt) != f->sphe);
      glEnd();
      glDisable(GL_LIGHTING);
      
    } else if (f->col == FACERED) {
      
      glColor3dv( redvec );
      glBegin(GL_POLYGON);
      he = f->sphe;
      do {
	vec = &(he->vt->vec);
	glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
      } while ((he = he->nxt) != f->sphe);
      glEnd();
      
    } else if (f->col == FACEGREEN) {
      
      glColor3dv(greenvec);
      glBegin(GL_POLYGON);
      he = f->sphe;
      do {
	vec = &(he->vt->vec);
	glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
      } while ((he = he->nxt) != f->sphe);
      glEnd();
      
    }
  }
}

void drawhppd_cmesh(HPpd *hppd, ScreenAtr *screen)
{
  HEdge     *hed;
  HVertex   *hvt, *shvt, *ehvt;
  HFace     *hfc;
  HHalfedge *he;
  Vec  *nrm;
  Spvt      *vt, *sv, *ev;
  void  material_binding(Material *);
  
  glPointSize(4.0);
  glLineWidth((GLfloat) 2.0);
  
  /* hvertices */
  glBegin( GL_POINTS );
  for ( hvt = hppd->shvt; hvt != (HVertex *) NULL; hvt = hvt->nxt ) {
    
    vt = ( !screen->no ) ? hvt->vt1 : hvt->vt2;
    
    if ( hvt->atr == VTXINTERNAL ) glColor3dv( darkorange4vec );
    else glColor3dv( magenta4vec );

    if ( hvt->col != PNTGREEN ) glColor3dv( redvec );

    glVertex3d((GLdouble) vt->vec.x, (GLdouble) vt->vec.y, (GLdouble) vt->vec.z);

  }
  glEnd();

  /* hedges */
  
    for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {
    
    shvt = hed->sv;
    ehvt = hed->ev;
    sv = ( !screen->no ) ? shvt->vt1 : shvt->vt2;
    ev = ( !screen->no ) ? ehvt->vt1 : ehvt->vt2;

    if ( hed->atr == EDGEINTERNAL ) glColor3dv( darkorange4vec );
    else glColor3dv( magenta4vec );

    glBegin(GL_LINE_STRIP);
    glVertex3d((GLdouble) sv->vec.x, (GLdouble) sv->vec.y, (GLdouble) sv->vec.z);
    glVertex3d((GLdouble) ev->vec.x, (GLdouble) ev->vec.y, (GLdouble) ev->vec.z);
    glEnd();
    
  }
  
  /* faces */
  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {

    if ( hfc->col != FACERED ) {
      
      glEnable( GL_LIGHTING );
      material_binding( swin->material );
  
      glBegin(GL_POLYGON);
      he = hfc->shhe;
      nrm = ( !screen->no ) ? &(hfc->nrm1) : &(hfc->nrm2);
      do {
	shvt = he->vt;
	sv = ( !screen->no ) ? shvt->vt1 : shvt->vt2;
	glNormal3d((GLdouble) nrm->x, (GLdouble) nrm->y, (GLdouble) nrm->z);
	glVertex3d((GLdouble) sv->vec.x, (GLdouble) sv->vec.y, (GLdouble) sv->vec.z);
      
      } while ((he = he->nxt) != hfc->shhe);
      glEnd();
      
      glDisable( GL_LIGHTING );
      
    } else {

      glColor3dv( redvec );
      glBegin( GL_POLYGON );
      he = hfc->shhe;
      do {
	shvt = he->vt;
	sv = ( !screen->no ) ? shvt->vt1 : shvt->vt2;
	glVertex3d( (GLdouble) sv->vec.x, (GLdouble) sv->vec.y, (GLdouble) sv->vec.z );
      
      } while ((he = he->nxt) != hfc->shhe);
    
      glEnd();


    }      
    
  }
    
}

void drawhppd_cmesh_enhanced(HPpd *hppd, ScreenAtr *screen)
{
  HEdge     *hed;
  HVertex   *hvt, *shvt, *ehvt;
  Spvt      *vt, *sv, *ev;
  HFace     *hfc;
  HHalfedge *he;
  Vec   *nrm;
  void  glu_sphere( Vec *, double );
  void  glu_cylinder( Vec *, Vec *, double );
  void  material_binding( Material * );
  
  /* hvertices */
  glEnable(GL_LIGHTING);
  
  for ( hvt = hppd->shvt; hvt != (HVertex *) NULL; hvt = hvt->nxt ) {
    
    if ( screen->no == SCREEN1 ) vt = hvt->vt1;
    else                         vt = hvt->vt2;
    
    if ( hvt->col == PNTGREEN ) material_binding( point_mat );
    else material_binding( pointred_mat );
    
    glu_sphere( &(vt->vec), SPHERE_RADIUS );
  }

  /* hedges */
  
  glLineWidth((GLfloat) 2.0);
  for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {
    
    shvt = hed->sv;
    ehvt = hed->ev;
    if ( screen->no == SCREEN1 ) {
      sv = shvt->vt1;
      ev = ehvt->vt1;
    } else {
      sv = shvt->vt2;
      ev = ehvt->vt2;
    }
    
    material_binding( line_mat );

    glu_cylinder( &(sv->vec), &(ev->vec) , TUBE_RADIUS );
    
  }

  /* faces */
  material_binding( swin->material );
  
  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {

    glBegin( GL_POLYGON );
    
    nrm = ( !screen->no ) ? &(hfc->nrm1) : &(hfc->nrm2);
    he = hfc->shhe;
    do {
      shvt = he->vt;
      sv = ( !screen->no ) ? shvt->vt1 : shvt->vt2;
      glNormal3d((GLdouble) nrm->x, (GLdouble) nrm->y, (GLdouble) nrm->z);
      glVertex3d((GLdouble) sv->vec.x, (GLdouble) sv->vec.y, (GLdouble) sv->vec.z);
      
    } while ((he = he->nxt) != hfc->shhe);
    
    glEnd();
  }
    
  glDisable( GL_LIGHTING );
}

void drawhppd_cpoint_enhanced(HPpd *hppd, ScreenAtr *screen)
{
  HEdge     *hed;
  HVertex   *hvt, *shvt, *ehvt;
  Spvt      *vt, *sv, *ev;
  HFace     *hfc;
  HHalfedge *he;
  Vec   *nrm;
  void  glu_sphere( Vec *, double );
  void  glu_cylinder( Vec *, Vec *, double );
  void  material_binding( Material * );
  
  /* hvertices */
  glEnable(GL_LIGHTING);
  
  for ( hvt = hppd->shvt; hvt != (HVertex *) NULL; hvt = hvt->nxt ) {
    
    if ( screen->no == SCREEN1 ) vt = hvt->vt1;
    else                         vt = hvt->vt2;
    
    if ( hvt->col == PNTGREEN ) material_binding( point_mat );
    else material_binding( pointred_mat );
    
    glu_sphere( &(vt->vec), SPHERE_RADIUS );
  }

  glDisable( GL_LIGHTING );
}

/* grouping */
void drawhppd_group(HPpd *hppd, ScreenAtr *screen, int tileno)
{
  HFace *hfc;
  HGfc  *hgfc;
  HGsf  *sf;
  HGvt  *hv;
  Spvt  *sv, *ev;
  HGed  *he;
  HGhe  *hghe;
  
  if ( hppd == (HPpd *) NULL ) return;
  if ( hppd->shfc == (HFace *) NULL ) return;
  
  glPointSize(4.0);
  glLineWidth((GLfloat) 3.0);
  glColor3dv( blackvec );
    
  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {

    if (hfc->no != tileno) continue;

    if ( !screen->no ) hgfc = hfc->hgfc1; else hgfc = hfc->hgfc2;
    if ( hgfc == (HGfc *) NULL ) continue;
    
    for ( sf = hgfc->shgsf; sf != (HGsf *) NULL; sf = sf->nxt ) {

      hghe = sf->shghe;

      do {
	sv = hghe->vt->vt;
	ev = hghe->nxt->vt->vt;
	glBegin(GL_LINE_STRIP);
	glVertex3d((GLdouble) sv->vec.x, (GLdouble) sv->vec.y, (GLdouble) sv->vec.z);
	glVertex3d((GLdouble) ev->vec.x, (GLdouble) ev->vec.y, (GLdouble) ev->vec.z);
	glEnd();
      } while ( (hghe = hghe->nxt) != sf->shghe );
      
    }
    
  }
  
}

void drawhppd_group_enhanced(HPpd *hppd, ScreenAtr *screen)
{
  HFace *hfc;
  HGfc  *hgfc;
  HGvt  *hv;
  HGed  *he;
  void glu_sphere( Vec *, double );
  void glu_cylinder( Vec *, Vec *, double );
  void material_binding(Material *);

  if ( hppd == (HPpd *) NULL ) return;
  if ( hppd->shfc == (HFace *) NULL ) return;
  
  glEnable(GL_LIGHTING);

  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {

    switch ( hfc->no %  NUM_GROUP_EDGE ) {
    case 0:
      material_binding( tropical0 );
      break;
    case 1:
      material_binding( tropical13 );
      break;
    case 2:
      material_binding( tropical6 );
      break;
    case 3:
      material_binding( tropical24 );
      break;
    case 4:
      material_binding( tropical29 );
      break;
    }
    
    if ( !screen->no ) hgfc = hfc->hgfc1; else hgfc = hfc->hgfc2;

    if ( hgfc != (HGfc *) NULL ) {

      /* vertices */
      for ( hv = hgfc->shgvt; hv != (HGvt *) NULL; hv = hv->nxt ) {
	glu_sphere( &(hv->vt->vec), SPHERE_RADIUS );
      }
      
      /* edges */
      for ( he = hgfc->shged; he != (HGed *) NULL; he = he->nxt ) {
	glu_cylinder( &(he->ed->sv->vec), &(he->ed->ev->vec) , TUBE_RADIUS );
      }

    }
  }
  
  glDisable(GL_LIGHTING);
}

/* hloop */
void drawhppd_hloop(HPpd *hppd, ScreenAtr *screen)
{
  HFace *hfc;
  Splp    *lp;
  Splv    *lv;
  
  glPointSize(4.0);
  glLineWidth((GLfloat) 2.0);

  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {

    if ( hfc->hloop != (HLoop *) NULL ) {
      
      if ( !screen->no ) lp = hfc->hloop->lp1; else lp = hfc->hloop->lp2;

      glColor3dv( redvec );
      
      glBegin(GL_LINE_LOOP);
      for (lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt) {
	glVertex3d((GLdouble) lv->vt->vec.x,
		   (GLdouble) lv->vt->vec.y,
		   (GLdouble) lv->vt->vec.z);
      }
      glEnd();

      for (lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt) {
	
	if ( lv->vt->col != PNTRED ) glColor3dv( orangevec );
	  else glColor3dv( redvec );
	
	glBegin(GL_POINTS);
	glVertex3d((GLdouble) lv->vt->vec.x,
		   (GLdouble) lv->vt->vec.y,
		   (GLdouble) lv->vt->vec.z);
	glEnd();
      }
    }
  }
}

void drawhppd_shortestpath(HPpd *hppd, ScreenAtr *screen)
{
  HEdge   *hed;
  Splp    *lp;
  Splv    *lv;
  Spvt    *vt;
  
  glLineWidth((GLfloat) 4.0);
  
  for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {
    
    if ( screen->no == SCREEN1 ) lp = hed->lp1;
    else lp = hed->lp2;

    if ( lp != (Splp *) NULL ) {
      
      if ( lp->col != LOOPRED ) glColor3dv( indianred4vec );
      else glColor3dv( redvec );
      
      glBegin(GL_LINE_STRIP);
      for (lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt) {
	glVertex3d( (GLdouble) lv->vt->vec.x,
		    (GLdouble) lv->vt->vec.y,
		    (GLdouble) lv->vt->vec.z );
      }
      glEnd();

      glPointSize( 7.0 );
      glBegin( GL_POINTS );
      for (lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt) {

	vt = lv->vt;

	if ( vt->sp_type == SP_VERTEX_HVERTEX ) continue;
	
	glColor3dv( orangevec );
	glVertex3d((GLdouble) lv->vt->vec.x, (GLdouble) lv->vt->vec.y,
		   (GLdouble) lv->vt->vec.z);
      }
      glEnd();
    }

  }

}

void drawppd_loop( Sppd *ppd, ScreenAtr *screen )
{
  Splp *lp;
  void draw_loop( Splp * );
  
  for ( lp = ppd->splp; lp != (Splp *) NULL; lp = lp->nxt ) {

    draw_loop( lp );

  }
}

void draw_loop( Splp *lp )
{
  Splv *lv;
  Spvt *vt;
  Vec  *vec;

  if ( lp == NULL ) return;
  /* loops */
  
  glLineWidth((GLfloat) 2.0);

  switch ( lp->type ) {
  case SHORTESTPATH:
    glColor3dv( red3vec );
    break;
  default:
    glColor3dv( bluevec );
  }

  switch ( lp->type ) {
  case CLOSEDLOOP:
    glBegin( GL_LINE_LOOP );
    break;
  default:
    glBegin( GL_LINE_STRIP );
    break;
  }
    
  for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    vec = &(lv->vt->vec);
    glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
  }
  glEnd();

  glPointSize(7.0);
  
  for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    
    vt = lv->vt;
    
    if ( vt->col != PNTRED ) glColor3dv( orangevec );
    else glColor3dv( redvec );

    if ( (vt->sp_type != SP_VERTEX_HVERTEX) && (vt->spn > 1 ) ) {
      glColor3dv( bluevec );
    }
    
    glBegin(GL_POINTS);
    glVertex3d( (GLdouble) vt->vec.x, (GLdouble) vt->vec.y, (GLdouble) vt->vec.z );
    glEnd();
  }
  
}

void draw_sglp( SGlp *lp )
{
  SGlpvt *lv;
  Vec  *vec;

  if ( lp == NULL ) return;
  /* loops */
  
  glLineWidth( (GLfloat) 4.0 );
  glColor3dv( bluevec );

  glBegin( GL_LINE_STRIP );
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = lv->nxt ) {
    vec = &( lv->vt->vec );
    glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
  }
  glEnd();

  glPointSize( 8.0 );
  glColor3dv( orangevec );
  
  glBegin( GL_POINTS );
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = lv->nxt ) {
    vec = &( lv->vt->vec );
    glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
  }
  glEnd();
}

void drawppd_loop_enhanced( Sppd *ppd, ScreenAtr *screen )
{
  Splp *lp;
  void draw_loop_enhanced( Splp * );
  
  for ( lp = ppd->splp; lp != (Splp *) NULL; lp = lp->nxt ) {

    draw_loop_enhanced( lp );

  }
}

void draw_loop_enhanced( Splp *lp )
{
  Splv *lv;
  Spvt *vt;
  void glu_sphere( Vec *, double );
  void  material_binding(Material *);

  if ( lp == NULL ) return;
  /* loops */
  
    glLineWidth((GLfloat) 2.0);

  
  switch ( lp->type ) {
  case SHORTESTPATH:
    glColor3dv( red3vec );
    break;
  default:
    glColor3dv( bluevec );
  }

  switch ( lp->type ) {
  case CLOSEDLOOP:
    glBegin( GL_LINE_LOOP );
    break;
  default:
    glBegin( GL_LINE_STRIP );
    break;
  }
    
  for (lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt) {
    glVertex3d( (GLdouble) lv->vt->vec.x,
		(GLdouble) lv->vt->vec.y,
		(GLdouble) lv->vt->vec.z );
  }
  glEnd();

  glEnable( GL_LIGHTING );
  for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    vt = lv->vt;
    switch ( vt->sp_type ) {
    case SP_VERTEX_HVERTEX:
      if ( vt->col != PNTRED ) material_binding( point_mat );
      else material_binding( pointred_mat );
      glu_sphere( &(vt->vec), SPHERE_RADIUS );
      break;
    default:
      if ( vt->col != PNTRED ) material_binding( vertex_mat );
      else material_binding( pointred_mat );
      glu_sphere( &(vt->vec), SMALL_SPHERE_RADIUS );
      break;
    }
  }
  glDisable( GL_LIGHTING );
}

void drawhppd_hmap( HPpd *hppd, ScreenAtr *screen )
{
  int   i;
  HGfc  *hgfc;
  HFace *hf;
  HGed  *he;
  HGvt  *hv;
  HGsf  *hs;
  HGhe  *hghe;
  Vec   *vec;
  void  material_binding(Material *);
  
  glLineWidth((GLfloat) 2.0);
  glPointSize(4.0);
  glColor3dv( blackvec );

  if ( hppd == NULL ) return;
  
  for ( hf = hppd->shfc; hf != (HFace *) NULL; hf = hf->nxt ) {

    hgfc = ( !screen->no ) ? hf->hgfc1->mhgfc : hf->hgfc2->mhgfc;

    /* edge */
    for ( he = hgfc->shged; he != (HGed *) NULL; he = he->nxt ) {

      if ( (he->rf == NULL) || ( he->rf == NULL) ) {
	glColor3dv( redvec );
      } else {
	glColor3dv( blackvec );
      }
	
      glBegin(GL_LINE_STRIP);
      vec = &(he->sv->vec);
      glVertex3d((GLdouble) vec->x, (GLdouble) vec->y,
		 (GLdouble) vec->z);
      vec = &(he->ev->vec);
      glVertex3d((GLdouble) vec->x, (GLdouble) vec->y,
		 (GLdouble) vec->z);
      glEnd();
    }

    /* vertex */
    glBegin( GL_POINTS );
    glColor3dv( blackvec );
    for ( hv = hgfc->shgvt; hv != (HGvt *) NULL; hv = hv->nxt ) {
      glVertex3d( (GLdouble) hv->vec.x,
		 (GLdouble) hv->vec.y,
		 (GLdouble) hv->vec.z );
    }
    glEnd();

    /* surface */
    glEnable( GL_LIGHTING );
    material_binding( metal10 );
    
    for ( hs = hgfc->shgsf; hs != (HGsf *) NULL; hs = hs->nxt ) {
      
      glBegin( GL_POLYGON );
      
      hghe = hs->shghe;

      do { 
	
	glNormal3d( (GLdouble) hs->nrm.x,
		    (GLdouble) hs->nrm.y,
		    (GLdouble) hs->nrm.z );
	
	vec = &(hghe->vt->vec);
	glVertex3d( (GLdouble) vec->x,
		    (GLdouble) vec->y,
		    (GLdouble) vec->z );
	
      } while ( ( hghe = hghe->nxt ) != hs->shghe );
      
      glEnd();
      
    }

    glDisable(GL_LIGHTING);
      
  }
}

