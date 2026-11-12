#include "simple_logger.h"

#include "gfc_text.h"
#include "gfc_input.h"
#include "gfc_config.h"

#include "gf3d_vgraphics.h"

#include "hud_menu.h"
#include "inventory.h"
#include "level.h"
#include "player.h"

typedef struct
{
	Sint16			health;
	Sint16			armor;
	Sint16			heat;
	Inventory		inventory;
	Uint8			damage_boost;
	Uint32			damage_boost_start;
	Uint8			speed_boost;
	Uint32			speed_boost_start;
	float			cameraFollowDistance;
	GFC_Vector2D	cameraRotation;
} PlayerData;

static Entity *player_load(const char *filename);
static int player_think(Entity *self);
static int player_update(Entity *self);
static int player_draw(Entity *self);
static void player_free(Entity *self);
static void player_handle_inputs(Entity *self);
static void player_handle_touches(Entity *self);

Entity *player_load(const char *filename)
{
	Entity *self;
	Model *model;
	SJson *json, *data;
	const char *str;
	GFC_Box bounds;
	GFC_Color color;

	json = sj_load(filename);
	if (!json)
	{
		slog("failed to load filename %s", filename);
		return NULL;
	}
	data = sj_object_get_value(json, "player");
	if (!data)
	{
		slog("failed to load player data from filename %s", filename);
		sj_free(json);
		return NULL;
	}
	str = sj_object_get_string(data, "model");
	if (!str)
	{
		slog("failed to locate player model from filename %s", filename);
		sj_free(json);
		return NULL;
	}
	model = model_load(str);
	if (!model)
	{
		slog("failed to load player model from filename %s", filename);
		sj_free(json);
		return NULL;
	}
	bounds = gfc_box_from_config(sj_object_get_value(data, "bounds"));
	if (bounds.w == 0 && bounds.h == 0 && bounds.d == 0)
	{
		slog("failed to load player bounds from filename %s", filename);
		sj_free(json);
		return NULL;
	}
	if (!sj_object_get_color_value(data, "color", &color)) color = GFC_COLOR_WHITE;

	self = entity_new();
	if (!self)
	{
		slog("failed to allocate memory for player entity");
		sj_free(json);
		return NULL;
	}
	sj_free(json);
	self->model = model;
	self->bounds = bounds;
	self->color = color;
	return self;
}

Entity *player_spawn(GFC_Vector3D position)
{
	Entity *self;
	PlayerData *data;

	self = player_load("defs/entities/player.json");
	if (!self) return NULL;

	self->layer = EL_PLAYER;
	self->mask = PLAYER_MASK;
	self->position = position;
	self->velocity = gfc_vector3d(0, 0, 0);
	self->rotation = gfc_vector3d(0, 0, 135);
	self->scale = gfc_vector3d(0.5, 0.5, 0.5);

	self->bounds.w = self->bounds.w * self->scale.x;
	self->bounds.h = self->bounds.h * self->scale.y;
	self->bounds.d = self->bounds.d * self->scale.z;

	self->bounds.x = position.x - 0.5 * self->bounds.w;
	self->bounds.y = position.y - 0.5 * self->bounds.h;
	self->bounds.z = position.z - 0.5 * self->bounds.d;

	self->think = player_think;
	self->update = player_update;
	self->draw = player_draw;
	self->free = player_free;

	self->data = gfc_allocate_array(sizeof(PlayerData), 1);
	if (self->data)
	{
		data = self->data;
		inventory_init(&data->inventory);
		inventory_load_data(&data->inventory, "defs/inventory_data.json");
		data->health = 100;
		data->armor = 25;
		data->heat = 25;
		data->cameraFollowDistance = 15;
		data->cameraRotation = gfc_vector2d(0, 0);
	}
	return self;
}

static void player_free(Entity *self)
{
	PlayerData *data;

	if ((!self) || (!self->data)) return;
	data = (PlayerData*)self->data;
	if (data)
	{
		if (&data->inventory)
		{
			inventory_purge_data("defs/inventory_data.json");
			inventory_close(&data->inventory);
		}
		free(data);
	}
	self->data = NULL;
}

static int player_think(Entity *self)
{
	PlayerData *data;
	GFC_Vector3D move = {0};

	if ((!self) || (!self->data)) return 0;
	data = (PlayerData*) self->data;

	// check damage boost
	if (data->damage_boost)
	{
		if (SDL_GetTicks() - data->damage_boost_start > 7000)
		{
			data->damage_boost = 0;
		}
	}
	// check speed boost
	if (data->speed_boost)
	{
		if (SDL_GetTicks() - data->speed_boost_start > 5000)
		{
			data->speed_boost = 0;
		}
	}

	// handle player inputs
	player_handle_inputs(self);

	// gravity
	self->velocity.z = -0.125;

	// collision detection
	if (!entity_move_check(self)) self->velocity = gfc_vector3d(0, 0, 0);

	// handle entity touches for this frame
	player_handle_touches(self);

	return 1;
}

