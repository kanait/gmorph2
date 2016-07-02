/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

static String fres[] = {
  /* fonts */
  "*XmScrolledList*fontList:	-adobe-helvetica-bold-r-normal--12-*",
  "*XmText*fontList:            -adobe-helvetica-bold-r-normal--12-*",
  "*XmButton*fontList:          -adobe-helvetica-bold-r-normal--12-*",
  "*XmLabel*fontList:           -adobe-times-*-r-normal--14-*",
  "*XmPulldownMenu*fontList:    -adobe-helvetica-bold-r-normal--12-*",
  "*XmCascadeButton*fontList:   -adobe-helvetica-bold-r-normal--12-*",
  "*XmPushButton*fontList:	-adobe-helvetica-bold-r-normal--12-*",
  "*XmPushButtonGadget*fontList: -adobe-helvetica-bold-r-normal--12-*",
  "*XmList*fontList:		-adobe-helvetica-bold-r-normal--12-*",
  "*XmScrolledList*fontList:	-adobe-helvetica-bold-r-normal--12-*",
  "*XmToggleButton*fontList:	-adobe-helvetica-bold-r-normal--12-*",
  /* miscs */
  "*XmToggleButton*marginHeight: 0",
  "*XmToggleButton*marginTop:	 0",
  "*XmToggleButton*marginBottom: 0",
  "*XmToggleButton*marginLeft:	0",
  "*XmPushButton*marginHeight:	0",
  "*XmPushButton*marginTop:	0",
  "*XmPushButton*marginBottom:	0",
  "*XmPushButton*marginLeft:	0",
  "*XmPushButton*alignment:	ALIGNMENT_BEGINNING",
  "*XmFrame*shadowType:		SHADOW_ETCHED_IN",
  "*dialogStyle, 		DIALOG_FULL_APPLICATION_MODAL",
  "*sgiMode:                    True",
  "*useSchemes:			Colors",
  "*useEnhancedFSB:		True",
  NULL
};

static char version[] = {
  "GMorph Version 2.0 (beta release 8) Copyright 1997-1998 by Takashi Kanai."
};

char versionshort[] = "GMorph Version 2.0b8";

Swin *swin;

