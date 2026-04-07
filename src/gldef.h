/* Copyright (c) 1997 Takashi Kanai; All rights reserved. */

#ifndef GMORPH_ENABLE_RENDERING
#define GMORPH_ENABLE_RENDERING 1
#endif

#include "render_types.h"

#if GMORPH_ENABLE_RENDERING
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif
