#include "simple_logger.h"

#include "hud_menu.h"
#include "inventory.h"

void inventory_init(Inventory *inventory)
{
	if (!inventory) return;

	inventory->itemList = gfc_list_new();
}

void inventory_close(Inventory *inventory)
{
	if (!inventory) return;

	gfc_list_foreach(inventory->itemList, (gfc_work_func*)item_free);
	gfc_list_delete(inventory->itemList);
	inventory->itemList = NULL;
}

Item *inventory_get_item_by_name(Inventory *inventory, const char *name)
{
	int i, c;
	Item *item;

	if (!inventory) return NULL;

	c = gfc_list_get_count(inventory->itemList);
	for (i = 0; i < c; i++)
	{
		item = gfc_list_get_nth(inventory->itemList, i);
		if (!item) continue;
		if (gfc_strlcmp(item->name, name) == 0) return item; // found it
	}
	slog("no item found by name '%s'", name);
	return NULL;
}

Uint8 inventory_add_item(Inventory *inventory, const char *name)
{
	Item *item;
	Window *win;

	if ((!inventory) || (!name)) return 0;

	win = window_find_by_name("hud_menu");

	// check if item is already in inventory
	item = inventory_get_item_by_name(inventory, name);
	if (item)
	{
		if (item->count >= item->max) return 0; 
		item->count++;

		if (gfc_strlcmp(item->name, "health") == 0)
		{
			hud_menu_update_inv_health(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "armor") == 0)
		{
			hud_menu_update_inv_armor(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "damage") == 0)
		{
			hud_menu_update_inv_damage(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "speed") == 0)
		{
			hud_menu_update_inv_speed(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "heat") == 0)
		{
			hud_menu_update_inv_heat(win, item->count);
		}

		return 1;
	}

	// item not found, create it
	item = item_new(name);
	if (!item) return 0;
	gfc_list_append(inventory->itemList, item);

	if (gfc_strlcmp(item->name, "health") == 0)
	{
		hud_menu_update_inv_health(win, item->count);
	}
	else if (gfc_strlcmp(item->name, "armor") == 0)
	{
		hud_menu_update_inv_armor(win, item->count);
	}
	else if (gfc_strlcmp(item->name, "damage") == 0)
	{
		hud_menu_update_inv_damage(win, item->count);
	}
	else if (gfc_strlcmp(item->name, "speed") == 0)
	{
		hud_menu_update_inv_speed(win, item->count);
	}
	else if (gfc_strlcmp(item->name, "heat") == 0)
	{
		hud_menu_update_inv_heat(win, item->count);
	}

	return 1;
}

void inventory_remove_item(Inventory *inventory, const char *name)
{
	Item *item;
	Window *win;

	if ((!inventory) || (!name)) return;

	item = inventory_get_item_by_name(inventory, name);
	win = window_find_by_name("hud_menu");

	if (item)
	{
		item->count--;

		if (gfc_strlcmp(item->name, "health") == 0)
		{
			hud_menu_update_inv_health(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "armor") == 0)
		{
			hud_menu_update_inv_armor(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "damage") == 0)
		{
			hud_menu_update_inv_damage(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "speed") == 0)
		{
			hud_menu_update_inv_speed(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "heat") == 0)
		{
			hud_menu_update_inv_heat(win, item->count);
		}
	}

	if (item && item->count == 0)
	{
		if (gfc_list_delete_data(inventory->itemList, item))
		{
			slog("failed delete of item %s", item);
			return;
		}
		item_free(item);
	}
	return;
}

void inventory_purge_data(const char *filename)
{
	SJson *test, *json, *djson;

	if (!filename) return;
	test = sj_load(filename);
	if (test)
	{
		if (!sj_object_get_value(test, "inventory_data"))
		{
			sj_free(test);
			slog("cannot purge '%s': not an inventory_data file", filename);
			return;
		}
	}

	json = sj_object_new();
	djson = sj_array_new();

	sj_object_insert(json, "inventory_data", djson);
	sj_save(json, filename);
	free(json);
	free(test);
}

void inventory_save_data(Inventory *inventory, const char *filename)
{
	int i, c;
	SJson *json, *djson, *ijson, *temp;
	Item *item;

	if ((!inventory) || (!filename)) return;

	json = sj_object_new();
	djson = sj_array_new();

	c = gfc_list_get_count(inventory->itemList);
	for (i = 0; i < c; i++)
	{
		item = gfc_list_get_nth(inventory->itemList, i);
		if (!item) continue;

		ijson = sj_object_new();
		temp = sj_new_str(item->name);
		sj_object_insert(ijson, "name", temp);
		temp = sj_new_int(item->count);
		sj_object_insert(ijson, "count", temp);
		sj_array_append(djson, ijson);
	}

	sj_object_insert(json, "inventory_data", djson);
	sj_save(json, filename);
	free(json);
}

void inventory_load_data(Inventory *inventory, const char *filename)
{
	int i, c;
	SJson *json, *djson, *ijson;
	Window *win;

	if ((!inventory) || (!filename)) return;

	win = window_find_by_name("hud_menu");

	json = sj_load(filename);
	if (!json)
	{
		slog("failed to load inventory data");
		return;
	}
	djson = sj_object_get_value(json, "inventory_data");
	if (!djson)
	{
		slog("failed to get inventory_data object in file '%s'", filename);
		return;
	}

	c = sj_array_get_count(djson);
	for (i = 0; i < c; i++)
	{
		const char *name;
		int j, count;

		ijson = sj_array_get_nth(djson, i);
		if (!ijson) continue;

		name = sj_object_get_value_as_string(ijson, "name");
		if (!name)
		{
			slog("failed to get name object in file '%s'", filename);
			continue;
		}

		if (!sj_object_get_value_as_int(ijson, "count", &count))
		{
			slog("failed to get count object in file '%s'", filename);
			continue;
		}

		for (j = 0; j < count; j++) inventory_add_item(inventory, name);

		if (gfc_strlcmp(name, "health") == 0)
		{
			hud_menu_update_inv_health(win, count);
		}
		else if (gfc_strlcmp(name, "armor") == 0)
		{
			hud_menu_update_inv_armor(win, count);
		}
		else if (gfc_strlcmp(name, "damage") == 0)
		{
			hud_menu_update_inv_damage(win, count);
		}
		else if (gfc_strlcmp(name, "speed") == 0)
		{
			hud_menu_update_inv_speed(win, count);
		}
		else if (gfc_strlcmp(name, "heat") == 0)
		{
			hud_menu_update_inv_heat(win, count);
		}
	}
}