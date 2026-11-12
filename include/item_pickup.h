#ifndef __ITEM_PICKUP_H__
#define __ITEM_PICKUP_H__

#include "gfc_vector.h"

#include "entity.h"

/**
* @brief spawn an item pickup entity
* @param itemName name of the item pickup to spawn
* @param position the position of the new item pickup
* @returns NULL on error, an item pickup otherwise
*/
Entity *item_pickup_spawn(const char *itemName, GFC_Vector3D position);

#endif