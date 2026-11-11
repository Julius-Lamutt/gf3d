#ifndef __ITEMS_H__
#define __ITEMS_H__

#include <SDL.h>

#include "gfc_text.h"

#include "gf2d_sprite.h"

#include "simple_json.h"

typedef struct
{
	GFC_TextLine	name;			/* used for item search by name */
	GFC_TextLine	display_name;	/* used for UI display name */
	Sprite			*sprite;		/* sprite for the item */
	Uint8			count;			/* how many of the item I have */
	Uint8			max;			/* how many of the item I can carry */
} Item;

/*
* @brief intilialize and load item definitions
* @param filename: the name of the json file to load
*/
void items_init(const char *filename);

/*
* @brief get an item definition by its name
* @param name: the name of the item
* @return NULL if not found, otherwise the definition of the item
*/
SJson *item_get_def_by_name(const char *name);

/*
* @brief create a new item
* @param name: the name of the item to create
* @return NULL if not found, otherwise the definition of the item
*/
Item *item_new(const char *name);

/*
* @brief free an item
* @param item: the item to free
*/
void item_free(Item *item);

#endif
