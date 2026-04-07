/* Copyright 1997 by Takashi KANAI */
/*
 * Legacy OpenMotif/X11 UI has been removed; Qt6 + batch CLI only.
 * Opaque placeholders keep legacy callback signatures compilable where still referenced.
 */
typedef void *Widget;
typedef void *XtAppContext;
typedef void *XtPointer;
typedef int Boolean;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef True
#define True 1
#endif
#ifndef False
#define False 0
#endif
