#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "gfc_audio.h"

#include "model.h"

typedef struct
{
	Model			*sky;
	GFC_TextLine	skyFile;
	Model			*terrain;
	GFC_TextLine	terrainFile;
	GFC_Box			bounds;
	Mix_Music		*bgMusic;
	SJson			*json;
} Level;

/**
* @brief create a level
* @returns a level
*/
Level *level_new();

/**
* @brief load a level from file
* @param filename the file to load the level from
* @returns a level
*/
Level *level_load(const char *filename);

/**
* @brief free a level
* param level the level to free
*/
void level_free(Level *level);

/**
* @brief draw a level
* @param level the level to draw
*/
void level_draw(Level *level);

/**
* @brief use a line test to determine if an object is touching the level
* @param level the level to test on
* @param edge the edge
* @param contact the point of contact
* @returns 1 if collision, 0 otherwise
*/
Uint8 level_line_test(Level *level, GFC_Edge3D edge, GFC_Vector3D *contact);

/*
* @brief determine if there is a collision with the level's bounds
* @param level the level to check the bounds for
* @param aabb the bounding box to check
* @returns 1 if there is a collision, 0 otherwise
*/
Uint8 level_bounds_test(Level *level, GFC_Box aabb);

#endif

