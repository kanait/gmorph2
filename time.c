/* Copyright 1997 by Takashi KANAI */
#include <stdio.h>
#include <sys/types.h>
#include <sys/times.h>
/* #include "tdef.h" */
#include "motif.h"
#include "gldef.h"
#include "smd.h"

/* struct tms { */
/*   time_t tms_utime; */
/*   time_t tms_stime; */
/*   time_t tms_cutime; */
/*   time_t tms_cstime; */
/* }; */

/* long  times(); */
#define TICKS 60.

static struct tms tbuf1;
static long real1;

void time_start(void)
{				/* start timer */
  real1 = times(&tbuf1);
}

void time_stop(void)
{				/* stop timer */
  struct tms tbuf2;
  long  real2;
  double realtime, usertime, systime;

  real2 = times(&tbuf2);
  realtime = (real2 - real1) / TICKS;
  usertime = (tbuf2.tms_utime - tbuf1.tms_utime) / TICKS;
  systime  = (tbuf2.tms_stime - tbuf1.tms_stime) / TICKS;
  display("processed time: \n");
  display("\treal:\t%.2f (s)\n", realtime);
  display("\tuser:\t%.2f (s)\n", usertime);
  display("\tsys: \t%.2f (s)\n", systime);
}
