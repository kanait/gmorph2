/* Copyright (c) 1997-1998 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "window.h"
#include "smd.h"

#include "xpm.h"
#include "icon.h"

#include "Progress.h"

#include <unistd.h>

#define  RSIZE   150
/* Morph animation frame delay (microseconds).
 * GUI "Start" button currently has no delay, so frames can be too fast
 * to see. */
#define MORPH_FRAME_USEC 20000

Widget progress = NULL;

/* Legacy glXChooseVisual fallback. */
static void choose_gl_visual_legacy(Display *dpy, int screen, ScreenAtr *sa)
{
  static int a0[] = {
    GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DEPTH_SIZE, 24,
    GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8,
    None
  };
  static int a1[] = {
    GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DEPTH_SIZE, 24,
    GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8,
    GLX_STENCIL_SIZE, 8,
    None
  };
  static int a2[] = {
    GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DEPTH_SIZE, 24,
    None
  };
  static int a3[] = {
    GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DEPTH_SIZE, 16,
    GLX_STENCIL_SIZE, 4,
    GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
    None
  };
  static int a4[] = {
    GLX_RGBA, GLX_DEPTH_SIZE, 24,
    GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8,
    None
  };
  int *lists[] = { a0, a1, a2, a3, a4 };
  size_t k;

  sa->vi = NULL;
  for (k = 0; k < sizeof(lists) / sizeof(lists[0]); k++) {
    sa->vi = glXChooseVisual(dpy, screen, lists[k]);
    if (sa->vi)
      return;
  }
}

/*
 * On some drivers such as NVIDIA, glXChooseVisual + glXCreateContext can fail
 * with X_GLXCreateContext / BadValue. Prefer GLX 1.3 FBConfig +
 * glXCreateNewContext when available.
 */
static void setup_gl_for_screen(Display *dpy, int screen, ScreenAtr *sa)
{
  int n;
  GLXFBConfig *fbc;
  static const int fb0[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_DOUBLEBUFFER, True,
    GLX_DEPTH_SIZE, 24,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    None
  };
  static const int fb1[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_DOUBLEBUFFER, True,
    GLX_DEPTH_SIZE, 24,
    None
  };
  static const int fb2[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_DEPTH_SIZE, 24,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    None
  };

  sa->glx_fbc = NULL;
  sa->vi = NULL;

  fbc = glXChooseFBConfig(dpy, screen, fb0, &n);
  if (!fbc || n < 1)
    fbc = glXChooseFBConfig(dpy, screen, fb1, &n);
  if (!fbc || n < 1)
    fbc = glXChooseFBConfig(dpy, screen, fb2, &n);

  if (fbc && n >= 1) {
    sa->vi = glXGetVisualFromFBConfig(dpy, fbc[0]);
    if (sa->vi) {
      sa->glx_fbc = (void *) fbc[0];
      XFree(fbc);
      return;
    }
    XFree(fbc);
  }

  choose_gl_visual_legacy(dpy, screen, sa);
}

/* "File" menu */

extern void file_open_initcb(Widget, XtPointer, XtPointer *);
extern void file_save_initcb(Widget, XtPointer, XtPointer *);
extern void quit_initcb(Widget, XtPointer, XtPointer *);
/* extern void edit_bgcolorcb(Widget, XtPointer *, XtPointer *); */

static Mb FileOpenButtons[] = {
  {NULL, "GMH", SMDFILEGMH, "smdpb", file_open_initcb, "Ctrl <Key> g", "<Ctrl> G"},
  {NULL, "PPD1", SMDFILEPPD1, "smdpb", file_open_initcb, "Ctrl <Key> f", "<Ctrl> F"},
  {NULL, "PPD2", SMDFILEPPD2, "smdpb", file_open_initcb, "Ctrl <Key> a", "<Ctrl> A"},
  {NULL, "GPPD", SMDFILEGPPD, "smdpb", file_open_initcb, "", ""}
};

static Mb FileSaveButtons[] = {
  {NULL, "GMH", SMDFILEGMH, "smdpb", file_save_initcb, "Ctrl <Key> m", "<Ctrl> M"},
  {NULL, "GPPD", SMDFILEGPPD, "smdpb", file_save_initcb, "Ctrl <Key> s", "<Ctrl> S"},
  {NULL, "PPD1", SMDFILEPPD1, "smdpb", file_save_initcb, "", ""},
  {NULL, "PPD2", SMDFILEPPD2, "smdpb", file_save_initcb, "", ""}
};

static Mb FileQuitButtons[] = {
  {NULL, "Quit", SMDNULL, "smdpb", quit_initcb, "Ctrl <Key> q", "<Ctrl> Q"}
};

static Pmcb FileButtons[] = {
  {NULL, NULL, "Open...", "", "", FileOpenButtons, 4, 'O'},
  {NULL, NULL, "Save...", "", "", FileSaveButtons, 4, 'S'},
  {NULL, NULL, "Exit...", "", "", FileQuitButtons, 1, 'E'}
};

static Mb EditColorButtons[] = {
/*   {NULL, "Background", SMDNULL, "smdpb", edit_bgcolorcb, "", ""} */
  {NULL, "Background", SMDNULL, "smdpb", NULL, "", ""}
};

static Pmcb EditButtons[] = {
  {NULL, NULL, "Color...", "", "", EditColorButtons, 1, 'C'}
};

/* top menu */
#define TOPMENU 2

static Pmcb topMenuMB[] = {
  {NULL, NULL, "File", "", "", NULL, 0, 'F'},
  {NULL, NULL, "Edit", "", "", NULL, 0, 'E'}
};
/***********************  menu  ***************************/