void main( int argc, char *argv[] )
{
  int    i, j, n, in;
  Arg    args[10];
  int    mdiv;
  Widget progress;
  char   file[3][BUFSIZ], rec[BUFSIZ], gppdfile[BUFSIZ], inppdfile[2][BUFSIZ];
  char   ogmhfile[BUFSIZ];
  int    fileflag, guiflag, printflag, recflag, helpflag;
  int    smoothflag, gppdflag, inppdflag;
  int    enh_dispflag, savehmflag, saveimgflag;
  int    spathflag;
  double sublength;
  HPpd   *hppd;
  Sppd   *ppd1, *ppd2, *mppd;
  Swin *create_swin( void );
  void free_swin( Swin * );
  void init_gl(void);
  void initsubwindows(Widget);
  Sppd *open_ppd(char *);
  Sppd *ppdgmorph_v2( HPpd * );
  void free_ppd(Sppd *);
  void printppd(Sppd *);
  void record_morphppd(char *, Sppd *);
  HPpd *open_gmh_file( char * );
  void ppdtocmesh( HPpd * );
  void write_ppd_file(char *, Sppd *);
  void set_windowtitle( char * );
  void make_shortest_path( HPpd * );
  void SGShortestPathsCreate( HPpd * );
  void write_gmh_file( char *, HPpd * );

/*   printf("(SGraph) %d\n", sizeof(SGraph)); */
/*   printf("(Spvt) %d\n", sizeof(Spvt)); */
/*   printf("(SGvt+SGvt_org) %d\n", sizeof(SGvt)+sizeof(SGvt_org)); */
/*   printf("(SGvt+SGvt_steiner) %d\n", sizeof(SGvt)+sizeof(SGvt_steiner)); */
/*   printf("(Sped) %d\n", sizeof(Sped)); */
/*   printf("(SGed) %d\n", sizeof(SGed)); */
  /* option */
  fileflag   = 0;
  spathflag  = SMD_OFF;
  guiflag    = SMD_OFF;
  printflag  = SMD_OFF;
  recflag    = SMD_OFF;
  gppdflag   = SMD_OFF;
  inppdflag  = SMD_OFF;
  helpflag   = SMD_OFF;
  smoothflag = SMD_OFF;
  savehmflag = SMD_OFF;
  saveimgflag = SMD_OFF;
  enh_dispflag = SMD_OFF;
  strcpy(rec, "poi");

  mdiv = 10;
  sublength = 0.01;
  i = 1; j = 0; 
  while( i < argc ) {
    if ( !strcmp(argv[i], "-gui") ) {
      guiflag = SMD_ON;
      ++i;
    } else if ( !strcmp(argv[i], "-print") ) {
      printflag = SMD_ON;
      ++i;
    } else if ( !strcmp(argv[i], "-spath") ) {
      spathflag = SMD_ON;
      ++i;
      sublength = atof(argv[i]);
      ++i;
      strcpy( ogmhfile, argv[i] );
      ++i;
    } else if ( !strcmp(argv[i], "-div") ) {
      ++i;
      mdiv = atoi(argv[i]);
      ++i;
    } else if ( !strcmp(argv[i], "-smooth") ) {
      smoothflag = SMD_ON;
      ++i;
    } else if ( !strcmp(argv[i], "-enh_disp") ) {
      enh_dispflag = SMD_ON;
      ++i;
    } else if ( !strcmp(argv[i], "-hmtops") ) {
      savehmflag = SMD_ON;
      ++i;
    } else if ( !strcmp(argv[i], "-mphtosgi") ) {
      saveimgflag = SMD_ON;
      ++i;
    } else if ( !strcmp(argv[i], "-rec") ) {
      recflag = SMD_ON;
      ++i;
      strcpy( rec, argv[i] );
      ++i;
    } else if ( !strcmp(argv[i], "-i") ) {
      inppdflag = SMD_ON;
      ++i;
      in = 0;
      while ( (in < 2) && (argv[i][0] != '-' ) ) {
	strcpy( inppdfile[in], argv[i] );
	++i; ++in;
      }
    } else if ( !strcmp(argv[i], "-m") ) {
      gppdflag = SMD_ON;
      ++i;
      strcpy( gppdfile, argv[i] );
      ++i;
    } else {
      if ( fileflag > 2 ) {
	helpflag = SMD_ON;
	break;
      }
      strcpy( file[j], argv[i] );
      ++i; ++j;
      ++fileflag;
    }
  }

  if ( fileflag < 2 ) helpflag = SMD_ON;
  if ( spathflag ) {
    guiflag = SMD_OFF;
    if ( fileflag == 1 ) {
      helpflag = SMD_OFF;
    }
  }
  if ( guiflag ) helpflag = SMD_OFF;
  if ( inppdflag ) {
    if ( in < 2 ) helpflag = SMD_ON;
    if ( fileflag ) helpflag = SMD_ON;
  }
  if ( gppdflag ) helpflag = SMD_OFF;
  
  if ( helpflag ) {
    fprintf(stderr, "%s\n", version);
    fprintf(stderr, "usage: \n\t %s (in.gmh) (out.ppd) \n\t\t[-i src.ppd target.ppd] \n\t\t[-m in.ppd]\n\t\t[-rec out]\n\t\t[-div val]\n\t\t[-gui]\n\t\t[-print]\n\t\t[-smooth]\n\t\t[-enh_disp]\n\t\t[-hmtops]\n\t\t[-spath sublength out.gmh]\n", argv[0]);
    fprintf(stderr, "options: \n");
    fprintf(stderr, "\t -i        : input src and target ppd file. \n");
    fprintf(stderr, "\t -m        : input morphing ppd file. \n");
    fprintf(stderr, "\t -rec      : recording morphing results. \n\t\t- output: continuous ppd files (default filename: poi) .\n");
    fprintf(stderr, "\t -div      : morphing division number (default: 10).\n");
    fprintf(stderr, "\t -gui      : display window.\n");
    fprintf(stderr, "\t -print    : print display mode.\n");
    fprintf(stderr, "\t -smooth   : smooth shading mode.\n");
    fprintf(stderr, "\t -enh_disp : enhanced display mode.\n");
    fprintf(stderr, "\t -hmtops   : harmonic map to postscript file.\n");
    fprintf(stderr, "\t -mphtosgi : morphing results to SGI image files.\n");
    fprintf(stderr, "\t -spath    : making shortest-path mode .\n\t\t- You can use this option *ONLY* in command-line mode. \n\t\t- You *MUST* specify sublength (default: 0.01) and out.gmh.\n");
    exit(1);
  }

  swin = create_swin();

  swin->prn  = printflag;
  swin->mdiv = mdiv;
  swin->smooth_shading = smoothflag;
  swin->ext_display = enh_dispflag;
  swin->savehmtops = savehmflag;
  swin->saveimg    = saveimgflag;
  swin->sublength  = sublength;
  
  if ( fileflag ) {

    swin->hppd = open_gmh_file( file[0] );

    if ( swin->hppd == (HPpd *) NULL ) {
      
      fprintf(stderr, "GMorph: Can't read %s. Exit ...\n", file[0]);
      free_swin( swin );
      
      exit(1);
    }
    
    if ( swin->hppd->ppd1 != (Sppd *) NULL ) {
      swin->screenatr[0].current_ppd = swin->hppd->ppd1;
      swin->screenatr[0].view_ppd    = swin->hppd->ppd1;
    }
    if ( swin->hppd->ppd2 != (Sppd *) NULL ) {
      swin->screenatr[1].current_ppd = swin->hppd->ppd2;
      swin->screenatr[1].view_ppd    = swin->hppd->ppd2;
    }
    
  } 

  if ( inppdflag ) {
    if ( inppdfile[0] ) {
      ppd1 = open_ppd( inppdfile[0] );
      swin->screenatr[0].current_ppd = ppd1;
      swin->screenatr[0].view_ppd    = ppd1;
    }
    if ( inppdfile[1] ) {
      ppd2 = open_ppd( inppdfile[1] );
      swin->screenatr[1].current_ppd = ppd2;
      swin->screenatr[1].view_ppd    = ppd2;
    }
  }

  /* for only making shortest-path */
  if ( spathflag ) {

    fprintf(stderr, "spath mode ... ");
    display("\n");
    SGShortestPathsCreate( swin->hppd );
/*     make_shortest_path ( swin->hppd ); */
    fprintf(stderr, "done.\n");
    display("write to %s ... ", ogmhfile );
    write_gmh_file( ogmhfile, swin->hppd );
    display("done.\n");
    free_swin( swin );
    exit(0);
  }
    
  if ( gppdflag ) {
    swin->morph_ppd = open_ppd( gppdfile );
  }
  
  /* GUI open */
  if ( guiflag ) {
    n = 0;
    XtSetArg(args[n], XmNtitle, version); ++n;

    swin->toplevel = XtAppInitialize(&(swin->apc), "SMD", NULL, 0, &argc,
				     argv, fres, (ArgList) args, n);
    initsubwindows(swin->toplevel);

    if ( fileflag ) {
      set_windowtitle( file[0] );
    }
    
    XtRealizeWidget(swin->toplevel);
    XtAppMainLoop(swin->apc);
  } else {

    if ( (gppdflag) && (recflag) ) {
      record_morphppd( rec, swin->morph_ppd );
      
      free_swin( swin );

      exit(1);
    }
    
    hppd = swin->hppd;

    if ( !hppd ) {
      display(" Error: hppd is not found.\n");
      
      free_swin( swin );

      exit(1);
    }
    if ( hppd->vn < 3 ) {
      
      display(" Error: 4 hvertices must be needed.\n");
      free_swin( swin );
      
      exit(1);
    }

    display("\n");
    mppd = hppd->gppd = ppdgmorph_v2( swin->hppd );
    if ( mppd == (Sppd *) NULL ) {
      display(" Error: Can't create morph ppd.\n");
      free_swin( swin );

      exit(1);
    }
    swin->morph_ppd = mppd;

    if ( file[1] ) {
      write_ppd_file( file[1], mppd );
    }
    
    if (recflag) {
      record_morphppd( rec, mppd );
    }
    
    free_swin( swin );
    
  }
}
