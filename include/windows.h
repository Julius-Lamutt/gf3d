#ifndef __WINDOWS_H__
#define __WINDOWS_H__

#include <SDL.h>

#include "gfc_list.h"
#include "gfc_shape.h"
#include "gfc_text.h"

#include "gf2d_sprite.h"

typedef struct Window_S
{
	Uint8			_inuse;			/*<no touchy*/
	Uint8			hidden;			/*<no drawing or updating if true*/
	GFC_TextLine	name;			/*<window name, should be unique*/
	GFC_List		*elements;		/*<list of elements that should be updated*/
	Sprite			*background;	/*<window background*/
	Sprite			*border;		/*<window border*/
	GFC_Rect		size;			/*<where on the screen to draw window*/
	GFC_Rect		canvas;			/*<where on the window to draw elements*/
	GFC_Color		bgColor;		/*<what color the to draw the background in*/
	GFC_Color		borderColor;	/*<what color to draw the border in*/
	struct Window_S *parent;		/*<pointer to parent window*/
	struct Window_S *child;			/*<pointer to child window*/

	void (*close_child)(struct Window_S *win, struct Window_S *child);
	int (*update)(struct Window_S *win, GFC_List *update_elements);
	int (*draw)(struct Window_S *win);
	int (*free)(struct Window_S *win);
	void *data;
} Window;

/*
* @brief this initializes the window management system and queues up cleaning on exit
* @param max: the maximum number of windows that can exist at the same time
*/
void window_system_init(Uint32 max);

/*
* @breif cleans up all active windows
* @param ignore: do not clean this window
*/
void window_clear_all(Window *ignore);

/*
* @brief get a blank window for use
* @returns NULL on no more room or error, a blank window otherwise
*/
Window *window_new();

/**
* @brief clean up a window and free its spot for future use
* @param win: the window to free
*/
void window_free(Window *win);

/*
* @brief run the update function for all active windows
*/
void window_system_update();

/*
* @brief draw all active windows
*/
void window_system_draw();

/*
* @brief load a parent window from a config file
* @param filename: the name of the window file to load
* @return NULL on error, a pointer to window otherwise
*/
Window *window_load(const char *filename);

/*
* @brief find a window by name
* @param name: name of the window
* @return NULL if not found, a pointer to a window otherwise
*/
Window *window_find_by_name(const char *name);

#endif