static int player_update(Entity *self)
{
	PlayerData *data;
	GFC_Matrix4 *view;
	GFC_Vector3D offset;
	Window *win;

	if ((!self) || (!self->data)) return 0;
	data = (PlayerData*) self->data;

	// update position
	gfc_vector3d_add(self->position, self->position, self->velocity);

	// update bounding box
	self->bounds.x = self->position.x - 0.5 * self->bounds.w;
	self->bounds.y = self->position.y - 0.5 * self->bounds.h;
	self->bounds.z = self->position.z - 0.5 * self->bounds.d;
	//slog("x: %f, y: %f, z: %f", self->bounds.x, self->bounds.y, self->bounds.z);

	// update camera
	offset = gfc_vector3d(0, -data->cameraFollowDistance, 0);
	gfc_vector3d_add(offset, offset, self->position);
	view = gf3d_vgraphics_get_view_matrix();
	gfc_matrix4_view(
		*view,
		offset,
		self->position,
		gfc_vector3d(0, 0, 1)
	);

	// update hud
	if (data->health < 0) data->health = 0;
	if (data->armor < 0) data->armor = 0;
	if (data->heat < 0) data->heat = 0;

	win = window_find_by_name("hud_menu");
	hud_menu_update_health(win, data->health);
	hud_menu_update_armor(win, data->armor);
	hud_menu_update_heat(win, data->heat);

	return 1;
}

static int player_draw(Entity *self)
{
	if (!self) return 0;

	return 0;
}

static void player_handle_inputs(Entity *self)
{
	PlayerData *data;
	GFC_Vector3D move = {0};

	if ((!self) || (!self->data)) return;
	data = (PlayerData*) self->data;

	// check movement input
	if (gfc_input_command_down("item_up")) move.y += 1;
	if (gfc_input_command_down("item_down")) move.y -= 1;
	if (gfc_input_command_down("item_left")) move.x -= 1;
	if (gfc_input_command_down("item_right")) move.x += 1;
	gfc_vector3d_normalize(&move);
	if (data->speed_boost) gfc_vector3d_scale(move, move, 2);
	else gfc_vector3d_scale(move, move, 0.6);
	self->velocity = move;

	// check item use input
	if (gfc_input_command_down("health"))
	{
		if (inventory_get_item_by_name(&data->inventory, "health"))
		{
			inventory_remove_item(&data->inventory, "health");
			data->health += 50;
		}
	}
	if (gfc_input_command_down("armor"))
	{
		if (inventory_get_item_by_name(&data->inventory, "armor"))
		{
			inventory_remove_item(&data->inventory, "armor");
			data->armor += 50;
		}
	}
	if (gfc_input_command_down("damage"))
	{
		if (inventory_get_item_by_name(&data->inventory, "damage"))
		{
			inventory_remove_item(&data->inventory, "damage");
			data->damage_boost = 1;
			data->damage_boost_start = SDL_GetTicks();
		}
	}
	if (gfc_input_command_down("speed"))
	{
		if (inventory_get_item_by_name(&data->inventory, "speed"))
		{
			inventory_remove_item(&data->inventory, "speed");
			data->speed_boost = 1;
			data->speed_boost_start = SDL_GetTicks();
		}
	}
	if (gfc_input_command_down("heat"))
	{
		if (inventory_get_item_by_name(&data->inventory, "heat"))
		{
			inventory_remove_item(&data->inventory, "heat");
			data->heat += 50;
		}
	}
}

static void player_handle_touches(Entity *self)
{
	int i, c;
	Entity *other;
	PlayerData *data;

	if ((!self) || !(self->data)) return;
	data = (PlayerData*) self->data;

	entity_get_touches(self);

	c = gfc_list_get_count(self->touches);
	for (i = 0; i < c; i++)
	{
		other = gfc_list_get_nth(self->touches, i);
		if (!other) continue;
		
		if (other->layer == EL_MONSTER)
		{
			self->velocity.x = 0;
			self->velocity.y = 0;
		}
		else if (other->layer == EL_ITEM)
		{
			if (gfc_strlcmp(other->name, "health") == 0)
			{
				if (inventory_add_item(&data->inventory, "health")) entity_free(other);
			}
			else if (gfc_strlcmp(other->name, "armor") == 0)
			{
				if (inventory_add_item(&data->inventory, "armor")) entity_free(other);
			}
			else if (gfc_strlcmp(other->name, "damage") == 0)
			{
				if (inventory_add_item(&data->inventory, "damage")) entity_free(other);
			}
			else if (gfc_strlcmp(other->name, "speed") == 0)
			{
				if (inventory_add_item(&data->inventory, "speed")) entity_free(other);
			}
			else if (gfc_strlcmp(other->name, "heat") == 0)
			{
				if (inventory_add_item(&data->inventory, "heat")) entity_free(other);
			}
		}
		else if (other->layer == EL_PROJECTILE)
		{
			continue;
		}
		else if (other->layer == EL_WORLD)
		{
			continue;
		}
	}
}

void player_damage(Entity *self, Uint32 damage)
{
	PlayerData *data;

	if ((!self) || !(self->data)) return;
	data = (PlayerData*) self->data;

	if (data->armor)
	{
		data->armor -= damage;
		data->health -= (int)(damage/5);
	}
	else data->health -= damage;
}