/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#define xmstrmk(x)	XmStringCreateLtoR((x), XmSTRING_DEFAULT_CHARSET)

typedef struct _mb {
  Widget   Button;
  char *Label;
  int  arg;
  char *RN;
  void  (*CB) ();
  char *accel;
  char *acceltext;
} Mb;

typedef struct _pmcb {
  Widget   CB;
  Widget   PM;
  char *Label;
  char *RNPM;
  char *RNCB;
  Mb   *Buttons;
  int   Nbuttons;
  char  mnemonic;
} Pmcb;




