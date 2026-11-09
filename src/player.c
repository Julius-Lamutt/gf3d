#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config.h"

#include "gf3d_vgraphics.h"

#include "level.h"
#include "player.h"

typedef struct
{
	float			cameraFollowDistance;
	GFC_Vector2D	cameraRotation;
} PlayerData;

static Entity *player_load(const char *filename);
static int player_think(Entity *self);
static int player_update(Entity *self);
static int player_draw(Entity *self);
static void player_free(Entity *self);

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

	self->position = position;
	self->velocity = gfc_vector3d(0, 0, 0);
	self->rotation = gfc_vector3d(0, 0, 135);

	self->bounds.x = position.x - 0.5 * self->bounds.w * self->scale.x;
	self->bounds.y = position.y - 0.5 * self->bounds.h * self->scale.y;
	self->bounds.z = position.z - 0.5 * self->bounds.d * self->scale.z;

	self->think = player_think;
	self->update = player_update;
	self->draw = player_draw;
	self->free = player_free;

	self->data = gfc_allocate_array(sizeof(PlayerData), 1);
	if (self->data)
	{
		data = self->data;
		data->cameraFollowDistance = 20;
		data->cameraRotation = gfc_vector2d(0, 0);
	}
	return self;
}

void player_free(Entity *self)
{
	PlayerData *data;

	if ((!self) || (!self->data)) return;
	data = (PlayerData*)self->data;
	if (data) free(data);
	self->data = NULL;
}

int player_think(Entity *self)
{
	GFC_Vector3D move = {0};

	if (!self) return 0;

	// check movement input
	if (gfc_input_command_down("item_up")) move.y += 1;
	if (gfc_input_command_down("item_down")) move.y -= 1;
	if (gfc_input_command_down("item_left")) move.x -= 1;
	if (gfc_input_command_down("item_right")) move.x += 1;

	self->velocity = move;

	// gravity
	//self->velocity.z = -0.125;

	// collision detection
	if (!entity_move_check(self)) self->velocity = gfc_vector3d(0, 0, 0);

	return 1;
}

int player_update(Entity *self)
{
	GFC_Matrix4 *view;
	GFC_Vector3D offset;

	if (!self) return 0;

	// update position
	gfc_vector3d_add(self->position, self->position, self->velocity);

	// update bounding box
	self->bounds.x = self->position.x - 0.5 * self->bounds.w * self->scale.x;
	self->bounds.y = self->position.y - 0.5 * self->bounds.h * self->scale.y;
	self->bounds.z = self->position.z - 0.5 * self->bounds.d * self->scale.z;
	//slog("x: %f, y: %f, z: %f", self->bounds.x, self->bounds.y, self->bounds.z);

	// update camera
	offset = gfc_vector3d(0, -20, 0);
	gfc_vector3d_add(offset, offset, self->position);
	view = gf3d_vgraphics_get_view_matrix();
	gfc_matrix4_view(
		*view,
		offset,
		self->position,
		gfc_vector3d(0, 0, 1)
	);

	return 1;
}

int player_draw(Entity *self)
{
	if (!self) return 0;

	return 0;
}