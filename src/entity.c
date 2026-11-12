#include "simple_logger.h"

#include "gfc_color.h"
#include "model.h"
#include "draw_3d.h"
#include "level.h"

#include "entity.h"

extern int __COLLISION;

typedef struct
{
	Entity		*entityList;	/**<list of entities*/
	Uint32		entMax;			/**<max number of entities supported*/
	Level		*level;			/**<current level*/
} EntityManager;

static EntityManager entity_manager = {0};

static void entity_system_close();
static void entity_think(Entity *self);
static void entity_update(Entity *self);
static void entity_draw(Entity *self);
static Uint8 entity_touch_test(Entity *self, Entity *other);

void entity_system_init(Uint32 entMax)
{
	if (!entMax)
	{
		slog("failed to initialize entity manager: entMax is 0");
		return;
	}
	entity_manager.entityList = gfc_allocate_array(sizeof(Entity), entMax);
	if (!entity_manager.entityList)
	{
		slog("failed to allocate memory for entity manager");
		return;
	}
	entity_manager.entMax = entMax;
	entity_manager.level = NULL;
	slog("entity system initialized");
	atexit(entity_system_close);
}

static void entity_system_close()
{
	int i;

	if (!entity_manager.entityList) return;

	for (i = 0; i < entity_manager.entMax; i++)
	{
		if (entity_manager.entityList[i]._inuse) entity_free(&entity_manager.entityList[i]);
	}
	slog("entity system closed");
}

Entity *entity_new()
{
	int i;

	for (i = 0; i < entity_manager.entMax; i++)
	{
		if (entity_manager.entityList[i]._inuse) continue;
		memset(&entity_manager.entityList[i], 0, sizeof(Entity));

		// init list of entity touches
		entity_manager.entityList[i].touches = gfc_list_new();
		if (!entity_manager.entityList[i].touches)
		{
			slog("failed to allocate list of entity touches");
			return NULL;
		}

		// set defaults
		entity_manager.entityList[i]._inuse = 1;
		entity_manager.entityList[i].rotation = gfc_vector3d(0, 0, 0);
		entity_manager.entityList[i].scale = gfc_vector3d(1, 1, 1);
		entity_manager.entityList[i].color = GFC_COLOR_WHITE;
		return &entity_manager.entityList[i];
	}
	slog("no more available entities");
	return NULL;
}

void entity_free(Entity *self)
{
	if ((!self) || (!self->_inuse)) return;

	if (self->model) model_free(self->model);
	if (self->touches) gfc_list_delete(self->touches);
	if (self->free) self->free(self);
	self->_inuse = 0;
}

void entity_system_set_level(Level *level)
{
	entity_manager.level = level;
}

void entity_system_think()
{
	int i;
	for (i = 0; i < entity_manager.entMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse) continue;
		entity_think(&entity_manager.entityList[i]);
	}
}

static void entity_think(Entity *self)
{
	if ((!self) || (!self->_inuse)) return;

	if (self->think)
	{
		if (self->think(self)) return;
	}

	// boilerplate entity think code

}

void entity_system_update()
{
	int i;
	for (i = 0; i < entity_manager.entMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse) continue;
		entity_update(&entity_manager.entityList[i]);
	}
}

static void entity_update(Entity *self)
{
	if ((!self) || (!self->_inuse)) return;

	if (self->update)
	{
		if (self->update(self)) return;
	}

	// boilerplate entity update code
	gfc_vector3d_add(self->position, self->position, self->velocity);
}

void entity_system_draw()
{
	int i;
	for (i = 0; i < entity_manager.entMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse) continue;
		entity_draw(&entity_manager.entityList[i]);
	}
}

static void entity_draw(Entity *self)
{
	GFC_Matrix4 mat = {0};

	if ((!self) || (!self->_inuse)) return;

	if (__COLLISION) draw_3d_cube_outline(self->bounds, GFC_COLOR_MAGENTA);

	if (self->draw)
	{
		if (self->draw(self)) return;
	}

	// boilerplate draw code
	gfc_matrix4_from_vectors(
		mat,
		self->position,
		self->rotation,
		self->scale);

	model_queue_render(self->model, mat, self->color);
}

