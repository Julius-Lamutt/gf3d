#include "simple_logger.h"

#include "items.h"

static void items_close();

static SJson *_itemJson = NULL;
static SJson *_itemDefs = NULL;

void items_init(const char* filename)
{
	if (!filename)
	{
		slog("no filename provided for item initialization");
		return;
	}
	_itemJson = sj_load(filename);
	if (!_itemJson)
	{
		slog("failed to load the json file for item definition");
		return;
	}
	_itemDefs = sj_object_get_value(_itemJson, "items");
	if (!_itemDefs)
	{
		slog("item definition file %s does not contain 'items' list", filename);
		sj_free(_itemJson);
		_itemJson = NULL;
		return;
	}
	slog("items initialized");
	atexit(items_close);
}

void items_close()
{
	if (_itemJson) sj_free(_itemJson);
	_itemJson = NULL;
	_itemDefs = NULL;
	slog("items closed");
}

SJson *item_get_def_by_name(const char *name)
{
	int i, c;
	SJson *item;
	const char *itemName;

	if (!name)
	{
		slog("item name '%s' could not be found", name);
		return NULL;
	}
	if (!_itemDefs)
	{
		slog("no item definitions loaded");
		return NULL;
	}
	c = sj_array_get_count(_itemDefs);
	for (i = 0; i < c; i++)
	{
		item = sj_array_get_nth(_itemDefs, i);
		if (!item) continue;
		itemName = sj_object_get_value_as_string(item, "name");
		if (!itemName) continue;
		if (gfc_strlcmp(name, itemName) == 0) return item; // found it
	}
	slog("no item found by name '%s'", name);
	return NULL;
}

Item *item_new(const char *name)
{
	Item *item;
	SJson *itemDef, *array;
	const char *display_name, *filename;
	int frame_w, frame_h, frames_per_line, max;
	Sprite* sprite;

	itemDef = item_get_def_by_name(name);
	if (!itemDef) return NULL;

	item = gfc_allocate_array(sizeof(Item), 1);
	if (!item)
	{
		slog("failed to create a new item");
		return NULL;
	}

	display_name = sj_object_get_value_as_string(itemDef, "display_name");
	if (!display_name)
	{
		free(item);
		slog("failed to find display_name object for item '%s'", name);
		return NULL;
	}

	array = sj_object_get_value(itemDef, "sprite");
	if (!array)
	{
		free(item);
		slog("failed to find sprite object for item '%s'", name);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		free(item);
		slog("missing sprite parameters for item '%s'", name);
		return NULL;
	}
	filename = sj_get_string_value(sj_array_get_nth(array, 0));
	if (!filename ||
		!sj_get_integer_value(sj_array_get_nth(array, 1), &frame_w) ||
		!sj_get_integer_value(sj_array_get_nth(array, 2), &frame_h) ||
		!sj_get_integer_value(sj_array_get_nth(array, 3), &frames_per_line))
	{
		free(item);
		slog("one or more sprite parameters are invalid for item '%s'", name);
		return NULL;
	}
	sprite = gf2d_sprite_load(filename, frame_w, frame_h, frames_per_line);

	if (!sj_object_get_value_as_int(itemDef, "max", &max))
	{
		free(item);
		slog("failed to find max object for item '%s'", name);
		return NULL;
	}

	gfc_line_cpy(item->name, name);
	gfc_line_cpy(item->display_name, display_name);
	item->sprite = sprite;
	item->count = 1;
	item->max = max;
	return item;
}

void item_free(Item* item)
{
	if (!item) return;
	if (item->sprite) gf2d_sprite_free(item->sprite);
	free(item);
}