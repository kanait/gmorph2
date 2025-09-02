RM = /bin/rm -f
CP = /bin/cp
XLIBS = -lX11 -lXext -lXt -lXi -lSM -lICE
LDLIBS = -lXpm -lGLw -lGLU -L/usr/lib -lGL -lXm -L/usr/X11/lib -lm $(XLIBS)
IMAGE = gmorph2b8
                     CC = gcc
                ROOTDIR = ..
                 SRCDIR = .
              LDOPTIONS = -O3
                 CFLAGS = -O3 -funroll-loops -ansi -pedantic -ffast-math -D_DEFAULT_SOURCE -DSHM -I/usr/include/X11
                DATADIR = $(SRCDIR)/data
                   OBJS = $(SRCDIR)/main.o \
                          $(SRCDIR)/smd.o \
		          $(SRCDIR)/window.o \
			  $(SRCDIR)/gldraw.o \
			  $(SRCDIR)/draw.o \
			  $(SRCDIR)/busy.o \
			  $(SRCDIR)/pick.o \
			  $(SRCDIR)/edit.o \
			  $(SRCDIR)/sellist.o \
			  $(SRCDIR)/file.o \
			  $(SRCDIR)/screen.o \
			  $(SRCDIR)/ppd.o \
			  $(SRCDIR)/ppdsolid.o \
			  $(SRCDIR)/ppdpart.o \
			  $(SRCDIR)/ppdface.o \
			  $(SRCDIR)/ppdloop.o \
			  $(SRCDIR)/ppdvertex.o \
			  $(SRCDIR)/ppdnormal.o \
			  $(SRCDIR)/ppdedge.o \
			  $(SRCDIR)/ppdlist.o \
			  $(SRCDIR)/oppd.o \
			  $(SRCDIR)/prique.o \
			  $(SRCDIR)/sgprique.o \
			  $(SRCDIR)/quadtree.o \
			  $(SRCDIR)/loop.o \
			  $(SRCDIR)/gmorph.o \
			  $(SRCDIR)/grouping.o \
			  $(SRCDIR)/harmonic.o \
			  $(SRCDIR)/correspond.o \
			  $(SRCDIR)/morphvec.o \
			  $(SRCDIR)/paths.o \
			  $(SRCDIR)/sgraph.o \
			  $(SRCDIR)/sgpaths.o \
			  $(SRCDIR)/hppd.o \
			  $(SRCDIR)/hppdlod.o \
			  $(SRCDIR)/hgppd.o \
			  $(SRCDIR)/hged.o \
			  $(SRCDIR)/glppd.o \
			  $(SRCDIR)/ps.o \
			  $(SRCDIR)/image.o \
			  $(SRCDIR)/sgiimage.o \
			  $(SRCDIR)/linbcg.o \
			  $(SRCDIR)/trisquare.o \
			  $(SRCDIR)/matlib.o \
			  $(SRCDIR)/veclib.o \
			  $(SRCDIR)/Progress.o \
			  $(SRCDIR)/time.o
                   CORE = core

all: $(IMAGE)

$(IMAGE): $(OBJS)
	$(RM) $(IMAGE) $(CORE)
	$(CC) $(LDOPTIONS) -o $(IMAGE) $(SMDOBJS) $(OBJS) $(LDLIBS)

clean:
	$(RM) $(OBJS) $(CORE) $(IMAGE)
