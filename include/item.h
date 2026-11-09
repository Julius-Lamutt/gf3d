#ifndef __ITEM_H__
#define __ITEM_H__

#include "gfc_vector.h"

#include "entity.h"

/**
* @brief spawn an item entity
* @param position the position of the new item
*/
Entity *item_spawn(const char *itemName, GFC_Vector3D position);

#endif