static void createmenubutton(Widget parent, Mb *Buttons, int nbuttons)
{
  Arg   args[5];
  int   wc;
  Cardinal nargs;
  Mb    *tb;

  for (wc = 0; wc < nbuttons; wc++) {
    tb = &(Buttons[wc]);
    XtSetArg(args[0], XmNlabelString, xmstrmk(tb->Label));
    nargs = 1;
    if (*(tb->accel) != '\0') {
      XtSetArg(args[1], XmNaccelerator, tb->accel);
      XtSetArg(args[2], XmNacceleratorText, xmstrmk(tb->acceltext));
      nargs = 3;
    }
    Buttons[wc].Button = (Widget) XmCreatePushButtonGadget(parent, tb->RN, args, nargs);
    XtManageChild(tb->Button);
    XtAddCallback(tb->Button, XmNactivateCallback, tb->CB, (XtPointer) tb->arg);
  }
}

static void createpulldownmenu(Widget parent, Pmcb *MenuPane, int Npanes)
{
  Arg   args[5];
  register int wc;
  Pmcb *tp;

  for (wc = 0; wc < Npanes; wc++) {
    tp = &(MenuPane[wc]);
    tp->PM = (Widget) XmCreatePulldownMenu(parent, tp->RNPM, NULL, 0);
    XtSetArg(args[0], XmNlabelString, xmstrmk(tp->Label));
    XtSetArg(args[1], XmNsubMenuId, tp->PM);
    XtSetArg(args[2], XmNmnemonic, tp->mnemonic);
    XtSetArg(args[3], XmNshadowThickness, 2);
    tp->CB = (Widget) XmCreateCascadeButton(parent, tp->RNCB, args, 4);
    XtManageChild(tp->CB);
    createmenubutton(tp->PM, tp->Buttons, tp->Nbuttons);
  }
}

Widget createmenupane(Widget parent)
{
  int   n;
  Arg   args[15];
  Widget topmenubar;

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNshadowThickness, 2);  n++;
  topmenubar = XmCreateMenuBar(parent, "winlabel", args, n);
  XtManageChild(topmenubar);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);
  n++;
  XtSetArg(args[n], XmNtopWidget, topmenubar);
  n++;
  createpulldownmenu(topmenubar, topMenuMB, 2);
  createpulldownmenu(topMenuMB[0].PM, FileButtons, 3);
  createpulldownmenu(topMenuMB[1].PM, EditButtons, 1);

  return topmenubar;
}

/***********************  quit  ***************************/

static Widget exitwarning;

void quitcb(Widget w, Widget wd, XmAnyCallbackStruct *msg_data)
{
  void free_swin( Swin * );

  switch (msg_data->reason) {
  case XmCR_OK:
    
    free_swin( swin );
    
    exit(1);
  case XmCR_CANCEL:
    XtUnmanageChild(exitwarning);
    break;
  }
}

void createquit(Widget parent)
{
  int      n;
  Arg      args[5];
  XmString exittxt;
  void     quitcb(Widget, Widget, XmAnyCallbackStruct *);

  exittxt = (XmString) XmStringCreate("Quit GMorph?", XmSTRING_DEFAULT_CHARSET);
  n = 0;
  XtSetArg(args[n], XmNmessageString, exittxt); n++;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ++n;
  exitwarning = (Widget) XmCreateQuestionDialog(parent, "exitQD", args, n);

  XtAddCallback(exitwarning, XmNcancelCallback, (XtCallbackProc) quitcb, exitwarning);
  XtAddCallback(exitwarning, XmNokCallback, (XtCallbackProc) quitcb, exitwarning);
  XtUnmanageChild((Widget) XmMessageBoxGetChild(exitwarning, XmDIALOG_HELP_BUTTON));
  XmStringFree(exittxt);
}

void quit_initcb(Widget w, XtPointer cld, XtPointer *cad)
{
  XtManageChild(exitwarning);
}

/******************** 3D window panel **********************/

static void initwindow3dcb(Widget w, XtPointer cld, GLwDrawingAreaCallbackStruct *cad)
{
  int   id;
  ScreenAtr *sa;
  void  init_gl3d(ScreenAtr *);
  
  id = (int) cld;
  sa = &(swin->screenatr[id]);
  
  init_gl3d(sa);
}

static void drawwindow3dcb(Widget w, XtPointer cld, GLwDrawingAreaCallbackStruct *cad)
{
  void  drawwindow(int);

  drawwindow((int) cld);
}

static void resizewindow3dcb(Widget w, XtPointer cld, GLwDrawingAreaCallbackStruct *cad)
{
  int   i;
  ScreenAtr *screen;
  void  drawwindow(int);

  i = (int) cld;
  screen = &(swin->screenatr[i]);
  screen->width   = (int) cad->width;
  screen->height  = (int) cad->height;
  screen->xcenter = (double) screen->width  / 2.0;
  screen->ycenter = (double) screen->height / 2.0;
  screen->resize  = SMD_ON;
  
  drawwindow(i);
}

