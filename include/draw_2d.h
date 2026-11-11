#ifndef __DRAW_2D_H__
#define __DRAW_2D_H__

#include "gfc_shape.h"

/**
* @brief initialize the 2d draw system for primtives
*/
void draw_2d_init();

/**
* @brief render a rectangle using a gfc_rect
* @param rect the gfc_rect to render
* @param color rect color
*/
void draw_2d_rect(GFC_Rect rect, GFC_Color color);

/**
* @brief render a rectangle outline using a gfc_rect
* @param rect the gfc_rect to render the outline for
* @param color rect outline color
*/
void draw_2d_rect_outline(GFC_Rect rect, GFC_Color color);

#endif
