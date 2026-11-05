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
	GFC_Vector3D	velocity;
	GFC_Color		colorMod;

	// Entity Functions

	int				(*think)(Entity_S *self);
	int				(*update)(Entity_S *self);
	int				(*draw)(Entity_S *self);
	void			(*free)(Entity_S *self);

	void			*data;						
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
*/
Uint8 entity_move_check(Entity *self);

#endif