/* Drawing Area */
static void create3dwinpane(Widget parent, Widget tm, Widget wchrc, Widget pinfo)
{
  int    i, n;
  ScreenAtr *sa;
  Arg    args[20];
  Widget darea;
  void   inputwindow3dcb(Widget, XtPointer, GLwDrawingAreaCallbackStruct *);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNwidth, swin->screenatr[0].width * 2);  n++;
  XtSetArg(args[n], XmNheight, swin->screenatr[0].height);  n++;
  XtSetArg(args[n], XmNtopWidget, tm);  n++;
  XtSetArg(args[n], XmNrightWidget, wchrc);  n++;
  XtSetArg(args[n], XmNbottomWidget, pinfo);  n++;
  XtSetArg(args[n], XmNleftOffset, 5);  n++;
  XtSetArg(args[n], XmNrightOffset, 5);  n++;
  XtSetArg(args[n], XmNbottomOffset, 5);  n++;
  darea = XmCreateForm(parent, "darea", args, n);
  XtManageChild(darea);

  for (i = 0; i < 2; ++i) {
    n = 0;
    sa = &(swin->screenatr[i]);
    if (i == 0) {
      /* left window */
      XtSetArg(args[n], XmNx, 0);  n++;
      XtSetArg(args[n], XmNy, 0);  n++;
      XtSetArg(args[n], XmNwidth, swin->screenatr[SCREEN1].width);  n++;
      XtSetArg(args[n], XmNheight, swin->screenatr[SCREEN1].height);  n++;
      XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
      XtSetArg(args[n], XmNrightAttachment, XmATTACH_SELF);  n++;  
      XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
      XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
    } else {
      /* right window */
      XtSetArg(args[n], XmNx, swin->screenatr[SCREEN1].width + 3);  n++;
      XtSetArg(args[n], XmNy, 0);  n++;
      XtSetArg(args[n], XmNwidth, swin->screenatr[SCREEN2].width);  n++;
      XtSetArg(args[n], XmNheight, swin->screenatr[SCREEN2].height);  n++;
      XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
      XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
      XtSetArg(args[n], XmNleftAttachment, XmATTACH_SELF);  n++; 
      XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
    }

    sa->fr3d = XmCreateFrame( darea, "frame", args, n );
    XtManageChild(sa->fr3d);

    n = 0;
    setup_gl_for_screen(XtDisplay(parent),
			XScreenNumberOfScreen(XtScreen(parent)),
			sa);
    if (!sa->vi) {
      fprintf(stderr,
	      "gmorph2b8: no suitable GLX visual (FBConfig + legacy).\n");
      exit(1);
    }
    sa->xc = NULL;
    XtSetArg(args[n], GLwNvisualInfo, sa->vi);  n++;
/*     sa->glw = XtCreateManagedWidget("glw3d",  glwMDrawingAreaWidgetClass, */
/* 				    sa->fr3d, args, n); */
    sa->glw = GLwCreateMDrawingArea(sa->fr3d, "glw3d",  args, n);
    XtAddCallback(sa->glw, GLwNginitCallback,
		  (XtCallbackProc) initwindow3dcb, (XtPointer) i);
    XtAddCallback(sa->glw, GLwNexposeCallback,
		  (XtCallbackProc) drawwindow3dcb, (XtPointer) i);
    XtAddCallback(sa->glw, GLwNresizeCallback,
		(XtCallbackProc) resizewindow3dcb, (XtPointer) i);
    XtAddCallback(sa->glw, GLwNinputCallback,
		  (XtCallbackProc) inputwindow3dcb, (XtPointer) i);
    /* GLwCreateMDrawingArea is unmanaged until explicitly managed and realized. */
    XtManageChild(sa->glw);
  }
}

/******************* 3D window display buttons *************************/

#define DISP_WIRE   0
#define DISP_SHADE  1
#define DISP_CPOINT 2
#define DISP_CMESH  3
#define DISP_LOOP   4
#define DISP_GROUP  5
#define DISP_SPATH  6
#define DISP_HMAP   7
#define DISP_COAXIS 8

static void wdis3dcb(Widget w, XtPointer cld, XmToggleButtonCallbackStruct *cad)
{
  void  drawwindow(int);

  switch (cad->set) {
  case TRUE:
    
   switch ( (int) cld ) {
   case DISP_WIRE:
     swin->dis3d.wire = SMD_ON;
     break;
   case DISP_SHADE:
     swin->dis3d.shading = SMD_ON;
     break;
   case DISP_CPOINT:
     swin->dis3d.cpoint = SMD_ON;
     break;
   case DISP_CMESH:
     swin->dis3d.cmesh = SMD_ON;
     break;
   case DISP_LOOP:
     swin->dis3d.loop = SMD_ON;
     break;
   case DISP_GROUP:
     swin->dis3d.group = SMD_ON;
     break;
   case DISP_SPATH:
     swin->dis3d.spath = SMD_ON;
     break;
   case DISP_HMAP:
     swin->dis3d.hmap = SMD_ON;
     break;
   case DISP_COAXIS:
     swin->dis3d.coaxis = SMD_ON;
     break;
   }
   break;
   
 case FALSE:

   switch ( (int) cld ) {
   case DISP_WIRE:
     swin->dis3d.wire = SMD_OFF;
     break;
   case DISP_SHADE:
     swin->dis3d.shading = SMD_OFF;
     break;
   case DISP_CPOINT:
     swin->dis3d.cpoint = SMD_OFF;
     break;
   case DISP_CMESH:
     swin->dis3d.cmesh = SMD_OFF;
     break;
   case DISP_LOOP:
     swin->dis3d.loop = SMD_OFF;
     break;
   case DISP_GROUP:
     swin->dis3d.group = SMD_OFF;
     break;
   case DISP_SPATH:
     swin->dis3d.spath = SMD_OFF;
     break;
   case DISP_HMAP:
     swin->dis3d.hmap = SMD_OFF;
     break;
   case DISP_COAXIS:
     swin->dis3d.coaxis = SMD_OFF;
     break;
   }
   break;
   
  }
  
  drawwindow( SCREEN1 );
  drawwindow( SCREEN2 );
  
}

