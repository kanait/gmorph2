/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "window.h"
#include "smd.h"

/* #include <Xm/Form.h> */
/* #include <Xm/Label.h> */
/* #include <Xm/PushB.h> */
/* #include <Xm/RowColumn.h> */
/* #include <Xm/Frame.h> */
/* #include <Xm/Text.h> */
/* #include <Xm/Separator.h> */
/* #include <Xm/ToggleB.h> */
#include <Sgm/ColorC.h>

/* Static variables */
static Widget dialogCC;
int    rgbBase;

void edit_bgcolorcb( Widget w, XtPointer client_data, XtPointer *call_data )
{
  int   ir, ig, ib;
  short r, g, b;

  r = (short) (swin->screenatr[0].bgrgb[0] * 255);
  g = (short) (swin->screenatr[0].bgrgb[1] * 255);
  b = (short) (swin->screenatr[0].bgrgb[2] * 255);
  SgColorChooserSetCurrentColor(dialogCC, r, g, b);
  SgColorChooserSetStoredColor(dialogCC, r, g, b);
  XtManageChild( dialogCC );
}


/* Callbacks from the dialog window color chooser */

static void ccokcb(Widget w, XtPointer client_data, XtPointer call_data)
{
  int   i;
  short r, g, b;
  void  drawwindow(int);

  SgColorChooserGetColor(w, &r, &g, &b);
  XtUnmanageChild( dialogCC );
  for ( i = 0; i < 2; ++i ) {
    swin->screenatr[i].bgrgb[0] = (float) r / 255;
    swin->screenatr[i].bgrgb[1] = (float) g / 255;
    swin->screenatr[i].bgrgb[2] = (float) b / 255;
    drawwindow(i);
  }
}

static void ccapplycb(Widget w, XtPointer client_data, XtPointer call_data)
{
  short r, g, b;
  int   i;
  void  drawwindow(int);
  
  SgColorChooserGetColor(w, &r, &g, &b);
  for ( i = 0; i < 2; ++i ) {
    swin->screenatr[i].bgrgb[0] = (float) r / 255;
    swin->screenatr[i].bgrgb[1] = (float) g / 255;
    swin->screenatr[i].bgrgb[2] = (float) b / 255;
    drawwindow(i);
  }
}

static void cccancelcb(Widget w, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild( dialogCC );
}

static void ccdragcb(Widget w, XtPointer client_data, XtPointer call_data)
{
/*   valueStatus (w, (SgColorChooserCallbackStruct *) call_data, "DRAG  (Dialog)"); */
}

static void cchelpcb(Widget w, XtPointer client_data, XtPointer call_data)
{
/*   buttonStatus (w, "HELP  (Dialog)"); */
}

static void ccvalueChangedcb(Widget w, XtPointer client_data, XtPointer call_data)
{
/*   valueStatus (w, (SgColorChooserCallbackStruct *) call_data, "VALUE CHANGED  (Dialog)"); */
}

void createcolorchooser(Widget parent)
{
  int n;
  Arg args[10];
  
  rgbBase = rgbBase == 10 ? 16 : 10;
  n = 0;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
  dialogCC = SgCreateColorChooserDialog ( parent, "popup", args, n );
  
  XtVaSetValues (dialogCC, SgNrgbBase, rgbBase, NULL);
  XtAddCallback (dialogCC, XmNokCallback, ccokcb, NULL);
  XtAddCallback (dialogCC, XmNapplyCallback, ccapplycb, NULL);
  XtAddCallback (dialogCC, XmNcancelCallback, cccancelcb, NULL);
  XtAddCallback (dialogCC, XmNdragCallback, ccdragcb, NULL);
  XtAddCallback (dialogCC, XmNhelpCallback, cchelpcb, NULL);
  XtAddCallback (dialogCC, XmNvalueChangedCallback, ccvalueChangedcb, NULL);
}

#ifdef NEVER
/* ##### This is never used. */
/* ##### I am not sure that it should not be hooked up and used, though, */
/* ##### so I am leaving it in. */
/* ##### It can be removed by the first person who is sure it is useless. */
static void  ColorChangeCallback(Widget w, void *client_data,
				 SgColorChooserCallbackStruct * call_data)
{
  Boolean *drag = (Boolean *) client_data;
  char color_text[128];

  if (call_data->reason == XmCR_DRAG)
    sprintf (color_text, "Drag color values: R %d, G %d, B %d", call_data->r,
	     call_data->g, call_data->b);
  else
    sprintf (color_text, "ChangedValue color values: R %d, G %d, B %d",
	     call_data->r, call_data->g, call_data->b);

  XmTextSetString (text_widget, color_text);
}
#endif

