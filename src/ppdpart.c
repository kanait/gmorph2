/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

/***************************************
  Part
****************************************/

Sppt *create_ppdpart(Sppd *ppd)
{
  Sppt *pt;

  pt = (Sppt *) malloc(sizeof(Sppt));
  
  pt->nxt = (Sppt *) NULL;
  if (ppd->sppt == (Sppt *) NULL) {
    pt->prv = (Sppt *) NULL;
    ppd->eppt = ppd->sppt = pt;
  } else {
    pt->prv   = ppd->eppt;
    ppd->eppt = ppd->eppt->nxt = pt;
  }

  /* id */
  pt->no   = ppd->pid;
  /* back solid */
  pt->bpso = (Spso *) NULL;
  ++( ppd->pn );
  ++( ppd->pid );

  return pt;
}

void free_ppdpart(Sppt *pt, Sppd *ppd)
{
  if (pt == (Sppt *) NULL) return;
  
  if (ppd->sppt == pt) {
    if ((ppd->sppt = pt->nxt) != (Sppt *) NULL)
      pt->nxt->prv = (Sppt *) NULL;
    else {
      ppd->eppt = (Sppt *) NULL;
    }
  } else if (ppd->eppt == pt) {
    pt->prv->nxt = (Sppt *) NULL;
    ppd->eppt = pt->prv;
    
  } else {
    pt->prv->nxt = pt->nxt;
    pt->nxt->prv = pt->prv;
  }
  
  free(pt);
  --( ppd->pn );
}