static int disp_kind_initial_on(int kind)
{
  switch (kind) {
  case DISP_WIRE:   return swin->dis3d.wire    == SMD_ON;
  case DISP_SHADE:  return swin->dis3d.shading == SMD_ON;
  case DISP_CPOINT: return swin->dis3d.cpoint  == SMD_ON;
  case DISP_CMESH:  return swin->dis3d.cmesh   == SMD_ON;
  case DISP_LOOP:   return swin->dis3d.loop    == SMD_ON;
  case DISP_GROUP:  return swin->dis3d.group   == SMD_ON;
  case DISP_SPATH:  return swin->dis3d.spath   == SMD_ON;
  case DISP_HMAP:   return swin->dis3d.hmap    == SMD_ON;
  case DISP_COAXIS: return swin->dis3d.coaxis  == SMD_ON;
  default:          return 0;
  }
}

static void wdis3denhcb( Widget w, XtPointer cld, XmToggleButtonCallbackStruct *cad )
{
  void  drawwindow(int);

  switch (cad->set) {
  case TRUE:
    swin->ext_display = SMD_ON;
    break;
  case FALSE:
    swin->ext_display = SMD_OFF;
  }
    
  drawwindow( SCREEN1 );
  drawwindow( SCREEN2 );
}

Widget CreateXpmDisplayToggleButton( Widget parent, int kind, char **icon_data )
{
  int n;
  Arg args[5];
  Widget          button;
  XpmAttributes   attributes;
  int             status, status1;
  Display         *dpy = XtDisplay ( parent );
  Pixmap          pix, mask;

  /*
   * Create a button widget
   */

  n = 0;
  XtSetArg(args[n], XmNindicatorOn, False);  n++;
  XtSetArg(args[n], XmNshadowThickness, 2);  n++;
  if (disp_kind_initial_on(kind))
    { XtSetArg(args[n], XmNset, True); n++; }
  button = XtCreateManagedWidget ( "button", xmToggleButtonWidgetClass,
				   parent, args, n);
  /*
   * Retrieve the depth and colormap used by this widget
   * and store the results in the corresponding field
   * of an XpmAttributes structure.
   */

  XtVaGetValues ( button, 
		  XmNdepth,    &attributes.depth,
		  XmNcolormap, &attributes.colormap,
		  NULL);

  /*
   * Specify the visual to be used and set the XpmAttributes mask.
   */

  attributes.visual = DefaultVisual ( dpy, DefaultScreen ( dpy ) );
  attributes.valuemask = XpmDepth | XpmColormap | XpmVisual;

  /*
   * Create the pixmap
   */

  status  = XpmCreatePixmapFromData( dpy, DefaultRootWindow ( dpy ), 
				     icon_data, &pix, &mask, &attributes );
  /*
   * The mask isn't used, so free it if one was created.
   */

  if ( mask ) XFreePixmap( dpy, mask );
  /*
   * Install the pixmap in the button
   */
     
  if ( status == XpmSuccess ) 
    XtVaSetValues ( button, XmNlabelType, XmPIXMAP, XmNlabelPixmap, pix, NULL );
  XtAddCallback( button, XmNvalueChangedCallback, (XtCallbackProc)  wdis3dcb,
		 (XtPointer) kind );
    
  return ( button );
}

/* morph division number */
Widget tilenoform, tilenotxt;

void settilenocb( Widget w, XtPointer cld, XtPointer *cad )
{
  char  str[BUFSIZ];
  void  settext(Widget, char *);

  XtManageChild( tilenoform );
  sprintf( str, "%d", swin->tileno ); 
  settext( tilenotxt, str );
}

void tilenookcb(Widget w, XtPointer cld, XtPointer *cad)
{
  unsigned char *str;

  str = (unsigned char *) XmTextGetString(tilenotxt);
  swin->tileno = atoi(str);
  XtFree(str);
  XtUnmanageChild( tilenoform );
}

void tilenocnclcb(Widget w, XtPointer cld, XtPointer *cad)
{
  XtUnmanageChild( tilenoform );
}

static void createtilenoform(Widget parent)
{
  int   n;
  Arg   args[20];
  Widget divrc, okrc, lbl, okbt, cnclbt;
  void  tilenocnclcb(Widget w, XtPointer cld, XtPointer *cad);
  void  tilenookcb(Widget w, XtPointer cld, XtPointer *cad);

  n = 0;
  XtSetArg(args[n], XmNautoUnmanage, False);  n++;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ++n;
  tilenoform = (Widget) XmCreateFormDialog(parent, "listform", args, n);

  divrc  = (Widget) XmCreateRowColumn(tilenoform, "winrc1", NULL, 0);
  XtManageChild(divrc);
  lbl    = XtCreateManagedWidget("Tile No.", xmLabelWidgetClass,
				 divrc, NULL, 0);

  tilenotxt = (Widget) XmCreateText(divrc, "nmtext", NULL, 0);
  XtManageChild(tilenotxt);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
  okrc   = (Widget) XmCreateRowColumn(divrc, "winrc1", args, n);
  XtManageChild(okrc);
  okbt   = XtCreateManagedWidget("Ok", xmPushButtonWidgetClass, okrc, NULL, 0);
  cnclbt = XtCreateManagedWidget("Cancel", xmPushButtonWidgetClass, okrc, NULL, 0);
  XtAddCallback(okbt, XmNactivateCallback, (XtCallbackProc) tilenookcb, NULL);
  XtAddCallback(cnclbt, XmNactivateCallback, (XtCallbackProc) tilenocnclcb, NULL);
}

