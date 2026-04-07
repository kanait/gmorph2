/* Rendering-related structs without requiring OpenGL headers.
 *
 * Phase 3: allows building a pure CLI target without linking OpenGL/X11/Qt.
 */
#ifndef GMORPH_RENDER_TYPES_H
#define GMORPH_RENDER_TYPES_H

typedef struct _material {
  float ambient[4];
  float diffuse[4];
  float emission[4];
  float specular[4];
  float shininess[1];
} Material;

typedef struct _light {
  int   enable;
  float ambient[4];
  float diffuse[4];
  float specular[4];
  float position[4];
  float lmodel_ambient[4];
  float local_view[1];
} Light;

#endif /* GMORPH_RENDER_TYPES_H */

