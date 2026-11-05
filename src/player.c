#include "simple_logger.h"

#include "gfc_input.h"

#include "level.h"
#include "player.h"

typedef struct
{
	float			cameraFollowDistance;
	GFC_Vector2D	cameraRotation;
} PlayerData;

int player_think(Entity *self);
int player_update(Entity *self);
int player_draw(Entity *self);
void player_free(Entity *self);

Entity *player_spawn(GFC_Vector3D position)
{
	Entity *self;
	PlayerData *data;

	self = entity_new();
	if (!self) return NULL;
	self->position = position;
	self->velocity = gfc_vector3d(0, 0, 0);

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

	if (gfc_input_command_down("item_up")) move.y += 1;
	if (gfc_input_command_down("item_down")) move.y -= 1;
	if (gfc_input_command_down("item_left")) move.x -= 1;
	if (gfc_input_command_down("item_right")) move.x += 1;

	self->velocity = move;

	return 1;
}

int player_update(Entity *self)
{
	if (!self) return 0;

	gfc_vector3d_add(self->position, self->position, self->velocity);

	return 1;
}

int player_draw(Entity *self)
{
	if (!self) return 0;

	return 1;
}