void createwindisp3d( Widget parent )
{
  int   n;
  Arg   args[20], pbargs[1];
  Widget tb, pb, fr, rc, yrc;
  void settilenocb( Widget, XtPointer, XtPointer * );
  
  fr = XtCreateManagedWidget("wchFrame", xmFrameWidgetClass, parent, NULL, 0);

  XtSetArg(pbargs[0], XmNalignment, XmALIGNMENT_BEGINNING);

  n = 0;
  XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); ++n;
  (void) XtCreateManagedWidget("Display", xmLabelWidgetClass, fr, args, n);

  n = 0;
  XtSetArg(args[n], XmNisAligned,    False); ++n;
  XtSetArg(args[n], XmNspacing,      0); ++n;
  XtSetArg(args[n], XmNmarginWidth,  0); ++n;
  XtSetArg(args[n], XmNmarginHeight, 0); ++n;
  yrc = XtCreateManagedWidget("smdrc", xmRowColumnWidgetClass, fr, args, n);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
  XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); ++n;
  XtSetArg(args[n], XmNnumColumns,   4); ++n;
  XtSetArg(args[n], XmNspacing,      0); ++n;
  XtSetArg(args[n], XmNmarginWidth,  0); ++n;
  XtSetArg(args[n], XmNmarginHeight, 0); ++n;
  rc = XtCreateManagedWidget("smdrc", xmRowColumnWidgetClass, yrc, args, n);
  
  (void) CreateXpmDisplayToggleButton ( rc, DISP_HMAP,   icon_vertex );
  (void) CreateXpmDisplayToggleButton ( rc, DISP_WIRE,   icon_wire );
  (void) CreateXpmDisplayToggleButton ( rc, DISP_SHADE,  icon_shade );
  (void) CreateXpmDisplayToggleButton ( rc, DISP_CPOINT, icon_cvp );
  (void) CreateXpmDisplayToggleButton ( rc, DISP_CMESH,  icon_Enhanced );
  (void) CreateXpmDisplayToggleButton ( rc, DISP_LOOP,   icon_boundary );
  (void) CreateXpmDisplayToggleButton ( rc, DISP_GROUP,  icon_tile );
  (void) CreateXpmDisplayToggleButton ( rc, DISP_SPATH,  icon_spath );
  (void) CreateXpmDisplayToggleButton ( rc, DISP_COAXIS, icon_axis );
  
  (void) XtCreateManagedWidget("smdsep", xmSeparatorWidgetClass, yrc, NULL, 0);

  n = 0;
  XtSetArg(args[n], XmNindicatorType, XmONE_OF_MANY);  n++;
  if ( swin->ext_display ) {
    XtSetArg(args[n], XmNset, True); ++n;
  }
  tb = XtCreateManagedWidget("Enhanced", xmToggleButtonWidgetClass, yrc, args, n);
  XtAddCallback(tb, XmNdisarmCallback, (XtCallbackProc) wdis3denhcb, NULL);

  pb = XtCreateManagedWidget("Set Tile No.", xmPushButtonWidgetClass, yrc, pbargs, 1);
  XtAddCallback(pb, XmNarmCallback, (XtCallbackProc) settilenocb, NULL);
}

static void gmorphcb( Widget w, XtPointer cld, XtPointer *cad )
{
  int    a, i;
  Spvt   *v;
  Sppd   *ppd;
  Spfc   *fc;
  int    div;
  char   file[BUFSIZ];
  double p, q;
  Vec    *mvec1, *mvec2;
  void   drawwindow(int);
  void   calc_fnorm(Spfc *);
  void   ppdnorm( Sppd * );
  int    saveRgbImage( char *, long, long );
  
  if ( swin->morph_ppd != (Sppd *) NULL ) {
    /* reset */
    swin->screenatr[0].view_ppd = (Sppd *) NULL;
    swin->screenatr[1].view_ppd = (Sppd *) NULL;
    drawwindow( SCREEN1 );
    drawwindow( SCREEN2 );
    
    ppd = swin->screenatr[0].view_ppd = swin->morph_ppd;
    div = swin->mdiv - 1;
    if ( div < 1 ) div = 1;
    if ( ppd != (Sppd *) NULL ) {
      for (a = 0; a <= div; ++a) {
	q = (double) a / (double) div;
	p = 1.0 - q;
	mvec1 = ppd->mvec1;
	mvec2 = ppd->mvec2;
	for (i = 0, v = ppd->spvt; v != (Spvt *) NULL; ++i, v = v->nxt) {
	  v->vec.x = p * mvec1[i].x + q * mvec2[i].x;
	  v->vec.y = p * mvec1[i].y + q * mvec2[i].y;
	  v->vec.z = p * mvec1[i].z + q * mvec2[i].z;
	}
	/* update normal */
	for (fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt) {
	  calc_fnorm(fc);
	}
	if ( swin->smooth_shading ) {
	  ppdnorm( ppd );
	}
	drawwindow( SCREEN1 );
	usleep(MORPH_FRAME_USEC);
	if ( swin->saveimg ) {
	  sprintf( file, "mymorph_%03d.sgi", a );
	  (void) saveRgbImage( file,
			       swin->screenatr[0].width - 1,
			       swin->screenatr[0].height - 1 );
			  
	}
      }
    }
  }
}

