/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

PQHeap *create_pqheap( int size )
{
  int    i;
  PQHeap *pqh;

  pqh = (PQHeap *) malloc(sizeof(PQHeap));

  pqh->pqcont = (PQCont *) malloc( size * sizeof(PQCont) );
  
  pqh->size     = size;
  for ( i = 0; i < size; ++i ) pqh->pqcont[i].id = i;
  pqh->size     = size;
  pqh->last     = 0;

  return pqh;
}

void swap_pqcont( PQCont *a, PQCont *b )
{
  double temp_dis;
  Spvt   *temp_vt;

  temp_dis    = a->distance;
  a->distance = b->distance;
  b->distance = temp_dis;

  temp_vt   = a->tgt_vt;
  a->tgt_vt = b->tgt_vt;
  b->tgt_vt = temp_vt;
  
  a->tgt_vt->pqc = a;
  b->tgt_vt->pqc = b;
  
  temp_vt   = a->prv_vt;
  a->prv_vt = b->prv_vt;
  b->prv_vt = temp_vt;
  
}  

int parent_num( int child )
{
  return (int) (child - 1) / 2;
}

void insert_pqcont( double dis, PQHeap *pqh, Spvt *tgt, Spvt *prv)
{
  void adjust_to_parent( int, PQHeap * );
  
  if ( pqh->last >= (pqh->size - 1) ) {
    return;
  }

  pqh->pqcont[pqh->last].distance = dis;
  pqh->pqcont[pqh->last].tgt_vt = tgt;
  pqh->pqcont[pqh->last].prv_vt = prv;
  tgt->pqc = &(pqh->pqcont[pqh->last]);
  tgt->pq_type = EXIST;
  
  /*   display(" dis = %g\n", pqh->pqcont[0].distance); */
							  
  adjust_to_parent( pqh->last, pqh );

  ++(pqh->last);

}

/* parent direction process */
void adjust_to_parent( int id, PQHeap *pqh )
{
  int  p, c;
  int  parent_num(int);
  void swap_pqcont(PQCont *, PQCont *);

  c = id;

  while ( c > 0 ) {
/*     display("ddda\n"); */
    p = parent_num( c );
/*     display("atp c %d -> p %d all %d\n", c, p, pqh->last); */
/*     display(" j = %d\n", j); */
    if ( pqh->pqcont[p].distance > pqh->pqcont[c].distance ) {
/*       display("dddb\n"); */
      swap_pqcont( &(pqh->pqcont[c]), &(pqh->pqcont[p]) );
/*       display("dddc\n"); */
      c = p;
    } else
      return;
  }
}

/* child direction process */
void adjust_to_child( int id, PQHeap *pqh )
{
  int  p, c, i2l, i2r;
  int  parent_num( int );
  void swap_pqcont(PQCont *, PQCont *);

  p = id;
  while ( p <= parent_num(pqh->last-1) ) {

    /* child: i2l, i2r */
    i2l = 2*p+1; i2r = i2l+1;
    if ( i2r == pqh->last ) { /* pqh->last does not exist. */
      c = i2l;
    } else if ( pqh->pqcont[i2l].distance < pqh->pqcont[i2r].distance ) {
      c = i2l;
    } else {
      c = i2r;
    }
/*     display("atc p %d -> c %d all %d\n", p, c, pqh->last-1); */
    
    if ( pqh->pqcont[p].distance > pqh->pqcont[c].distance ) {
      swap_pqcont( &(pqh->pqcont[p]), &(pqh->pqcont[c]) );
    } else return;
    
    p = c;
    
  }
}

void adjust_pqheap( Id id, PQHeap *pqh )
{
  void adjust_to_parent( int, PQHeap * );
  void adjust_to_child( int, PQHeap * );
  
  adjust_to_parent( id, pqh );
/*   adjust_to_child( id, pqh ); */
}

void deletemin_pqcont( PQHeap *pqh )
{
  int  i;
  void adjust_to_child( int, PQHeap * );
  
  /* empty */
  if ( pqh->last == 0 ) {
    return;
  }
  

  /* delete mininum (root) */
  pqh->pqcont[0].tgt_vt->pq_type = DELETED;
  pqh->pqcont[0].tgt_vt->pqc     = (PQCont *) NULL;

  pqh->pqcont[0].distance = pqh->pqcont[pqh->last-1].distance;
  pqh->pqcont[0].tgt_vt   = pqh->pqcont[pqh->last-1].tgt_vt;
  pqh->pqcont[0].prv_vt   = pqh->pqcont[pqh->last-1].prv_vt;

  pqh->pqcont[0].tgt_vt->pqc = &(pqh->pqcont[0]);

  --(pqh->last);

  i = 0;
  adjust_to_child( i, pqh );
}

void free_pqheap ( PQHeap *pqh )
{
  free( pqh->pqcont );
  free( pqh );
}


