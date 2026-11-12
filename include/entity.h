#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "model.h"
#include "level.h"

#define NO_MASK			(EL_NONE)
#define PLAYER_MASK		(EL_MONSTER|EL_ITEM|EL_PROJECTILE|EL_WORLD)
#define MONSTER_MASK	(EL_PLAYER|EL_MONSTER|EL_PROJECTILE|EL_WORLD)
#define ITEM_MASK		(EL_PLAYER|EL_WORLD)
#define PROJECTILE_MASK (EL_PLAYER|EL_MONSTER|EL_WORLD)
#define WORLD_MASK		(EL_PLAYER|EL_MONSTER|EL_PROJECTILE)
#define ALL_MASK		(EL_ALL)

#define PLAYER_CLIP		(EL_PLAYER)
#define MONSTER_CLIP    (EL_MONSTER)

typedef enum
{
	EL_NONE = 0,
	EL_PLAYER = 1,
	EL_MONSTER = 2,
	EL_ITEM = 4,
	EL_PROJECTILE = 8,
	EL_WORLD = 16,
	EL_ALL = 31
} EntityLayers;

typedef struct Entity_S
{
	// Basic Entity Data

	Uint8			_inuse;		/**<no touchy*/
	GFC_TextLine	name;
	Uint8			layer;
	Model			*model;
	GFC_Color		color;

	// Physics Data

	Uint8			mask;
	GFC_Box			bounds;
	GFC_Vector3D	position;
	GFC_Vector3D	rotation;
	GFC_Vector3D	scale;
	GFC_Vector3D	velocity;
	GFC_List		*touches;	/**<entities touched this frame*/

	// Entity Functions

	int		(*think)(struct Entity_S *self);
	int		(*update)(struct Entity_S *self);
	int		(*draw)(struct Entity_S *self);
	void	(*free)(struct Entity_S *self);

	void	*data;						
} Entity;

/**
* @brief initialize the entity system
* @param entityMax the max number of entities
*/
void entity_system_init(Uint32 entMax);

/**
* @brief spawn a blank entity
* @returns a blank entity, NULL otheriwse
*/
Entity *entity_new();

/**
* @brief cleanup an entity when it is no longer needed
* @param ent the ent to free
*/
void entity_free(Entity *self);

/**
* @brief set the current level for all entities
* @param level the level to set
*/
void entity_system_set_level(Level *level);

/**
* @brief run the think functions for all active entities
*/
void entity_system_think();

/**
* @brief run the update functions for all active entities
*/
void entity_system_update();

/**
* @brief draw all active entities
*/
void entity_system_draw();

/**
* @brief check if entity can move
* @param self the entity to check
* @returns 1 if entity can move, 0 otherwise
*/
Uint8 entity_move_check(Entity *self);

/**
* @brief get the entities touched this frame
* @param self the entity to find touches for
*/
void entity_get_touches(Entity *self);

/**
* @brief get the closest monster entity
* @param self the entity to find the closest monster entity for
*/
Entity *entity_get_closest_monster(Entity *self);

#endif