static void gmorphorgmeshcb(Widget w, XtPointer cld, XtPointer *cad)
{
  void  drawwindow(int);

  swin->screenatr[0].view_ppd = swin->screenatr[0].current_ppd;
  swin->screenatr[1].view_ppd = swin->screenatr[1].current_ppd;

  drawwindow( SCREEN1 );
  drawwindow( SCREEN2 );
  
  swin->view_attribs = VIEW_ORGMESH;
}

static void gmorphresetcb(Widget w, XtPointer cld, XtPointer *cad)
{
  Sppd  *ppd;
  Spvt  *v;
  Spfc  *fc;
  Vec   *mvec1;
  void  calc_fnorm(Spfc *);
  void  drawwindow(int);
  void   ppdnorm( Sppd * );

  if ( swin->hppd == NULL ) return;
  if ( swin->morph_ppd == NULL ) return;
  
  ppd = swin->screenatr[0].view_ppd = swin->morph_ppd;
  swin->screenatr[1].view_ppd = (Sppd *) NULL;

  mvec1 = ppd->mvec1;
  for ( v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {
    v->vec.x = mvec1[v->no].x;
    v->vec.y = mvec1[v->no].y;
    v->vec.z = mvec1[v->no].z;
  }
  for (fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt) {
    calc_fnorm(fc);
  }
  if ( swin->smooth_shading ) {
    ppdnorm( ppd );
  }
  
  drawwindow( SCREEN1 );
  drawwindow( SCREEN2 );

  swin->view_attribs = VIEW_INTPMESH;
}

/* morph division number */
Widget mdivform, mdivtxt;

void mdivinitcb(Widget w, XtPointer cld, XtPointer *cad)
{
  char  str[BUFSIZ];
  void  settext(Widget, char *);

  XtManageChild(mdivform);
  sprintf(str, "%d", swin->mdiv); 
  settext(mdivtxt, str);
}

void mdivokcb( Widget w, XtPointer cld, XtPointer *cad )
{
  unsigned char *str;

  str = (unsigned char *) XmTextGetString(mdivtxt);
  swin->mdiv = atoi(str);
  XtFree(str);
  XtUnmanageChild(mdivform);
}

void mdivcnclcb( Widget w, XtPointer cld, XtPointer *cad )
{
  XtUnmanageChild(mdivform);
}

static void createmdivform( Widget parent )
{
  int   n;
  Arg   args[20];
  Widget divrc, okrc, lbl, okbt, cnclbt;
  void  mdivcnclcb(Widget w, XtPointer cld, XtPointer *cad);
  void  mdivokcb(Widget w, XtPointer cld, XtPointer *cad);

  n = 0;
  XtSetArg(args[n], XmNautoUnmanage, False);  n++;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ++n;
  mdivform = (Widget) XmCreateFormDialog(parent, "listform", args, n);

  divrc  = (Widget) XmCreateRowColumn(mdivform, "winrc1", NULL, 0);
  XtManageChild(divrc);
  lbl    = XtCreateManagedWidget("Morphing divide number", xmLabelWidgetClass,
				 divrc, NULL, 0 );

  mdivtxt = (Widget) XmCreateText(divrc, "nmtext", NULL, 0);
  XtManageChild(mdivtxt);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
  okrc   = (Widget) XmCreateRowColumn(divrc, "winrc1", args, n);
  XtManageChild(okrc);
  okbt   = XtCreateManagedWidget("Ok", xmPushButtonWidgetClass, okrc, NULL, 0);
  cnclbt = XtCreateManagedWidget("Cancel", xmPushButtonWidgetClass, okrc, NULL, 0);
  XtAddCallback(okbt, XmNactivateCallback, (XtCallbackProc) mdivokcb, NULL);
  XtAddCallback(cnclbt, XmNactivateCallback, (XtCallbackProc) mdivcnclcb, NULL);
}

/* edit */

static Widget edit_tb[EDIT_NONE];

static void change_editcb( Widget w, XtPointer cld, XmToggleButtonCallbackStruct *cad )
{
  int  i;
  void cancel_all( void );
  void screen_initialize_sgraph( ScreenAtr * );
  void screen_exit_sgraph( ScreenAtr * );

  i = swin->edit_type;
  if ( i != EDIT_NONE ) {
    XmToggleButtonSetState( edit_tb[i], False, False );
  }    

  cancel_all();
  
  if ( i == EDIT_SPATH ) {
    screen_exit_sgraph( &(swin->screenatr[SCREEN1]) );
    screen_exit_sgraph( &(swin->screenatr[SCREEN2]) );
  }
  swin->edit_type  = EDIT_NONE;
  
  swin->edit_type = (int) cld;

  if ( swin->edit_type == EDIT_SPATH ) {
    screen_initialize_sgraph( &(swin->screenatr[SCREEN1]) );
    screen_initialize_sgraph( &(swin->screenatr[SCREEN2]) );
  }
}

Widget CreateXpmEditToggleButton( Widget parent, int kind, char **icon_data )
{
  int n;
  Arg args[5];
  Widget          button;
  XpmAttributes   attributes;
  int             status, status1;
  Display         *dpy = XtDisplay ( parent );
  Pixmap          pix, mask;

  /*
   * Create a button widget
   */

  n = 0;
  XtSetArg(args[n], XmNindicatorOn, False);  n++;
  XtSetArg(args[n], XmNshadowThickness, 2);  n++;
  button = XtCreateManagedWidget( "button", xmToggleButtonWidgetClass,
				   parent, args, n );
  /*
   * Retrieve the depth and colormap used by this widget
   * and store the results in the corresponding field
   * of an XpmAttributes structure.
   */

  XtVaGetValues( button, 
		 XmNdepth,    &attributes.depth,
		 XmNcolormap, &attributes.colormap,
		 NULL);

  /*
   * Specify the visual to be used and set the XpmAttributes mask.
   */

  attributes.visual = DefaultVisual ( dpy, DefaultScreen ( dpy ) );
  attributes.valuemask = XpmDepth | XpmColormap | XpmVisual;

  /*
   * Create the pixmap
   */

  status  = XpmCreatePixmapFromData( dpy, DefaultRootWindow ( dpy ), 
				     icon_data, &pix, &mask, &attributes );
  /*
   * The mask isn't used, so free it if one was created.
   */

  if ( mask ) XFreePixmap( dpy, mask );
  /*
   * Install the pixmap in the button
   */
     
  if ( status == XpmSuccess ) 
    XtVaSetValues ( button, XmNlabelType, XmPIXMAP, XmNlabelPixmap, pix, NULL );
  XtAddCallback( button, XmNarmCallback, (XtCallbackProc)  change_editcb,
		 (XtPointer) kind );
    
  return ( button );
}

void creategmorphbutton( Widget parent )
{
  int   n;
  Arg   args[20], pbargs[1];
  Widget fr, pb, rc, rbox, yrc;
  void ppdtocmeshcb( Widget, XtPointer, XtPointer * );
  void makegmorphcb( Widget, XtPointer, XtPointer * );
  void makeshortestpathcb( Widget, XtPointer, XtPointer * );
  void SGMakeShortestPathsCB( Widget, XtPointer, XtPointer * );
  void clearshortestpathcb( Widget, XtPointer, XtPointer * );

  fr = XtCreateManagedWidget("wchFrame", xmFrameWidgetClass, parent, NULL, 0);

  XtSetArg(pbargs[0], XmNalignment, XmALIGNMENT_BEGINNING);

  n = 0;
  XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); ++n;
  (void) XtCreateManagedWidget("Edit", xmLabelWidgetClass, fr, args, n);

  n = 0;
  XtSetArg(args[n], XmNisAligned,    False); ++n;
  XtSetArg(args[n], XmNspacing,      0); ++n;
  XtSetArg(args[n], XmNmarginWidth,  0); ++n;
  XtSetArg(args[n], XmNmarginHeight, 0); ++n;
  yrc = XtCreateManagedWidget("smdrc", xmRowColumnWidgetClass, fr, args, n);
  
  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
  XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); ++n;
  XtSetArg(args[n], XmNnumColumns, 4); ++n;
  XtSetArg(args[n], XmNspacing,      0); ++n;
  XtSetArg(args[n], XmNmarginWidth,  0); ++n;
  XtSetArg(args[n], XmNmarginHeight, 0); ++n;
  rc = XtCreateManagedWidget("smdrc", xmRowColumnWidgetClass, yrc, args, n);
  
  edit_tb[0] = CreateXpmEditToggleButton( rc, EDIT_LOOP,        icon_create_ppdloop );
  edit_tb[1] = CreateXpmEditToggleButton( rc, EDIT_DEL_LOOP,    icon_delete_ppdloop );
  edit_tb[2] = CreateXpmEditToggleButton( rc, EDIT_FACE,        icon_create_ppdface );
  edit_tb[3] = CreateXpmEditToggleButton( rc, EDIT_DEL_FACE,    icon_delete_ppdface );
  edit_tb[4] = CreateXpmEditToggleButton( rc, EDIT_HPOINT,      icon_create_cvp );
  edit_tb[5] = CreateXpmEditToggleButton( rc, EDIT_MOV_HPOINT,  icon_move_cvp );
  edit_tb[6] = CreateXpmEditToggleButton( rc, EDIT_DEL_HPOINT,  icon_delete_cvp );
  edit_tb[7] = CreateXpmEditToggleButton( rc, EDIT_HFACE,       icon_create_pcm_face );
  edit_tb[8] = CreateXpmEditToggleButton( rc, EDIT_DEL_HFACE,   icon_delete_pcm_face );
  edit_tb[9]  = CreateXpmEditToggleButton( rc, EDIT_SPATH,      icon_make_shortestpath );
  edit_tb[10] = CreateXpmEditToggleButton( rc, EDIT_DEL_SPATH,  icon_delete_shortestpath);
  edit_tb[11] = CreateXpmEditToggleButton( rc, EDIT_SPATH_MANU, icon_manu_shortestpath );
  
  (void) XtCreateManagedWidget("smdsep", xmSeparatorWidgetClass, yrc, NULL, 0);
  
  pb = XtCreateManagedWidget("SPaths Create", xmPushButtonWidgetClass, yrc, pbargs, 1);
  XtAddCallback(pb, XmNdisarmCallback, (XtCallbackProc) makeshortestpathcb, NULL);
