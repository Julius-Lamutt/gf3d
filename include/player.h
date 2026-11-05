#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

/**
* @brief spawn a player entity
* @param position the position of the new player
*/
Entity *player_spawn(GFC_Vector3D position);

#endif
