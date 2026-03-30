
/*
 * Progress - A simple progress meter widget, incorporating display of
 *		percentage complete, and estimated time to completion.
 *
 * Copyright (c) 1997 Richard H. Porter
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * Author: Dick Porter <dick@cymru.net>
 *
 */

#include <Xm/XmP.h>
#include <Xm/PrimitiveP.h>
#include <Xm/DrawP.h>
#include <Xm/XmStrDefs.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ProgressP.h"

extern void XrhpProgressSetValue( Widget, int );

#define PROGRESS_DEFAULTHEIGHT 20
#define PROGRESS_DEFAULTWIDTH 200

#define offset(field) XtOffsetOf(XrhpProgressRec, field)

static XtResource resources[] = {
	{XmNmaxValue, XmCMaxValue, XmRInt, sizeof(int),
		offset(progress.max_value), XmRString, "100"},
	{XmNshowPercent, XmCShowPercent, XmRBoolean, sizeof(Boolean),
		offset(progress.show_percent), XmRString, "False"},
	{XmNshowTime, XmCShowTime, XmRBoolean, sizeof(Boolean),
		offset(progress.show_time), XmRString, "True"},
	{XmNvalue, XmCValue, XmRInt, sizeof(int),
		offset(progress.value), XmRString, "0"},
	{XmNfont, XmCFont, XmRFontStruct, sizeof(XFontStruct *),
		offset(progress.font), XmRString, XtDefaultFont},
};

#undef offset

/* Declaration of methods */

