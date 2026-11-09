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

void entity_system_close();
void entity_think(Entity *ent);
void entity_update(Entity *ent);
void entity_draw(Entity *ent);

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

void entity_system_close()
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

		// set defaults
		entity_manager.entityList[i]._inuse = 1;
		entity_manager.entityList[i].rotation = gfc_vector3d(0, 0, 0);
		entity_manager.entityList[i].scale = gfc_vector3d(1, 1, 1);
		entity_manager.entityList[i].color = GFC_COLOR_WHITE;
		return &entity_manager.entityList[i];
	}
	return NULL;
}

void entity_free(Entity *self)
{
	if ((!self) || (!self->_inuse)) return;

	if (self->model) model_free(self->model);
	memset(self, 0, sizeof(Entity));
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

void entity_think(Entity *self)
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

void entity_update(Entity *self)
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

void entity_draw(Entity *self)
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

	bounds.x = self->bounds.x + self->velocity.x - 1.5;
	bounds.y = self->bounds.y + self->velocity.y - 1.5;
	bounds.z = self->bounds.z + self->velocity.z - 1.5;
	bounds.w = self->bounds.w;
	bounds.h = self->bounds.h;
	bounds.d = self->bounds.d;

	edge1 = gfc_edge3d(bounds.x, bounds.y, bounds.z, bounds.x + bounds.w, bounds.y + bounds.h, bounds.z);
	edge2 = gfc_edge3d(bounds.x + bounds.w, bounds.y, bounds.z, bounds.x, bounds.y + bounds.h, bounds.z);

	if (level_bounds_test(entity_manager.level, bounds)) return 0;
	if (level_line_test(entity_manager.level, edge1, NULL)) return 0;
	if (level_line_test(entity_manager.level, edge2, NULL)) return 0;
	return 1;
}