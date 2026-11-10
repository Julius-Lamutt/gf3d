#include "simple_logger.h"

#include "gfc_config.h"
#include "gfc_config_def.h"
#include "gfc_text.h"

#include "item.h"

typedef struct
{
	Uint8	_inuse;		/**<just a placeholder for now*/
} ItemData;

static int item_think(Entity *self);
static int item_update(Entity *self);
static int item_draw(Entity *self);
static void item_free(Entity *self);

Entity *item_spawn(const char *itemName, GFC_Vector3D position)
{
	int i, c;
	SJson *json, *itemList, *item;
	Entity *self;
	ItemData *data;
	Model *model;
	GFC_Box bounds;
	GFC_Color color;
	const char *str1, *str2;

	if (!itemName) return NULL;

	json = sj_load("defs/items.json");
	if (!json)
	{
		slog("failed to load item file");
		return NULL;
	}
	itemList = sj_object_get_value(json, "items");
	if (!itemList)
	{
		slog("cannot load item names from item file");
		sj_free(json);
		return NULL;
	}
	c = sj_array_get_count(itemList);
	for (i = 0; i < c; i++)
	{
		item = sj_array_get_nth(itemList, i);
		if (!item) continue;
		str1 = sj_object_get_string(item, "name");
		if (gfc_strlcmp(str1, itemName) == 0) break;
	}
	if (i == c)
	{
		slog("failed to find item '%s' in item file", itemName);
		sj_free(json);
		return NULL;
	}
	str2 = sj_object_get_string(item, "model");
	if (!str2)
	{
		slog("failed to locate item model for item '%s'", itemName);
		sj_free(json);
		return NULL;
	}
	model = model_load(str2);
	if (!model)
	{
		slog("failed to load player model for item '%s'", itemName);
		sj_free(json);
		return NULL;
	}
	bounds = gfc_box_from_config(sj_object_get_value(item, "bounds"));
	if (bounds.w == 0 && bounds.h == 0 && bounds.d == 0)
	{
		slog("failed to load player bounds for item '%s'", itemName);
		sj_free(json);
		return NULL;
	}
	if (!sj_object_get_color_value(item, "color", &color)) color = GFC_COLOR_WHITE;

	self = entity_new();
	if (!self)
	{
		slog("failed to allocate memory for item entity");
		sj_free(json);
		return NULL;
	}
	gfc_line_cpy(self->name, str1);
	self->layer = EL_ITEM;
	self->mask = ITEM_MASK;
	self->model = model;
	self->position = position;
	self->bounds = bounds;

	self->bounds.w = self->bounds.w * self->scale.x;
	self->bounds.h = self->bounds.h * self->scale.y;
	self->bounds.d = self->bounds.d * self->scale.z;

	self->bounds.x = position.x - 0.5 * self->bounds.w;
	self->bounds.y = position.y - 0.5 * self->bounds.h;
	self->bounds.z = position.z - 0.5 * self->bounds.d;

	self->color = color;

	self->think = item_think;
	self->update = item_update;
	self->draw = item_draw;
	self->free = item_free;

	self->data = gfc_allocate_array(sizeof(ItemData), 1);
	if (self->data)
	{
		data = self->data;
		data->_inuse = 1;
	}
	return self;
}

static void item_free(Entity *self)
{
	ItemData *data;

	if ((!self) || (!self->data)) return;
	data = (ItemData*)self->data;
	if (data) free(data);
	self->data = NULL;
}

static int item_think(Entity *self)
{
	GFC_Vector3D move = {0};

	if (!self) return 0;

	// gravity
	//self->velocity.z = -0.125;

	// collision detection
	if (!entity_move_check(self)) self->velocity = gfc_vector3d(0, 0, 0);

	return 1;
}

static int item_update(Entity *self)
{
	if (!self) return 0;

	// update position
	gfc_vector3d_add(self->position, self->position, self->velocity);

	// update bounding box
	self->bounds.x = self->position.x - 0.5 * self->bounds.w;
	self->bounds.y = self->position.y - 0.5 * self->bounds.h;
	self->bounds.z = self->position.z - 0.5 * self->bounds.d;

	return 1;
}

static int item_draw(Entity *self)
{
	if (!self) return 0;

	return 0;
}