/*   XtAddCallback(pb, XmNdisarmCallback, (XtCallbackProc) SGMakeShortestPathsCB, NULL); */
  
  pb = XtCreateManagedWidget("SPaths Clear", xmPushButtonWidgetClass, yrc, pbargs, 1);
  XtAddCallback(pb, XmNdisarmCallback, (XtCallbackProc) clearshortestpathcb, NULL);
  
  fr = XtCreateManagedWidget("wchFrame", xmFrameWidgetClass, parent, NULL, 0);
  n = 0;
  XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); ++n;
  (void) XtCreateManagedWidget("Morph", xmLabelWidgetClass, fr, args, n);

  yrc = XtCreateManagedWidget("smdrc", xmRowColumnWidgetClass, fr, NULL, 0);

  pb = XtCreateManagedWidget("Make Morph", xmPushButtonWidgetClass, yrc, pbargs, 1);
  XtAddCallback(pb, XmNdisarmCallback, (XtCallbackProc) makegmorphcb, NULL);
  
  pb = XtCreateManagedWidget("Start", xmPushButtonWidgetClass, yrc, pbargs, 1);
  XtAddCallback(pb, XmNdisarmCallback, (XtCallbackProc) gmorphcb, NULL);
  
  pb = XtCreateManagedWidget("Set Morph Divs", xmPushButtonWidgetClass, yrc, pbargs, 1);
  XtAddCallback(pb, XmNdisarmCallback, (XtCallbackProc) mdivinitcb, NULL);
  
  pb = XtCreateManagedWidget("Interp. Mesh", xmPushButtonWidgetClass, yrc, pbargs, 1);
  XtAddCallback(pb, XmNdisarmCallback, (XtCallbackProc) gmorphresetcb, NULL);
  
  pb = XtCreateManagedWidget("Orig. Meshes", xmPushButtonWidgetClass, yrc, pbargs, 1);
  XtAddCallback(pb, XmNdisarmCallback, (XtCallbackProc) gmorphorgmeshcb, NULL);
  
}

