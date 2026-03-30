/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

/***************************************
  Solid
****************************************/

Spso *create_ppdsolid(Sppd *ppd)
{
  Spso *so;

  so = (Spso *) malloc(sizeof(Spso));
  
  so->nxt = (Spso *) NULL;
  if (ppd->spso == (Spso *) NULL) {
    so->prv = (Spso *) NULL;
    ppd->epso = ppd->spso = so;
  } else {
    so->prv   = ppd->epso;
    ppd->epso = ppd->epso->nxt = so;
  }

  /* id */
  so->no  = ppd->sid;
  ++( ppd->sn );
  ++( ppd->sid );

  return so;
}

void free_ppdsolid(Spso *so, Sppd *ppd)
{
  if (so == (Spso *) NULL) return;
  
  if (ppd->spso == so) {
    if ((ppd->spso = so->nxt) != (Spso *) NULL)
      so->nxt->prv = (Spso *) NULL;
    else {
      ppd->epso = (Spso *) NULL;
    }
  } else if (ppd->epso == so) {
    so->prv->nxt = (Spso *) NULL;
    ppd->epso = so->prv;
    
  } else {
    so->prv->nxt = so->nxt;
    so->nxt->prv = so->prv;
  }
  
  free(so);
  --( ppd->sn );

}

