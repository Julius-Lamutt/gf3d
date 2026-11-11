#ifndef __INVENTORY_H__
#define __INVENTORY_H__

#include "gfc_list.h"

#include "items.h"

typedef enum
{
	II_ITEM1,
	II_ITEM2,
	II_ITEM3,
	II_ITEM4,
	II_ITEM5,
	II_ITEMMAX
} InventoryIndinces;

typedef struct
{
	GFC_List *itemList;
} Inventory;

/*
* @brief intilialize the inventory
* @param inventory the inventory to initialize
*/
void inventory_init(Inventory *inventory);

/*
* @brief free the inventory and all of its items
* @param inventory: the inventory to be freed
*/
void inventory_close(Inventory *inventory);

/*
* @brief get an inventory item by name
* @param inventory: the inventory that is being checked for an item
* @param name: name of the item
* @return NULL if item is not found, a pointer to the item otherwise
*/
Item *inventory_get_item_by_name(Inventory *inventory, const char *name);

/*
* @brief add an item to the inventory
* @param inventory: the inventory where the item will be added
* @param name: the name of the item to be added
*/
void inventory_add_item(Inventory *inventory, const char *name);

/*
* @brief remove an item from the inventory
* @param inventory: the inventory the item will be removed from
* @param name: the name of the item to be removed
*/
void inventory_remove_item(Inventory *inventory, const char *name);

/*
* @brief Purges a file containing inventory data. If file does not contain inventory data,
* the function will short-circuit before any data is purged. NOTE: this is irreversible.
* DO NOT USE WITHOUT DOUBLE CHECKING FILENAME!
* @param filename: name of the inventory file to purge
*/
void inventory_purge_data(const char *filename);

/*
* @brief saves inventory data for later retrieval
* @param inventory: the inventory to save data for
* @param filename: name of the inventory file to save to
*/
void inventory_save_data(Inventory *inventory, const char *filename);

/*
* @brief load inventory data
* @param inventory: the inventory to load data for
* @param filename: name of the inventory file to load from
*/
void inventory_load_data(Inventory *inventory, const char *filename);

#endif

