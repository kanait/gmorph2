/*****************************************************
 * busy.c: Demonstrate the use of a work procedure
 *         to unset a busy cursor
 *****************************************************/

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"
#include <X11/cursorfont.h>

void DisplayBusyCursor ( Widget w ) 
{
  Boolean RemoveBusyCursor ( XtPointer );
  static Cursor cursor = 0;
  /* static cursor = NULL;*/

  if ( swin->use_qt_gui || w == NULL )
    return;

  if ( !cursor ) 
    cursor = XCreateFontCursor ( XtDisplay ( w ), XC_watch );  

  XDefineCursor ( XtDisplay ( w ), XtWindow ( w ), cursor );

  XFlush ( XtDisplay ( w ) );

  XtAppAddWorkProc ( XtWidgetToApplicationContext ( w ), 
		     RemoveBusyCursor, ( XtPointer ) w );

  XFreeCursor( XtDisplay( w ), cursor );
}
    
Boolean RemoveBusyCursor ( XtPointer clientData ) 
{
  Widget w = ( Widget ) clientData;

  XUndefineCursor( XtDisplay ( w ), XtWindow ( w ) );

  return ( TRUE );
}
        
                
