/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include <stdio.h>
#include <string.h>

#include "motif.h"
#include "gldef.h"
#include "smd.h"

#include "Progress.h"

/*******************************************************************************
  The Geometric Morphing Main Routine
*******************************************************************************/
  
Sppd *ppdgmorph_v2( HPpd *hppd )
{
  Sppd  *gppd;
  void  initialize_gmorphv2( HPpd * );
  void  hppd_to_hgppd( HPpd * );
  void  hppdharmonic( HPpd * );
  Sppd  *hppd_to_gppd( HPpd * );
  void  calc_morph_vector(HPpd *, Sppd *);
  void  after_morphing( Sppd * );
  void  time_start(void);
  void  time_stop(void);
  void  mhgfc_harmonicmap_to_ps( HPpd * );
  void write_ppd_file(char *, Sppd *);

  gppd = (Sppd *) NULL;
  
  if ( hppd == (HPpd *) NULL ) {
    displayinfo("hppd is not found.\n");
    return (Sppd *) NULL;
  }
  if ( (hppd->ppd1 == (Sppd *) NULL) || (hppd->ppd2 == (Sppd *) NULL) ) {
    displayinfo("2 ppds are not found.\n");
    return (Sppd *) NULL;
  }
  if ( !hppd->fn ) {
    displayinfo("hface is not found.\n");
    return (Sppd *) NULL;
  }

  ProgressSetValue( 0 );
  displayinfo("STEP 0: initialize... ");
  initialize_gmorphv2( hppd );
  displayinfo("done. \n");
  ProgressSetValue( 20 );
  
  displayinfo("STEP 1: grouping... ");
  hppd_to_hgppd( hppd );
  displayinfo("done. \n");
  ProgressSetValue( 40 );
  
  displayinfo("STEP 2: harmonic map... ");
  hppdharmonic( hppd );
  displayinfo("done. \n");
  ProgressSetValue( 60 );

  time_start();
  displayinfo("STEP 3: creating new ppd... ");
  gppd = hppd_to_gppd( hppd );
  displayinfo("done.\n");
  ProgressSetValue( 80 );

  displayinfo("STEP 4: calc morphing vector... ");
  calc_morph_vector( hppd, gppd );
  after_morphing( gppd );
  displayinfo("done.\n");
  ProgressSetValue( 100 );

  displayinfo("new ppd: vertex %d edge %d face %d\n", gppd->vn,
	      gppd->en, gppd->fn);
  display("time:combining\n");
  time_stop();

  ProgressSetValue( 0 );
  
  return gppd;
  
}

/*******************************************
  Initialize Morphing Process
*******************************************/

void initialize_gmorphv2( HPpd *hppd )
{
  HFace *hfc;
  void  create_hloop( HFace *, HPpd * );
  
  /* create_hloop */
  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {
    
    create_hloop( hfc, hppd );
    
  }
}

/*******************************************
  After Morphing Process
*******************************************/

void after_morphing( Sppd * ppd )
{
  Spfc *fc;
  Spso *so;
  Sppt *pt;
  Spvt *vt;
  Sped *ed;
  Sphe *he;
  Vec  *mvec1, *mvec2;
  Spso *create_ppdsolid(Sppd *);
  Sppt *create_ppdpart(Sppd *);
  void  calc_fnorm(Spfc *);

  /* create solid, part */
/*   so = create_ppdsolid( ppd ); */
/*   pt = create_ppdpart( ppd ); */
/*   pt->bpso = so; */
  
  mvec1 = ppd->mvec1;
  for (vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt) {
/*     vt->bpso = so; */
    vt->vec.x = mvec1[vt->no].x;
    vt->vec.y = mvec1[vt->no].y;
    vt->vec.z = mvec1[vt->no].z;
  }
  
  for (fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt) {
/*     fc->bpso = so; */
/*     fc->bppt = pt; */
    calc_fnorm( fc );
  }

  /* edge_face check */
/*   for ( fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt ) { */
/*     he = fc->sphe; */
/*     display("fc %d\n", fc->no); */
/*     do { */
/*       ++(he->ed->fn); */
/*     } while ( (he = he->nxt) != fc->sphe ); */
/*   } */
  
/*   for ( ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt ) { */
/*     if ( ed->fn > 2 ) { */
/*       display("Edge No.%d has more than three edges. type %d\n", ed->no, ed->sp_type ); */
/*     } */
/*   } */
}

