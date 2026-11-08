#ifndef __DRAW_3D_H__
#define __DRAW_3D_H__

#include "gfc_primitives.h"

#include "model.h"

/**
* @brief initialize the 3d draw system for primtives
*/
void draw_3d_init();

/**
* @brief render a cube using a gfc_box
* @param cube the gfc_box to render
* @param color cube color
*/
void draw_3d_cube(GFC_Box cube, GFC_Color color);

/**
* @brief render the outline of a cube using a gfc_box
* @param cube the gfc_box to outline
* @param color outline color
*/
void draw_3d_cube_outline(GFC_Box cube, GFC_Color color);

/**
* @brief render a sphere using gfc_sphere
* @param sphere the gfc_sphere to render
* @param color sphere color
*/
void draw_3d_sphere(GFC_Sphere sphere, GFC_Color color);

/**
* @brief render the outline of a sphere using gfc_sphere
* @param sphere the gfc_sphere to outline
* @param color outline color
*/
void draw_3d_sphere_outline(GFC_Sphere sphere, GFC_Color color);

#endif