static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Redisplay(Widget, XEvent *, Region);
static void Destroy(Widget);
static void Resize(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static XtGeometryResult QueryGeometry(Widget, XtWidgetGeometry *, XtWidgetGeometry *);

/* these Core methods not used:
 *
 * static void ClassInitialize();
 * static void Realize();
 */

XrhpProgressClassRec xrhpProgressClassRec = {
	{
/* core_class fields */
	/* superclass */		(WidgetClass) &xmPrimitiveClassRec,
	/* class_name */		"XrhpProgress",
	/* widget_size */		sizeof(XrhpProgressRec),
	/* class_initialize */		NULL,
	/* class_part_initialize */	NULL,
	/* class_inited */		FALSE,
	/* initialize */		Initialize,
	/* initialize_hook */		NULL,
	/* realize */			XtInheritRealize,
	/* actions */			NULL,
	/* num_actions */		0,
	/* resources */			resources,
	/* num_resources */		XtNumber(resources),
	/* xrm_class */			NULLQUARK,
	/* compress_motion */		TRUE,
	/* compress_exposure */		XtExposeCompressMultiple,
	/* compress_enterleave */	TRUE,
	/* visible_interest */		FALSE,
	/* destroy */			Destroy,
	/* resize */			Resize,
	/* expose */			Redisplay,
	/* set_values */		SetValues,
	/* set_values_hook */		NULL,
	/* set_values_almost */		XtInheritSetValuesAlmost,
	/* get_values_hook */		NULL,
	/* accept_focus */		NULL,
	/* version */			XtVersion,
	/* callback_private */		NULL,
	/* tm_table */			XtInheritTranslations,
	/* query_geometry */		QueryGeometry,
	/* display_accelerator */	XtInheritDisplayAccelerator,
	/* extension */			NULL
	},
	{
/* Primitive class fields */
	/* border_highlight */		XmInheritBorderHighlight,       
	/* border_unhighlight */	XmInheritBorderUnhighlight,    
	/* translations */		XtInheritTranslations,      
	/* arm_and_activate */		NULL,             
	/* syn resources */		NULL,           
	/* num_syn_resources */		0, 
	/* extension */			NULL,             
	},
	{
/* extension */				0,
	},
};

WidgetClass xrhpProgressWidgetClass=(WidgetClass)&xrhpProgressClassRec;

static void
SetClipRegions(Widget w)
{
	XrhpProgressWidget cw=(XrhpProgressWidget)w;
	XRectangle ClipRect;

	ClipRect.x=cw->primitive.shadow_thickness +
			cw->primitive.highlight_thickness;
	ClipRect.y=cw->primitive.shadow_thickness +
			cw->primitive.highlight_thickness;
	ClipRect.width=cw->core.width -
			(2 * (cw->primitive.shadow_thickness +
			cw->primitive.highlight_thickness));
	ClipRect.height=cw->core.height -
			(2 * (cw->primitive.shadow_thickness +
			cw->primitive.highlight_thickness));

	XSetClipRectangles(XtDisplay(cw), cw->progress.line_gc, 0, 0,
		&ClipRect, 1, Unsorted);
	XSetClipRectangles(XtDisplay(cw), cw->progress.blank_gc, 0, 0,
		&ClipRect, 1, Unsorted);
	XSetClipRectangles(XtDisplay(cw), cw->progress.text_gc, 0, 0,
		&ClipRect, 1, Unsorted);
}

static void
GetGC(Widget w)
{
	XrhpProgressWidget cw=(XrhpProgressWidget)w;
	XGCValues values;
	XtGCMask mask=GCForeground | GCBackground;

	values.foreground=cw->core.background_pixel;
	values.background=cw->primitive.foreground;

	cw->progress.blank_gc=XtGetGC((Widget)cw, mask, &values);

	values.foreground=cw->primitive.foreground;
	values.background=cw->core.background_pixel;

	cw->progress.line_gc=XtGetGC((Widget)cw, mask, &values);

	values.function=GXxor;
	values.foreground=cw->primitive.foreground ^ cw->core.background_pixel;
	mask=GCFunction | GCForeground | GCBackground;

	cw->progress.text_gc=XtGetGC((Widget)cw, mask, &values);
	XSetFont(XtDisplay(cw), cw->progress.text_gc, (cw->progress.font)->fid);

	SetClipRegions((Widget)cw);
}

static void
Initialize(Widget treq, Widget tnew, ArgList args, Cardinal *num_args)
{
	XrhpProgressWidget new=(XrhpProgressWidget)tnew;

	new->progress.start_time=time(NULL);

	/* 
	 *  Check instance values set by resources that may be invalid. 
	 */

	if(new->progress.max_value<1) {
		XtWarning("Progress: Maximum value must be >0.");
		new->progress.max_value=1;
	}

	if(new->progress.value>new->progress.max_value) {
		XtWarning("Progress: Value must not be > maximum value.");
		new->progress.value=new->progress.max_value;
	}

	if(new->core.width==0) {
		XtWarning("Progress: core width is 0, setting to default.");
		new->core.width=PROGRESS_DEFAULTWIDTH;
	}

	if(new->core.height==0) {
		XtWarning("Progress: core height is 0, setting to default.");
		new->core.height=PROGRESS_DEFAULTHEIGHT;
	}

	new->core.width+=(new->primitive.shadow_thickness +
			new->primitive.highlight_thickness);
	new->core.height+=(new->primitive.shadow_thickness +
			new->primitive.highlight_thickness);

	GetGC((Widget)new);
}

static void
Redisplay(Widget w, XEvent *event, Region region)
{
	XrhpProgressWidget cw=(XrhpProgressWidget)w;

	if(event->xany.type != Expose)
		return;

	if (!XtIsRealized((Widget)cw))
		return;

	if(cw->primitive.shadow_thickness > 0)
		_XmDrawShadows(XtDisplay(cw), XtWindow(cw),
				cw->primitive.bottom_shadow_GC,
				cw->primitive.top_shadow_GC,
				(int)cw->primitive.highlight_thickness,
				(int)cw->primitive.highlight_thickness,
				(int)(cw->core.width -
					(2*cw->primitive.highlight_thickness)),
				(int)(cw->core.height -
					(2*cw->primitive.highlight_thickness)),
				(int)cw->primitive.shadow_thickness,
				XmSHADOW_OUT);

	if(cw->primitive.highlighted) {
		if(((XrhpProgressWidgetClass)XtClass(cw))->
			primitive_class.border_highlight) {
			(*((XrhpProgressWidgetClass)XtClass(cw))->
				primitive_class.border_highlight)((Widget)cw);
		}
	} else {
		if(((XrhpProgressWidgetClass)XtClass(cw))->
			primitive_class.border_unhighlight) {
			(*((XrhpProgressWidgetClass)XtClass(cw))->
				primitive_class.border_unhighlight)((Widget)cw);
		}
	}

	XrhpProgressSetValue(w, cw->progress.value);
}

static Boolean
SetValues(Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args)
{
	XrhpProgressWidget curcw=(XrhpProgressWidget)current;
	XrhpProgressWidget newcw=(XrhpProgressWidget)new;
	Boolean do_redisplay = False;

	if((curcw->primitive.foreground != newcw->primitive.foreground) ||
	   (curcw->core.background_pixel != newcw->core.background_pixel) ||
	   (curcw->progress.font != newcw->progress.font)) {
		XtReleaseGC((Widget)curcw, curcw->progress.line_gc);
		XtReleaseGC((Widget)curcw, curcw->progress.blank_gc);
		XtReleaseGC((Widget)curcw, curcw->progress.text_gc);
		GetGC((Widget)newcw);

		do_redisplay=True;
	}

	if((curcw->core.height != newcw->core.height) ||
	   (curcw->core.width != newcw->core.width)) {
		SetClipRegions((Widget)newcw);

		do_redisplay=True;
	}

	if(newcw->progress.value==0)
		newcw->progress.start_time=time(NULL);

	if(curcw->progress.value != newcw->progress.value) {
		if(newcw->progress.value>newcw->progress.max_value) {
			XtWarning("Progress: value cant be > max_value.");
			newcw->progress.value=newcw->progress.max_value;
		}

		do_redisplay=True;
	}

	if(curcw->progress.max_value != newcw->progress.max_value)
		do_redisplay=True;

	if(curcw->progress.show_percent != newcw->progress.show_percent)
		do_redisplay=True;

	if(curcw->progress.show_time != newcw->progress.show_time)
		do_redisplay=True;

	return do_redisplay;
}


static void
Destroy(Widget w)
{
	XrhpProgressWidget cw=(XrhpProgressWidget)w;

	if(cw->progress.line_gc)
		XFreeGC(XtDisplay(cw), cw->progress.line_gc);

	if(cw->progress.blank_gc)
		XFreeGC(XtDisplay(cw), cw->progress.blank_gc);

	if(cw->progress.text_gc)
		XFreeGC(XtDisplay(cw), cw->progress.text_gc);
}

static void
Resize(Widget w)
{
	XrhpProgressWidget cw=(XrhpProgressWidget)w;

	SetClipRegions((Widget)cw);
}

static XtGeometryResult QueryGeometry(Widget w, XtWidgetGeometry *proposed, XtWidgetGeometry *answer)
{
	return XtGeometryYes;
}

void XrhpProgressSetValue(Widget w, int value)
{
	XrhpProgressWidget cw=(XrhpProgressWidget)w;
	Display *display;
	Window window;

	int percent;
	time_t elapsed_time, etf_time;
	int line_len,x,y;

	char buf[16];
	int dir,ascent,descent;
	XCharStruct overall;

	cw->progress.value=value;
	if(value==0)
		cw->progress.start_time=time(NULL);

	if(!XtIsRealized((Widget)cw))
		return;

	display=XtDisplay(cw);
	window=XtWindow(cw);

	/* The line is clipped by the GC, so I can save myself some
		arithmetic here by not taking the borders into account */

	XSetLineAttributes(display, cw->progress.line_gc, cw->core.height,
		LineSolid, CapButt, JoinBevel);
	XSetLineAttributes(display, cw->progress.blank_gc, cw->core.height,
		LineSolid, CapButt, JoinBevel);

	line_len=(int)(cw->progress.value * cw->core.width) /
		cw->progress.max_value;

	/* Draw percentage line */
	XDrawLine(display, window, cw->progress.line_gc, 0, cw->core.height/2,
		line_len, cw->core.height/2);
	XDrawLine(display, window, cw->progress.blank_gc, line_len+1, cw->core.height/2, cw->core.width, cw->core.height/2);

	percent=(int)(cw->progress.value * 100 / cw->progress.max_value);

	elapsed_time=time(NULL) - cw->progress.start_time;

	if(percent)
		etf_time=(100 * elapsed_time) / percent - elapsed_time;
	else
		etf_time=0;

	if(cw->progress.show_percent && percent > 0 && percent < 100) {
		sprintf(buf, "%d%%", percent);

		XTextExtents(cw->progress.font, buf, strlen(buf), &dir,
			&ascent, &descent, &overall);
		x=(cw->core.width - overall.width) / 2;
		y=(cw->core.height - (ascent + descent)) / 2 + ascent;

		XDrawString(display, window, cw->progress.text_gc, x, y,
			buf, strlen(buf));
	}

	if(cw->progress.show_time && elapsed_time > 0 &&
	   percent > 0 && percent < 100) {
		div_t time_div;

		if(elapsed_time < 60)
			sprintf(buf, "%d sec%s", (int)elapsed_time,
				elapsed_time==1?"":"s");
		else if(elapsed_time<3600) {
			time_div=div(elapsed_time, 60);
			sprintf(buf, "%d:%02d mins", time_div.quot,
				time_div.rem);
		} else {
			time_div=div(elapsed_time, 60);
			time_div=div(time_div.quot, 60);
			sprintf(buf, "%d:%02d hrs", time_div.quot,
				time_div.rem);
		}

		XTextExtents(cw->progress.font, buf, strlen(buf), &dir,
			&ascent, &descent, &overall);
		x=cw->primitive.shadow_thickness +
			cw->primitive.highlight_thickness + 2;
		y=(cw->core.height - (ascent + descent)) / 2 + ascent;

		XDrawString(display, window, cw->progress.text_gc, x, y,
			buf, strlen(buf));
	}

	if(cw->progress.show_time && etf_time > 0 && percent < 100) {
		div_t time_div;

		if(etf_time < 60)
			sprintf(buf, "%d sec%s", (int)etf_time,
				etf_time==1?"":"s");
		else if(etf_time<3600) {
			time_div=div(etf_time, 60);
			sprintf(buf, "%d:%02d mins", time_div.quot,
				time_div.rem);
		} else {
			time_div=div(etf_time, 60);
			time_div=div(time_div.quot, 60);
			sprintf(buf, "%d:%02d hrs", time_div.quot,
				time_div.rem);
		}

		XTextExtents(cw->progress.font, buf, strlen(buf), &dir,
			&ascent, &descent, &overall);
		x=cw->core.width - (overall.width +
			cw->primitive.shadow_thickness +
			cw->primitive.highlight_thickness + 2);
		y=(cw->core.height - (ascent + descent)) / 2 + ascent;

		XDrawString(display, window, cw->progress.text_gc, x, y,
			buf, strlen(buf));
	}
}

void ProgressSetValue( int value )
{
  if ( progress != NULL ) {
    XrhpProgressSetValue( progress, value );
  }
}
  
