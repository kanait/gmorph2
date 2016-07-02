/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/GLwMDrawA.h>

/* "Material" structure */

typedef struct _material {
  GLfloat ambient[4];
  GLfloat diffuse[4];
  GLfloat emission[4];
  GLfloat specular[4];
  GLfloat shininess[1];
} Material;

/* "Light" structure */

typedef struct _light {
  int       enable;
  GLfloat   ambient[4];
  GLfloat   diffuse[4];
  GLfloat   specular[4];
  GLfloat   position[4];
  GLfloat   lmodel_ambient[4];
  GLfloat   local_view[1];
} Light;

