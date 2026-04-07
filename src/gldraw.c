/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include "cinc.h"
#include "motif.h"
#include "gldef.h"
#include "smd.h"

#if defined(__APPLE__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

void definelinestyle(void)
{
  /* 0:  solid line
     1: broken line
     2: dotted line
   */
  glNewList(1, GL_COMPILE);
  glLineStipple((GLint) 2, 0xF0F0);
  glEndList();
  glNewList(2, GL_COMPILE);
  glLineStipple((GLint) 2, 0x8888);
  glEndList();
}

/*********************************************************
                           2D
**********************************************************/

/* draw line */

void drawline(Vec2d *spnt, Vec2d *epnt)
{
  GLfloat v[2];

  glCallList(0);
  glEnable(GL_LINE_STIPPLE);

  glBegin(GL_LINE_STRIP);
  v[0] = (GLfloat) spnt->x;
  v[1] = (GLfloat) spnt->y;
  glVertex2fv(v);
  v[0] = (GLfloat) epnt->x;
  v[1] = (GLfloat) epnt->y;
  glVertex2fv(v);
  glEnd();

  glDisable(GL_LINE_STIPPLE);
}

/* draw broken line */

void drawbrknline(Vec2d *spnt, Vec2d *epnt)
{
  GLfloat v[2];

  glCallList(1);
  glEnable(GL_LINE_STIPPLE);

  glBegin(GL_LINE_STRIP);
  v[0] = (GLfloat) spnt->x;
  v[1] = (GLfloat) spnt->y;
  glVertex2fv(v);
  v[0] = (GLfloat) epnt->x;
  v[1] = (GLfloat) epnt->y;
  glVertex2fv(v);
  glEnd();

  glDisable(GL_LINE_STIPPLE);
}

/* draw circle mark */

void drawmark(Vec2d *pos)
{
  GLUquadricObj *qobj = gluNewQuadric();
  gluQuadricDrawStyle(qobj, GLU_SILHOUETTE);
  glPushMatrix();
  glTranslatef((GLfloat) pos->x, (GLfloat) pos->y, 0.);
  gluDisk(qobj, 0., 2.0, 32, 1);
  glPopMatrix();
  gluDeleteQuadric(qobj);

}

void drawrectangle(Vec2d *pos, double dsize)
{
  Vec2d spnt, epnt;
  void  drawline(Vec2d *, Vec2d *);

  spnt.x = pos->x - dsize;
  spnt.y = pos->y - dsize;
  epnt.x = pos->x + dsize;
  epnt.y = pos->y - dsize;
  drawline(&spnt, &epnt);
  epnt.x = pos->x - dsize;
  epnt.y = pos->y + dsize;
  drawline(&spnt, &epnt);
  epnt.x = pos->x + dsize;
  epnt.y = pos->y + dsize;
  spnt.x = pos->x + dsize;
  spnt.y = pos->y - dsize;
  drawline(&spnt, &epnt);
  spnt.x = pos->x - dsize;
  spnt.y = pos->y + dsize;
  drawline(&spnt, &epnt);
}


/* draw circle filled mark */

void drawfillmark(Vec2d *pos)
{
  /* OGLXXX See gluDisk man page. */
  GLUquadricObj *qobj = gluNewQuadric();
  glPushMatrix();
  glTranslatef((GLfloat) pos->x, (GLfloat) pos->y, 0.0);
  gluDisk(qobj, 0., 5.0, 32, 1);
  glPopMatrix();
  gluDeleteQuadric(qobj);
}

/*****************************************************************
                             3D
******************************************************************/

/* draw 3D line */

void draw3Dline(Vec *spnt, Vec *epnt)
{
  GLfloat v[3];

  glCallList(0);
  glEnable(GL_LINE_STIPPLE);

  glBegin(GL_LINE_STRIP);
  v[0] = (GLfloat) spnt->x;
  v[1] = (GLfloat) spnt->y;
  v[2] = (GLfloat) spnt->z;
  glVertex3fv(v);
  v[0] = (GLfloat) epnt->x;
  v[1] = (GLfloat) epnt->y;
  v[2] = (GLfloat) epnt->z;
  glVertex3fv(v);
  glEnd();

  glDisable(GL_LINE_STIPPLE);
}

/* draw 3D broken line */

void draw3Dbrknline(Vec *spnt, Vec *epnt)
{
  GLfloat v[3];

  /* OGLXXX setlinestyle: Check list numbering. */

  glCallList(1);
  glEnable(GL_LINE_STIPPLE);

  glBegin(GL_LINE_STRIP);
  v[0] = (GLfloat) spnt->x;
  v[1] = (GLfloat) spnt->y;
  v[2] = (GLfloat) spnt->z;
  glVertex3fv(v);
  v[0] = (GLfloat) epnt->x;
  v[1] = (GLfloat) epnt->y;
  v[2] = (GLfloat) epnt->z;
  glVertex3fv(v);
  glEnd();

  glDisable(GL_LINE_STIPPLE);
}

/* draw 3D dotted line */

void draw3Ddotline(Vec *spnt, Vec *epnt)
{
  GLfloat v[3];

  glCallList(2);
  glEnable(GL_LINE_STIPPLE);

  glBegin(GL_LINE_STRIP);
  v[0] = (GLfloat) spnt->x;
  v[1] = (GLfloat) spnt->y;
  v[2] = (GLfloat) spnt->z;
  glVertex3fv(v);
  v[0] = (GLfloat) epnt->x;
  v[1] = (GLfloat) epnt->y;
  v[2] = (GLfloat) epnt->z;
  glVertex3fv(v);
  glEnd();

  glDisable(GL_LINE_STIPPLE);
}

