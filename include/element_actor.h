#ifndef __ELEMENT_ACTOR_H__
#define __ELEMENT_ACTOR_H__

#include "gf2d_sprite.h"

#include "elements.h"

typedef struct
{
	Sprite		*image;			/*<the sprite for the actor*/
	GFC_Color	colorShift;		/*<the color shift for the actor*/
	Uint8		frame;			/*<the frame of the actor to draw*/
} ActorElement;

/*
* @brief load an actor element
* @param windel: the window element to load the actor element from
* @return NULL on error, a pointer to an actor element otherwise
*/
ActorElement *element_actor_load(SJson *windel);

/*
* @brief draws an actor element
* @param actor: the actor element to draw
* @param bounds: the bounds for the actor element
*/
void element_actor_draw(ActorElement *actor, GFC_Rect bounds);

/*
* @brief free an actor element from memory
* @param label: the actor element to be freed
*/
void element_actor_free(ActorElement *actor);

#endif

