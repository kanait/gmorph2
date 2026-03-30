/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
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

/***********************  file  ***************************/

static char *(filetxt1[]) = {
  "Ppd  File Select",
  "Ppd  File Select",
  "Gmh  File Select"
};

static char *(filetxt2[]) = {
  "*.ppd",
  "*.ppd",
  "*.gmh"
};

static Widget filesel;

/* file selection box open (for reading file) */

void file_open_initcb( Widget w, XtPointer cld, XtPointer *cad )
{
  int   i, n;
  Arg   args[5];
  XmString  text1, text2;

  i = swin->filed = (int) cld;
  swin->opend = SMDOPEN;
  text1 = (XmString) XmStringCreate(filetxt1[i], XmSTRING_DEFAULT_CHARSET);
  text2 = (XmString) XmStringCreate(filetxt2[i], XmSTRING_DEFAULT_CHARSET);
  n = 0;
  XtSetArg(args[n], XmNfilterLabelString, text1);
  n++;
  XtSetArg(args[n], XmNdirMask, text2);
  n++;
  XtSetValues(filesel, args, 2);
  XtManageChild(filesel);
  XmStringFree(text1);
  XmStringFree(text2);
}

void file_save_initcb(Widget w, XtPointer cld, XtPointer *cad)
{
  int   i, n;
  Arg   args[5];
  XmString  text1, text2;

  i = swin->filed = (int) cld;
  swin->opend = SMDSAVE;
  text1 = (XmString) XmStringCreate(filetxt1[i], XmSTRING_DEFAULT_CHARSET);
  text2 = (XmString) XmStringCreate(filetxt2[i], XmSTRING_DEFAULT_CHARSET);
  n = 0;
  XtSetArg(args[n], XmNfilterLabelString, text1);
  n++;
  XtSetArg(args[n], XmNdirMask, text2);
  n++;
  XtSetValues(filesel, args, 2);
  XtManageChild(filesel);
  XmStringFree(text1);
  XmStringFree(text2);
}

void fileokcb(Widget w, XtPointer cld, XtPointer *cad)
{
  Widget   TextW;
  char  *str, tstr[BUFSIZ];
  int   ppd_ok(char *, int);
  int   gmh_ok(char *);
  int   gppd_ok( char * );
  void  find_fileheader( char *, char * );
  void  drawwindow(int);
  void  calc_fnorm(Spfc *);
  void  ppdnorm(Sppd *);
  int   redraw;
  Spvt *v;
  Spfc *fc;
  Sppd *ppd;
  Vec  *mvec1;

  TextW = (Widget) XmFileSelectionBoxGetChild(w, XmDIALOG_TEXT);
  str = (char *) XmTextGetString(TextW);
  XtUnmanageChild( filesel );
  find_fileheader( str, (char *) tstr );

  redraw = (swin->opend == SMDOPEN);
  if (swin->filed == SMDFILEPPD1)
    ppd_ok(tstr, 0);
  else if (swin->filed == SMDFILEPPD2)
    ppd_ok(tstr, 1);
  else if (swin->filed == SMDFILEGPPD)
    if ( gppd_ok(tstr) == SMD_ON ) {
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
    if (gmh_ok(tstr) != SUCCEED)
      redraw = 0;
  }

  /* ダイアログを閉じても GLw に Expose が来ないことがあり、読み込み直後に描画されない */
  if (redraw) {
    drawwindow(SCREEN1);
    drawwindow(SCREEN2);
  }

  XtFree(str);
}

void filecnclcb(Widget w, XtPointer cld, XtPointer *cad)
{
  XtUnmanageChild(filesel);
}

void createfileserver(Widget parent)
{
  int   n;
  Arg   args[20];
  
  n = 0;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ++n;
  filesel = (Widget) XmCreateFileSelectionDialog(parent, "winfilesel", args, n);
  XtAddCallback(filesel, XmNokCallback, (XtCallbackProc) fileokcb, NULL);
  XtAddCallback(filesel, XmNcancelCallback, (XtCallbackProc) filecnclcb, NULL);
}


