/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include <string.h>
#include "motif.h"
#include "gldef.h"
#include "smd.h"

void find_fileheader(char *filename, char *tname)
{
  int   i, j, a;
  char  name[BUFSIZ];

  for (i = strlen(filename) - 1; filename[i] != '/' && i; --i);

  if ( !i ) a = 0;
  else a = i+1;
  strcpy(tname, "\0");
  strcpy(name, "\0");
  for (j = a; j < strlen(filename); ++j) {
    sprintf(name, "%s%c", tname, filename[j]);
    strcpy(tname, name);
  }
}

void filehead(char *filename, char *tname)
{
  int   i, j, a;
  char  name[BUFSIZ];

  for (i = strlen(filename) - 1; filename[i] != '.' && i; --i);

  strcpy(tname, "\0");
  strcpy(name, "\0");
  for (j = 0; j < i; ++j) {
    sprintf(name, "%s%c", tname, filename[j]);
    strcpy(tname, name);
  }
}

char *open_filename(char *str, char *name, char *dname)
{
  int   tidx;
  struct stat statbuf;
  char  com[BUFSIZ];

  if (stat(str, &statbuf) == -1) {
    if (str) {
      for (tidx = strlen(str) - 1;
	   str[tidx] != '/' &&
	   tidx; tidx--) {
      };

      sprintf(name, "%s", &str[tidx + 1]);
      strcpy(dname, str);
      return name;
    }
  }
  if (statbuf.st_mode & S_IFDIR) {
    sprintf(com, "dir %s\n", str);
  } else {
    for (tidx = strlen(str) - 1;
	 str[tidx] != '/' &&
	 tidx; tidx--) {
    };

    strcpy(dname, str);
    strcpy(name, &(str[tidx + 1]));
  }
  return name;
}

char *cut(char *sstr, char *res)
{
  int   search;
  char *str;

  str = sstr;
  search = 0;
  while (SMD_ON) {
    if (search == 0) {		/* before found */
      if (*str == ' ' || *str == '\t') {
	++str;
      } else if (*str == '\0' || *str == '\n') {
	str = NULL;
	break;
      } else {
	*res++ = *str++;
	search = 1;
      }
    } else if (search == 1) {	/* now searching */
      if (*str == ' ' || *str == '\t') {
	++str;
	search = 2;
      } else if (*str == '\0' || *str == '\n') {
	str = NULL;
	break;
      } else {
	*res++ = *str++;
      }
    } else {			/* after found */
      if (*str == ' ' || *str == '\t') {
	++str;
      } else if (*str == '\0' || *str == '\n') {
	str = NULL;
	break;
      } else {
	break;
      }
    }
  }
  *res = '\0';
  return str;
}

void smddebug(char *fmt,...)
{
  va_list argp;

  if (swin->prn) {
    va_start(argp, fmt);
    vfprintf(stdout, fmt, argp);
    va_end(argp);
    fflush(stdout);
    fflush(stdout);
  }
}


void display(char *fmt,...)
{
  va_list argp;

  if (swin->prn) {
    va_start(argp, fmt);
    vfprintf(stdout, fmt, argp);
    va_end(argp);
    fflush(stdout);
    fflush(stdout);
  }
}


void dummy(void)
{
}

void gmorph_file_dialog_ok(const char *path)
{
  char  full[BUFSIZ];
  int   ppd_ok(char *, int);
  int   gmh_ok(char *);
  int   gppd_ok( char * );
  void  drawwindow(int);
  void  calc_fnorm(Spfc *);
  void  ppdnorm(Sppd *);
  int   redraw;
  Spvt *v;
  Spfc *fc;
  Sppd *ppd;
  Vec  *mvec1;

  if (path == NULL)
    return;
  strncpy(full, path, BUFSIZ - 1);
  full[BUFSIZ - 1] = '\0';

  redraw = (swin->opend == SMDOPEN);
  if (swin->filed == SMDFILEPPD1)
    ppd_ok(full, 0);
  else if (swin->filed == SMDFILEPPD2)
    ppd_ok(full, 1);
  else if (swin->filed == SMDFILEGPPD)
    if ( gppd_ok(full) == SMD_ON ) {
      /* Show only left window immediately. */
      swin->screenatr[0].view_ppd = swin->morph_ppd;
      swin->screenatr[1].view_ppd = (Sppd *) NULL;

      ppd = swin->screenatr[0].view_ppd;
      if ( ppd != (Sppd *) NULL ) {
	/* Mimic gmorphcb() initial state (a = 0 -> p=1, q=0). */
	mvec1 = ppd->mvec1;
	for ( v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {
	  v->vec.x = mvec1[v->no].x;
	  v->vec.y = mvec1[v->no].y;
	  v->vec.z = mvec1[v->no].z;
	}
	/* update face normals */
	for ( fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt ) {
	  calc_fnorm(fc);
	}
	if ( swin->smooth_shading ) {
	  ppdnorm(ppd);
	}
      }
    } else {
      redraw = 0;
    }
  else if (swin->filed == SMDFILEGMH) {
    if (gmh_ok(full) != SUCCEED)
      redraw = 0;
  }

  if (redraw) {
    drawwindow(SCREEN1);
    drawwindow(SCREEN2);
  }
}