Widget createwch( Widget parent, Widget tm )
{
  int   n;
  Arg   args[20];
  Widget wchrc;
  void   createwindisp3d(Widget);
  void   creategmorphbutton(Widget);

  n = 0;
  XtSetArg(args[n], XmNresizeWidth, False);  n++;
  XtSetArg(args[n], XmNwidth, RSIZE);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, tm);  n++;
  XtSetArg(args[n], XmNtopOffset, 5);  n++;
  XtSetArg(args[n], XmNrightOffset, 5);  n++;
  wchrc = (Widget) XmCreateRowColumn(parent, "winrc1", args, n);
  XtManageChild(wchrc);

  createwindisp3d(wchrc);
  creategmorphbutton(wchrc);

  progress=XtVaCreateManagedWidget("progress",
				   xrhpProgressWidgetClass, wchrc,
				   XmNwidth, RSIZE,
				   XmNheight, 15,
				   XmNmaxValue, 100,
				   XmNshowPercent, True,
				   XmNshowTime, False, NULL);

/* 				   XtVaTypedArg, XmNfont, */
/* 				   XmRString, "-adobe-helvetica-bold-r-normal--12-*", */
/* 				   47,  */
  ProgressSetValue( 0 );
  
  return wchrc;
}

/* information window */

static Widget pinfo = NULL;
static XmTextPosition w_position;

void displayinfo(char *fmt, ...)
{
  char str[BUFSIZ];
/*   void  infoprompt(void); */

  va_list argp;
  
  if ( pinfo ) {
    va_start(argp, fmt);
    vsprintf(str, fmt, argp);
    va_end(argp);

    XmTextInsert(pinfo, w_position, str);
    w_position += strlen(str);
    XtVaSetValues(pinfo, XmNcursorPosition, w_position, NULL);
    XmTextShowPosition(pinfo, w_position);
  } else {
    if ( swin->prn ) {
      va_start(argp, fmt);
      vfprintf(stdout, fmt, argp);
      va_end(argp);
      fflush(stdout);
      fflush(stdout);
    }
  }
}

static Widget createinfowindow(Widget mainw, Widget wchrc)
{
  int   n;
  Arg   args[15];

  n = 0;
  XtSetArg(args[n], XmNleftAttachment,  XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNrightWidget, wchrc);  n++;
  XtSetArg(args[n], XmNbottomOffset, 5);  n++;
  XtSetArg(args[n], XmNleftOffset, 5);  n++;
  XtSetArg(args[n], XmNeditMode,     XmMULTI_LINE_EDIT);  n++;
  XtSetArg(args[n], XmNheight,       100);  n++;
  XtSetArg(args[n], XmNwordWrap,     True); n++;
  XtSetArg(args[n], XmNscrollHorizontal, False); n++;
  pinfo = (Widget) XmCreateScrolledText(mainw, "wintext", args, n);
  XtManageChild(pinfo);

  displayinfo("Welcome to GMorph!! \n");
  
  return pinfo;
}

/******************* window lay out *************************/

void initsubwindows(Widget parent)
{
  Widget mainw, tm, wchrc;
  void createfileserver(Widget);
  void createquit(Widget);
  void selecthvertexform(Widget);
  void createshortestpathform(Widget);
/*   void createcolorchooser(Widget); */

  mainw = XmCreateForm(parent, "mainwindow", NULL, 0); 
  XtManageChild(mainw);

  tm = createmenupane(mainw);
  wchrc = createwch(mainw, tm);
  pinfo = createinfowindow(mainw, wchrc);
  create3dwinpane(mainw, tm, wchrc, pinfo);
  createfileserver(parent);
  createquit(parent);
  createmdivform(parent);
/*   createcolorchooser(parent); */
  createtilenoform(parent);
}

void settext(Widget w, char *file)
{
  XmTextSetString(w, file);
}

extern char versionshort[];

void set_windowtitle(char *str)
{
  int n;
  char title[BUFSIZ];
  Arg args[1];
  
  n = 0;
  
  sprintf( title, "%s: %s", versionshort, str );
  XtSetArg( args[n], XmNtitle, title );  n++;
  XtSetValues( swin->toplevel, args, n );
}