void clear_gmorph( HPpd *hppd )
{
  HFace *hfc;
  HLoop *hl;
  Sppd  *ppd;
  Splp  *lp;
  Splv  *lv;
  Spvt  *vt;
  Sped  *ed;
  void  free_hgppd( HGppd * );
  void  free_hgfc( HGfc * );
  void  free_ppd( Sppd * );
  
  if ( hppd == (HPpd *) NULL ) return;

  /* free hgppd */
  free_hgppd( hppd->hgppd1 ); hppd->hgppd1 = (HGppd *) NULL;
  free_hgppd( hppd->hgppd2 ); hppd->hgppd2 = (HGppd *) NULL;

  for ( hfc = hppd->shfc; hfc != (HFace *) NULL; hfc = hfc->nxt ) {
    
    /* free morphing hgppd face */
    free_hgfc( hfc->mhgfc );

    hfc->mhgfc = (HGfc *) NULL;
    hfc->hgfc1 = (HGfc *) NULL;
    hfc->hgfc2 = (HGfc *) NULL;

    /* delete hloop links */
    hl = hfc->hloop;
    lp = hl->lp1;
    for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
      lv->hgvt = (HGvt *) NULL;
    }
    lp = hl->lp2;
    for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
      lv->hgvt = (HGvt *) NULL;
    }
  }
  
  /* free morphing ppd */
  free_ppd( hppd->gppd );
  
  ppd = hppd->ppd1;
  for ( vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt ) {
    
    vt->hgvt = (HGvt *) NULL;
    vt->mvt  = (Spvt *) NULL;
    vt->group_id = SMDNULL;
    
  }

  for ( ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt ) {
    
    ed->group_id = SMDNULL;
    
  }
  ppd = hppd->ppd2;
  for ( vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt ) {
    
    vt->hgvt = (HGvt *) NULL;
    vt->mvt  = (Spvt *) NULL;
    vt->group_id = SMDNULL;
    
  }

  for ( ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt ) {
    
    ed->group_id = SMDNULL;
    
  }
}