void glu_sphere( Vec *pos, double radius )
{
  GLUquadricObj   *qobj;
  GLint           slices = 20,staks = 10;

  if ((qobj = gluNewQuadric()) != NULL) {
    glPushMatrix();
    glTranslated( (GLdouble) pos->x, (GLdouble) pos->y, (GLdouble) pos->z );
    gluSphere(qobj, (GLdouble) radius, slices, staks);
    glPopMatrix();
    gluDeleteQuadric(qobj);
  }
}

void glu_cylinder( Vec *sv, Vec *ev, double rad )
{
  GLUquadricObj   *cyl;
  GLint           slices = 20,staks = 10;
  Vec sub;
  double height;
  double rx, ry;
  double V3Length( Vec * );
  
  if ((cyl = gluNewQuadric()) != NULL) {
    glPushMatrix();
    sub.x  = ev->x - sv->x;
    sub.y  = ev->y - sv->y;
    sub.z  = ev->z - sv->z;
    height = V3Length( &sub );
    sub.x /= height; sub.y /= height; sub.z /= height;
    
    ry = asin( sub.x );
    rx = acos( sub.z / cos(ry) );

    if ( fabs ( sub.y + sin(rx) * cos(ry) > SMDZEROEPS) ) {
      rx = 2 * SMDPI - rx;
    }
    
    glTranslated ( sv->x, sv->y, sv->z );
    glRotated( rx * 180.0 / SMDPI, 1.0, 0.0, 0.0 );
    glRotated( ry * 180.0 / SMDPI, 0.0, 1.0, 0.0 );
    gluCylinder(cyl, rad, rad, height, slices, staks);
    glPopMatrix();
    gluDeleteQuadric(cyl);
  }
}

  
int makerasterfont(ScreenAtr *screen, char *fontname)
{
  (void) fontname;
  if (screen)
    screen->fontOffset = 0;
  return FAIL;
}

int printstring(ScreenAtr *screen, char *s)
{
  glPushAttrib(GL_LIST_BIT);
  glListBase((GLuint) screen->fontOffset);
  glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *) s);
  glPopAttrib();

  return SUCCEED;
}

void material_binding(Material *material)
{
  glMaterialfv(GL_FRONT, GL_AMBIENT, material->ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material->diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material->specular);
  glMaterialfv(GL_FRONT, GL_EMISSION, material->emission);
  glMaterialfv(GL_FRONT, GL_SHININESS, material->shininess);
}

Material *open_mtl(char *filename, Material *omtl)
{
  FILE     *fp;
  Material *nmtl;
  char     key[BUFSIZ], buf[BUFSIZ], val[5][BUFSIZ];
  
  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "can't open %s.\n", filename);
    return omtl;
  }

  free(omtl);
  nmtl = (Material *) malloc(sizeof(Material));

  while (fgets(buf, BUFSIZ, fp)) {
    sscanf(buf, "%s", key);
    if (comment(buf[0]))  continue;
    else if (!strcmp(key, "ambient")) {
      sscanf(buf, "%s%s%s%s%s", val[0], val[1], val[2], val[3], val[4]);
      nmtl->ambient[0] = (GLfloat) atof(val[1]);
      nmtl->ambient[1] = (GLfloat) atof(val[2]);
      nmtl->ambient[2] = (GLfloat) atof(val[3]);
      nmtl->ambient[3] = (GLfloat) atof(val[4]);
    } else if (!strcmp(key, "diffuse")) {
      sscanf(buf, "%s%s%s%s%s", val[0], val[1], val[2], val[3], val[4]);
      nmtl->diffuse[0] = (GLfloat) atof(val[1]);
      nmtl->diffuse[1] = (GLfloat) atof(val[2]);
      nmtl->diffuse[2] = (GLfloat) atof(val[3]);
      nmtl->diffuse[3] = (GLfloat) atof(val[4]);
    } else if (!strcmp(key, "emission")) {
      sscanf(buf, "%s%s%s%s%s", val[0], val[1], val[2], val[3], val[4]);
      nmtl->emission[0] = (GLfloat) atof(val[1]);
      nmtl->emission[1] = (GLfloat) atof(val[2]);
      nmtl->emission[2] = (GLfloat) atof(val[3]);
      nmtl->emission[3] = (GLfloat) atof(val[4]);
    } else if (!strcmp(key, "specular")) {
      sscanf(buf, "%s%s%s%s%s", val[0], val[1], val[2], val[3], val[4]);
      nmtl->specular[0] = (GLfloat) atof(val[1]);
      nmtl->specular[1] = (GLfloat) atof(val[2]);
      nmtl->specular[2] = (GLfloat) atof(val[3]);
      nmtl->specular[3] = (GLfloat) atof(val[4]);
    } else if (!strcmp(key, "shininess")) {
      sscanf(buf, "%s%s", val[0], val[1]);
      nmtl->shininess[0] = (GLfloat) atof(val[1]);
    }
  }

  return nmtl;
}
    
int mtl_ok(char *str)
{
  void drawwindow(int);
  Material *open_mtl(char *, Material *);
  
  if (swin->opend == SMDOPEN) {
    swin->material = open_mtl(str, swin->material);
  }
  drawwindow(0);
  drawwindow(1);
  return SUCCEED;
}


