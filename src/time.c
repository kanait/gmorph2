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

static int time_last_valid;
static double time_last_real;
static double time_last_user;
static double time_last_sys;

void time_invalidate_last(void)
{
  time_last_valid = 0;
}

int time_last_processed(double *real_sec, double *user_sec, double *sys_sec)
{
  if (!time_last_valid)
    return 0;
  if (real_sec != NULL)
    *real_sec = time_last_real;
  if (user_sec != NULL)
    *user_sec = time_last_user;
  if (sys_sec != NULL)
    *sys_sec = time_last_sys;
  return 1;
}

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
  time_last_real = realtime;
  time_last_user = usertime;
  time_last_sys = systime;
  time_last_valid = 1;
  display("processed time: \n");
  display("\treal:\t%.2f (s)\n", realtime);
  display("\tuser:\t%.2f (s)\n", usertime);
  display("\tsys: \t%.2f (s)\n", systime);
}
