#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"

/**
* @brief spawn a monster
* @param monsterName name of the monster to spawn
* @param position where to spawn the monster
* @return NULL on error, a monster otherwise
*/
Entity *monster_spawn(const char *monsterName, GFC_Vector3D position);

/*
* @brief damage a monster
* @param self the monster to damage
* @param damage the amount of damage to deal
*/
void monster_damage(Entity *self, Uint32 damage);

#endif

