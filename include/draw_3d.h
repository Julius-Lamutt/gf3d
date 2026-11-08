#ifndef __DRAW_3D_H__
#define __DRAW_3D_H__

#include "gfc_primitives.h"

#include "model.h"

void draw_3d_init();

void draw_3d_cube(GFC_Box cube);

void draw_3d_sphere(GFC_Sphere sphere);

#endif