Uint8 entity_move_check(Entity *self)
{
	GFC_Box bounds = {0};
	GFC_Edge3D edge1, edge2;

	if ((!self) || (!entity_manager.level)) return 0;

	bounds.x = self->bounds.x + self->velocity.x - 0.5 * self->bounds.w * self->scale.x;
	bounds.y = self->bounds.y + self->velocity.y - 0.5 * self->bounds.h * self->scale.y;
	bounds.z = self->bounds.z + self->velocity.z - 0.5 * self->bounds.d * self->scale.z;
	bounds.w = self->bounds.w;
	bounds.h = self->bounds.h;
	bounds.d = self->bounds.d;

	edge1 = gfc_edge3d(bounds.x + 0.5*bounds.w, bounds.y + 0.5*bounds.h, bounds.z,
				bounds.x + 0.5*bounds.w, bounds.y + 0.5*bounds.h, bounds.z + 0.5*bounds.d);
	
	edge2 = gfc_edge3d(bounds.x + bounds.w, bounds.y, bounds.z, bounds.x, bounds.y + bounds.h, bounds.z);

	// entities should not leave world bounds
	if (level_bounds_test(entity_manager.level, bounds)) return 0;

	// if entity hits top or bottom of a mesh, halt z movement
	if (level_line_test(entity_manager.level, edge1, NULL)) self->velocity.z = 0;

	//if (level_line_test(entity_manager.level, edge2, NULL)) self->velocity.z = 0;
	return 1;
}

void entity_get_touches(Entity *self)
{
	int i;
	Entity *other;

	gfc_list_clear(self->touches);
	for (i = 0; i < entity_manager.entMax; i++)
	{
		other = &entity_manager.entityList[i];
		if (!other || !other->_inuse) continue;
		if (other == self) continue;

		// discard touches with entities not part of collision mask
		switch (other->layer)
		{
		case EL_PLAYER:
			if (!(self->mask & EL_PLAYER)) continue;
			break;

		case EL_MONSTER:
			if (!(self->mask & EL_MONSTER)) continue;
			break;

		case EL_ITEM:
			if (!(self->mask & EL_ITEM)) continue;
			break;

		case EL_PROJECTILE:
			if (!(self->mask & EL_PROJECTILE)) continue;
			break;

		case EL_WORLD:
			if (!(self->mask & EL_WORLD)) continue;
			break;
		}

		if (entity_touch_test(self, other)) gfc_list_append(self->touches, other);
	}
}

static Uint8 entity_touch_test(Entity *self, Entity *other)
{
	GFC_Box selfBox, otherBox;

	if ((!self) || (!other)) return 0;

	selfBox.x = self->bounds.x + self->velocity.x - 0.5 * self->bounds.w * self->scale.x;
	selfBox.y = self->bounds.y + self->velocity.y - 0.5 * self->bounds.h * self->scale.y;
	selfBox.z = self->bounds.z + self->velocity.z - 0.5 * self->bounds.d * self->scale.z;
	selfBox.w = self->bounds.w;
	selfBox.h = self->bounds.h;
	selfBox.d = self->bounds.d;

	otherBox.x = other->bounds.x + other->velocity.x - 0.5 * other->bounds.w * other->scale.x;
	otherBox.y = other->bounds.y + other->velocity.y - 0.5 * other->bounds.h * other->scale.y;
	otherBox.z = other->bounds.z + other->velocity.z - 0.5 * other->bounds.d * other->scale.z;
	otherBox.w = other->bounds.w;
	otherBox.h = other->bounds.h;
	otherBox.d = other->bounds.d;

	return gfc_box_overlap(selfBox, otherBox);
}

Entity *entity_get_closest_monster(Entity *self)
{
	Entity *other = NULL, *closest = NULL;
	float dist, distBest = 5;

	if (!self) return;

	int i;
	for (i = 0; i < entity_manager.entMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse) continue;
		if (!(entity_manager.entityList[i].layer == EL_MONSTER)) continue;

		other = &entity_manager.entityList[i];
		dist = gfc_vector3d_magnitude_between(self->position, other->position);
		if (dist < distBest)
		{
			closest = other;
			distBest = dist;
		}
	}
	return closest;
}