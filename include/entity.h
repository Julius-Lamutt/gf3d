#ifndef __ENTITY_H__
#define __ENTITY_H__

typedef struct Entity_S
{
	// Basic Entity Data

	Uint8			_inuse;
	Model			*model;
	GFC_Vector3D	position;
	GFC_Vector3D	rotation;
	GFC_Vector3D	scale;
	GFC_Box			bounds;
	GFC_Color		color;
	GFC_Vector3D	velocity;

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

#endif

