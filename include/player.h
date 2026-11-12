#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

/**
* @brief spawn a player entity
* @param position the position of the new player
*/
Entity *player_spawn(GFC_Vector3D position);

/*
* @brief damage a player
* @param self the player to damage
* @param damage the amount of damage to deal
*/
void player_damage(Entity *self, Uint32 damage);

#endif