void record_morphppd( char *rec, Sppd * ppd )
{
  int    i, a;
  double p, q;
  int    div;
  char   file[BUFSIZ];
  Spvt   *v;
  Spfc *fc;
  Vec    *mvec1, *mvec2;
  void   write_ppd_file(char *, Sppd *);
  void   calc_fnorm( Spfc * );
  void   ppdnorm( Sppd * );

  if (ppd != (Sppd *) NULL) {
    i = 1;
    div = swin->mdiv - 1;

    mvec1 = ppd->mvec1;
    mvec2 = ppd->mvec2;
    
    for (a = 0; a <= div; ++a) {
      q = (double) a / (double) div;
      p = 1.0 - q;
      for ( v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {
	v->vec.x = p * mvec1[v->no].x + q * mvec2[v->no].x;
	v->vec.y = p * mvec1[v->no].y + q * mvec2[v->no].y;
	v->vec.z = p * mvec1[v->no].z + q * mvec2[v->no].z;
      }
      /* update normal */
      if ( swin->smooth_shading ) {
	for (fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt) {
	  calc_fnorm(fc);
	}
	ppdnorm( ppd );
      }
      sprintf(file, "%s%03d.ppd", rec, i);
      write_ppd_file(file, ppd);
      ++i;
    }
  }
}

void write_gmh_file( char *file, HPpd *hppd )
{
  FILE *fp;
  HVertex *hv;
  HFace   *hf;
  HEdge   *hed;
  Splv    *lv;
  HHalfedge *he;
  char  name[BUFSIZ], str[BUFSIZ];
  int   i;
  int   n_vted1, n_vted2;
  Spvt *vt;
  Sppd *ppd;
  void write_ppd_file( char *, Sppd * );
  void filehead( char *, char * );

  if ( hppd == (HPpd *) NULL ) return;
  
  if ( (hppd->ppd1 == (Sppd *) NULL) || (hppd->ppd2 == (Sppd *) NULL) )
    return;

  if ( (fp = fopen(file, "w")) == NULL ) {
    return;
  }
  
  fprintf(fp, "header\n");
  fprintf(fp, "\tppd\t2\n");
  if ( hppd != (HPpd *) NULL ) {
    fprintf(fp, "\thppd\t1\n");
    if ( hppd->vn ) fprintf(fp, "\thvertex\t%d\n", hppd->vn );
    if ( hppd->fn ) fprintf(fp, "\thface\t%d\n", hppd->fn );
    if ( hppd->en ) fprintf(fp, "\thedge\t%d\n", hppd->en );

    ppd = hppd->ppd1;
    i = 0;
    for ( vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt ) {
      if ( (vt->type == VERTEX_ADDED) && (vt->sp_ed != (Sped *) NULL)
	   && ( vt->sp_type == SP_VERTEX_BOUNDARY ) ) {
	++i;
      }
    }
    n_vted1 = i;
    
    if ( i ) fprintf(fp, "\tvted1\t%d\n", i );
    
    ppd = hppd->ppd2;
    i = 0;
    for ( vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt ) {
      if ( (vt->type == VERTEX_ADDED) && (vt->sp_ed != (Sped *) NULL)
	   && ( vt->sp_type == SP_VERTEX_BOUNDARY ) ) {
	++i;
      }
    }
    n_vted2 = i;
    
    if ( i ) fprintf(fp, "\tvted2\t%d\n", i );
    
  }
  fprintf(fp, "end\n");

  fprintf(fp, "ppd\n");
  filehead( file, str );
  /* ppd1 */
  sprintf( name, "%s_1.ppd", str );
  fprintf( fp, "\t1\t%s\n", name );
  write_ppd_file( name, hppd->ppd1 );
  /* ppd2 */
  sprintf( name, "%s_2.ppd", str );
  fprintf( fp, "\t2\t%s\n", name );
  write_ppd_file( name, hppd->ppd2 );
  fprintf(fp, "end\n");

  if ( hppd == (HPpd *) NULL ) {
    fclose(fp);
    return;
  }

  /* hvertex */
  if ( hppd->vn ) {
    fprintf(fp, "hvertex\n");
    i = 1;
    for (hv = hppd->shvt; hv != (HVertex *) NULL; hv = hv->nxt) {
      fprintf(fp, "\t%d\t%d %d\n", i, hv->vt1->sid, hv->vt2->sid );
      hv->sid = i;
      ++i;
    }
    fprintf(fp, "end\n");
  }

  /* hface */
  if ( hppd->fn ) {
    fprintf(fp, "hface\n");
    i = 1;
    for (hf = hppd->shfc; hf != (HFace *) NULL; hf = hf->nxt) {
	
      he = hf->shhe;
      hf->sid = i;
      fprintf(fp, "\t%d\t", i);
      do {
	fprintf( fp, "%d ", he->vt->sid );
      } while ( (he = he->nxt) != hf->shhe );
      ++i;
      fprintf(fp, "\n");
	
    }
    fprintf(fp, "end\n");
  }

  /* hedge */
  if ( hppd->en ) {
    fprintf(fp, "hedge\n");
    i = 1;
    for (hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt) {
	
      fprintf(fp, "\t%d\tsv %d ev %d rf %d lf %d sp %d\n",
	      i,
	      hed->sv->sid,
	      hed->ev->sid,
	      ( hed->rf != (HFace *) NULL ) ? hed->rf->sid : 0,
	      ( hed->lf != (HFace *) NULL ) ? hed->lf->sid : 0,
	      ( hed->lp1 != (Splp *) NULL ) ? 1 : 0 );
      if ( hed->lp1 != (Splp *) NULL ) {
	fprintf(fp, "\t\tlp1\t");
	for ( lv = hed->lp1->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
	  fprintf(fp, "%d ", lv->vt->sid );
	}
	fprintf(fp, "\n");
	fprintf(fp, "\t\tlp2\t");
	for ( lv = hed->lp2->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
	  fprintf(fp, "%d ", lv->vt->sid );
	}
	fprintf(fp, "\n");
      }
      ++i;
    }
    fprintf(fp, "end\n");
  }

  /* vertex->sp_ed for grouping */
  if ( n_vted1 ) {
    fprintf(fp, "vted1\n");
    i = 1;
    for ( vt = hppd->ppd1->spvt; vt != (Spvt *) NULL; vt = vt->nxt ) {
      if ( (vt->type == VERTEX_ADDED) && (vt->sp_ed != (Sped *) NULL)
	   && ( vt->sp_type == SP_VERTEX_BOUNDARY ) ) {
	fprintf(fp, "\t%d\t%d %d %d %g\n",
		i++, vt->sid,
		vt->sp_ed->sv->sid,
		vt->sp_ed->ev->sid,
		vt->sp_val );
      }
    }
    fprintf(fp, "end\n");
  }

  if ( n_vted2 ) {
    fprintf(fp, "vted2\n");
    i = 1;
    for ( vt = hppd->ppd2->spvt; vt != (Spvt *) NULL; vt = vt->nxt ) {
      if ( (vt->type == VERTEX_ADDED) && (vt->sp_ed != (Sped *) NULL)
	   && ( vt->sp_type == SP_VERTEX_BOUNDARY ) ) {
	fprintf(fp, "\t%d\t%d %d %d %g\n",
		i++, vt->sid,
		vt->sp_ed->sv->sid,
		vt->sp_ed->ev->sid,
		vt->sp_val );
      }
    }
    fprintf(fp, "end\n");
  }

  fclose(fp);
}

/* Resolve relative PPD names from the directory that contains the .gmh file. */
static void gmh_resolve_ppd_path(const char *gmhpath, const char *name,
				 char *out, size_t outsz)
{
  const char *slash;

  if (name == NULL || *name == '\0' || outsz == 0) {
    if (outsz)
      out[0] = '\0';
    return;
  }
  if (name[0] == '/') {
    snprintf(out, outsz, "%s", name);
    return;
  }
  slash = strrchr(gmhpath, '/');
  if (slash == NULL) {
    snprintf(out, outsz, "%s", name);
    return;
  }
  {
    size_t dlen = (size_t)(slash - gmhpath + 1U);
    if (dlen >= outsz)
      dlen = outsz - 1U;
    memcpy(out, gmhpath, dlen);
    out[dlen] = '\0';
    snprintf(out + dlen, outsz - dlen, "%s", name);
  }
}

HPpd *open_gmh_file( char *file )
{
  FILE      *fp;
  int       i, num, mode, id, done, ibuf[BUFSIZ], n;
  Sppd      *ppd1, *ppd2;
  HPpd      *hppd;
  HVertex   *hv, *sv, *ev;
  Spvt      *vt, *psv, *pev;
  Sped      *ed;
  HFace     *hf, *rf, *lf;
  int       rf_flag, lf_flag;
  HEdge     *hed;
  HHalfedge *he;
  char      key[BUFSIZ], val[5][BUFSIZ], poi[5][BUFSIZ], buf[BUFSIZ], *bpt, stc[BUFSIZ];
  char      ppdpath[BUFSIZ];
  Sppd      *open_ppd(char *);
  Spvt      *list_ppdvertex( Sppd *, Id );
  Sped      *find_ppdedge( Spvt *, Spvt * );
  HPpd      *create_hppd( void );
  HVertex   *list_hppdvertex( HPpd *, Id );
  HFace     *list_hppdface( HPpd *, Id );
  HVertex   *create_hppdvertex( HPpd * );
  HEdge     *create_hppdedge( HPpd * );
  HFace     *create_hppdface( HPpd * );
  HHalfedge *create_hppdhalfedge( HFace * );
  Splp      *create_shortestpath( char *, Sppd * );
  HEdge     *find_hppdedge( HVertex *, HVertex *, HPpd * );
  char      *cut(char *, char *);
  void      free_hppd( HPpd * );
  void      calc_hppd_boundary( HPpd * );
  void      make_shortest_path( HPpd * );
  void      calc_hppdfnorm( HFace * );
  int       find_hppdface_edge_links( HFace *, HEdge * );
  void      free_ppdvertex_noedge( Sppd * );
  
  if ((fp = fopen(file, "r")) == NULL) {
    displayinfo("Error: %s No such file or directory.\n", file );
    return (HPpd *) NULL;
  }
  mode = PPD_NONE;

  hppd = create_hppd();
  
  id = 0;
  while ( fgets(buf, BUFSIZ, fp) ) {
    sscanf(buf, "%s", key);
    if (comment(buf[0]))
      continue;
    else if (!strcmp(key, "end")) {
      
      mode = PPD_NONE;
      
    } else if (mode == PPD_HEADER) {	/* read "header" */
      
      sscanf(buf, "%s%s", key, val[0]);
      num = atoi(val[0]);
      
      if ( !strcmp(key, "ppd") ) {
	if (num != 2) {
	  displayinfo("Error: 2 ppd must be specified.\n");
	  fclose(fp);
	  free_hppd( hppd );
	  return (HPpd *) NULL;
	}
      }
      
    } else if (mode == PPD_PPD) {
      
      sscanf(buf, "%s%s", key, val[0]);
      gmh_resolve_ppd_path(file, val[0], ppdpath, sizeof(ppdpath));

      if (id == 0) {
	ppd1 = open_ppd(ppdpath);
	hppd->ppd1 = ppd1;
	if (ppd1 == (Sppd *) NULL) {
	  displayinfo("Error: can't open ppd: %s\n", ppdpath);
	  fprintf(stderr, "GMorph: can't open ppd: %s\n", ppdpath);
	  fclose(fp);
	  free_hppd(hppd);
	  return (HPpd *) NULL;
	}
      } else {
	ppd2 = open_ppd(ppdpath);
	hppd->ppd2 = ppd2;
	if (ppd2 == (Sppd *) NULL) {
	  displayinfo("Error: can't open ppd: %s\n", ppdpath);
	  fprintf(stderr, "GMorph: can't open ppd: %s\n", ppdpath);
	  fclose(fp);
	  free_hppd(hppd);
	  return (HPpd *) NULL;
	}
      }

      ++id;
      
    } else if (mode == PPD_HVERTEX) {
      
      sscanf(buf, "%s%s%s", key, val[0], val[1]);
      hv = create_hppdvertex ( hppd );
      hv->vt1 = list_ppdvertex(ppd1, (Id) atoi(val[0]) - 1);
      if ( hv->vt1 != (Spvt *) NULL ) {
	hv->vt1->sp_type = SP_VERTEX_HVERTEX;
	hv->vt1->hvt     = hv;
      }
      hv->vt2 = list_ppdvertex( ppd2, (Id) atoi(val[1]) - 1);
      if ( hv->vt2 != (Spvt *) NULL ) {
	hv->vt2->sp_type = SP_VERTEX_HVERTEX;
	hv->vt2->hvt     = hv;
      }
      
    } else if (mode == PPD_HFACE) {
      
      bpt = buf;
      bpt = cut(bpt, stc);

      hf = create_hppdface( hppd );
      
      done = SMD_OFF;
      n = 0;
      while (!done) {
	bpt = cut(bpt, stc);
	if (!bpt) {
	  if (strcmp(stc, "\\")) {
	    ibuf[n++] = atoi(stc)-1;
	  }
	  done = SMD_ON;
	} else {
	  ibuf[n++] = atoi(stc)-1;
	}
      }
      
      for (i = 0; i < n; ++i) {
      	he = create_hppdhalfedge( hf );
	he->vt = list_hppdvertex( hppd, (Id) ibuf[i]);
      }

      /* make HEdges */
/*       he = hf->shhe; */
/*       do { */
	
/* 	sv = he->vt; */
/* 	ev = he->nxt->vt; */
/* 	if ( (hed = find_hppdedge( sv, ev, hppd )) == (HEdge *) NULL ) { */
	  
/* 	  hed = create_hppdedge( hppd ); */
/* 	  hed->sv = sv; */
/* 	  hed->ev = ev; */
	  
/* 	} */
/* 	he->hed = hed; */
    
/* 	if ( hed->rf == (HFace *) NULL ) hed->rf = hf; */
/* 	else hed->lf = hf; */
	
/*       } while ( (he = he->nxt) != hf->shhe ); */
      
      calc_hppdfnorm( hf );

    } else if ( mode == PPD_HEDGE ) {
      
      sscanf(buf, "%s%s%s%s%s%s%s%s%s%s%s", key,
	     poi[0], val[0],
	     poi[1], val[1],
	     poi[2], val[2],
	     poi[3], val[3],
	     poi[4], val[4] );
      
      hed = create_hppdedge( hppd );

      hed->sv = list_hppdvertex( hppd, (Id) atoi(val[0]) - 1 );
      hed->ev = list_hppdvertex( hppd, (Id) atoi(val[1]) - 1 );

      rf = list_hppdface( hppd, (Id) atoi(val[2]) - 1 );
      rf_flag = SMDNULL;
      if ( rf != (HFace *) NULL ) {
	rf_flag = find_hppdface_edge_links( rf, hed );
      }
      lf = list_hppdface( hppd, (Id) atoi(val[3]) - 1 );
      lf_flag = SMDNULL;
      if ( lf != (HFace *) NULL ) {
	lf_flag = find_hppdface_edge_links( lf, hed );
      }
      if ( rf_flag != SMDNULL ) {

	if ( rf_flag == SMD_ON ) {
	  hed->rf = rf;
	  hed->lf = lf;
	} else {
	  hed->rf = lf;
	  hed->lf = rf;
	}

      } else {

	if ( lf_flag != SMDNULL ) {

	  if ( lf_flag == SMD_ON ) {
	    hed->rf = lf;
	    hed->lf = rf;
	  } else {
	    hed->rf = rf;
	    hed->lf = lf;
	  }
	}
      }
	    
	  
      /* shortest-path */
      if ( atoi(val[4]) ) {
	fgets( buf, BUFSIZ, fp );
	hed->lp1 = create_shortestpath( (char *) buf, hppd->ppd1 );
	hed->lp1->hed = hed;
	fgets( buf, BUFSIZ, fp );
	hed->lp2 = create_shortestpath( (char *) buf, hppd->ppd2 );
	hed->lp2->hed = hed;
      }
      
    } else if ( mode == PPD_HVERTEX ) {
      
      sscanf( buf, "%s%s%s", key, val[0], val[1] );
      hv = create_hppdvertex( hppd );
      hv->vt1 = list_ppdvertex( ppd1, (Id) atoi(val[0]) - 1);
      if ( hv->vt1 != (Spvt *) NULL ) {
	hv->vt1->sp_type = SP_VERTEX_HVERTEX;
      }
      hv->vt2 = list_ppdvertex( ppd2, (Id) atoi(val[1]) - 1);
      if ( hv->vt2 != (Spvt *) NULL ) {
	hv->vt2->sp_type = SP_VERTEX_HVERTEX;
      }
      
    } else if ( mode == PPD_VTED1 ) {
      
      sscanf( buf, "%s%s%s%s%s", key, val[0], val[1], val[2], val[3] );

      vt  = list_ppdvertex( ppd1, atoi(val[0])-1 );
      psv = list_ppdvertex( ppd1, atoi(val[1])-1 );
      pev = list_ppdvertex( ppd1, atoi(val[2])-1 );
      ed  = find_ppdedge( psv, pev );
      if ( ed != (Sped *) NULL ) {
	vt->sp_ed  = ed;
	if ( ed->sv == psv ) {
	  vt->sp_val = atof( val[3] );
	} else {
	  vt->sp_val = 1.0 - atof( val[3] );
	}
      }
      
    } else if ( mode == PPD_VTED2 ) {
      
      sscanf(buf, "%s%s%s%s%s", key, val[0], val[1], val[2], val[3]);

      vt  = list_ppdvertex( ppd2, atoi(val[0])-1 );
      psv = list_ppdvertex( ppd2, atoi(val[1])-1 );
      pev = list_ppdvertex( ppd2, atoi(val[2])-1 );
      ed  = find_ppdedge( psv, pev );
      if ( ed != (Sped *) NULL ) {
	vt->sp_ed  = ed;
	if ( ed->sv == psv ) {
	  vt->sp_val = atof( val[3] );
	} else {
	  vt->sp_val = 1.0 - atof( val[3] );
	}
      }
/*       display("vt %d sp_ed %d sp_val %g\n", vt->no, vt->sp_ed->no, vt->sp_val ); */
      
    } else if (mode == PPD_NONE) {
      
      if (!strcmp(key, "header")) {
	mode = PPD_HEADER;
      } else if ( !strcmp(key, "ppd") ) {
	mode = PPD_PPD;
	id = 0;
      } else if ( !strcmp(key, "hface") ) {
	mode = PPD_HFACE;
	id = 0;
      } else if ( !strcmp(key, "hedge") ) {
	mode = PPD_HEDGE;
	id = 0;
      } else if ( !strcmp(key, "hvertex") ) {
	mode = PPD_HVERTEX;
	id = 0;
      } else if ( !strcmp(key, "vted1") ) {
	mode = PPD_VTED1;
	id = 0;
      } else if ( !strcmp(key, "vted2") ) {
	mode = PPD_VTED2;
	id = 0;
      }
      
    }
  }
  fclose(fp);

  free_ppdvertex_noedge( hppd->ppd1 );
  free_ppdvertex_noedge( hppd->ppd2 );

  return hppd;
}

Splp *create_shortestpath( char *buf, Sppd *ppd )
{
  char *bpt, stc[BUFSIZ];
  Splp *lp;
  Splv *lv, *plv;
  Sped *ed;
  Spvt *vt;
  Splp *create_splp( void );
  Splv *create_ppdloopvertex( Splp * );
  Spvt *list_ppdvertex( Sppd *, Id );
  Sped *find_ppdedge( Spvt *, Spvt * );
  Sped *create_ppdedge( Sppd * );
  char *cut( char *, char * );
  Vted *create_vtxed( Spvt *, Sped * );
  
  bpt = buf;
  bpt = cut( bpt, stc );

  lp = create_splp();
  lp->type  = SHORTESTPATH;

  while ( bpt ) {
    
    bpt = cut( bpt, stc );

    vt = list_ppdvertex( ppd, atoi(stc)-1 );
    lv = create_ppdloopvertex( lp );
    lv->vt = vt; ++( vt->spn );

  }

  plv = lv = lp->splv;
  lv  = lv->nxt;

  while ( lv != (Splv *) NULL ) {

    if ( (lv != lp->splv) && (lv != lp->eplv ) ) {
      lv->vt->sp_type = SP_VERTEX_BOUNDARY;
      /* for grouping */
      lv->vt->lp = lp;
    }
    
    if ( (ed = find_ppdedge( plv->vt, lv->vt )) == (Sped *) NULL ) {
	    
      /* create additional edges */
      ed = create_ppdedge( ppd );
      ed->sp_type = SP_EDGE_BOUNDARY;
      ed->type = EDGE_ADDED;
      ed->sv = plv->vt;
      ed->ev = lv->vt;
      (void) create_vtxed( ed->sv, ed );
      (void) create_vtxed( ed->ev, ed );

    }
    ed->sp_type = SP_EDGE_BOUNDARY;

    if ( lv == lp->eplv ) break;
    plv = lv;
    lv = lv->nxt;

  }

  return lp;
}


int gmh_ok( char *str )
{
  Sppd  *ppd;
  void  write_gmh_file( char *, HPpd * );
  HPpd  *open_gmh_file( char * );
  void  free_hppd( HPpd * );
  void  set_windowtitle(char *);
  
  if (swin->opend == SMDSAVE) {
    
    write_gmh_file( str, swin->hppd );
    set_windowtitle( str );
    
  } else {
    
    free_hppd( swin->hppd );
    swin->hppd = open_gmh_file( str );
    
    if ( swin->hppd == (HPpd *) NULL ) {
      return FAIL;
    }

    if ( swin->hppd->ppd1 != (Sppd *) NULL ) {
      swin->screenatr[0].current_ppd = swin->hppd->ppd1;
      swin->screenatr[0].view_ppd    = swin->hppd->ppd1;
    }
    if ( swin->hppd->ppd2 != (Sppd *) NULL ) {
      swin->screenatr[1].current_ppd = swin->hppd->ppd2;
      swin->screenatr[1].view_ppd    = swin->hppd->ppd2;
    }
    set_windowtitle( str );
    
  }
  
  return SUCCEED;
}

void makegmorphcb( Widget w, XtPointer cld, XtPointer *cad )
{
  Sppd *ppdgmorph_v2( HPpd * );
  void clear_gmorph( HPpd * );
  void DisplayBusyCursor ( Widget w );

  if ( swin->hppd == (HPpd *) NULL ) {
    displayinfo("can't execute gmorph.\n");
    return;
  }
  
  if ( swin->morph_ppd != (Sppd *) NULL ) {
    clear_gmorph( swin->hppd );
    swin->morph_ppd = (Sppd *) NULL;
  }

  DisplayBusyCursor ( w );
  
  if ( (swin->morph_ppd = ppdgmorph_v2( swin->hppd )) == (Sppd *) NULL ) {
    displayinfo("can't execute gmorph.\n");
    return;
  }

/*  swin->screenatr[0].current_ppd = swin->morph_ppd;
  swin->screenatr[0].view_ppd = swin->morph_ppd;
  swin->screenatr[1].current_ppd = (Sppd *) NULL;
  swin->screenatr[1].view_ppd = (Sppd *) NULL;
*/
  displayinfo("gmorph succeedingly ended.\n");
  
}




