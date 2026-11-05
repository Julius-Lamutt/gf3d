#include "simple_logger.h"

#include "gf3d_load_obj.h"

#include "level.h"

Level *level_new()
{
	Level *level;
	level = gfc_allocate_array(sizeof(Level), 1);
	if (!level)
	{
		slog("failed to allocate space for level");
		return NULL;
	}

	// boilerplate code
	gfc_matrix4_identity(level->terrainMatrix);
	return level;
}

Level *level_load(const char *filename)
{
	SJson *json;
	SJson *data;
	const char *str;
	Level *level;

	json = sj_load(filename);
	if (!json)
	{
		slog("failed to open level file %s", filename);
		return NULL;
	}

	level = level_new();
	if (!level)
	{
		slog("failed to make level to load");
		return NULL;
	}
	level->json = json;

	str = sj_object_get_string(json, "sky");
	if (str)
	{
		level->sky = model_load(str);
		if (level->sky) gfc_line_cpy(level->skyFile, str);
	}
	str = sj_object_get_string(json, "terrain");
	if (str)
	{
		level->terrain = model_load(str);
		if (level->terrain) gfc_line_cpy(level->terrainFile, str);
	}
	level->bounds = gfc_box_from_config(sj_object_get_value(bounds));
	str = sj_object_get_string(json, "music");
	if (str)
	{
		level->bgMusic = gfc_dound_load_music(str);
		if (level->bgMusic) Mix_PlayMusic(level->bgMusic, -1);
	}
	return level;
}

void level_free(Level *level)
{
	if (!level) return;

	if (level->sky) model_free(level->sky);
	if (level->terrain) model_free(level->terrain);
	if (level->bgMusic)
	{
		Mix_HaltMusic(level->bgMusic);
		Mix_FreeMusic(level->bgMusic);
	}
	if (level->json) sj_free(level->json);
	free(level);
}

void level_draw(Level *level)
{
	if (!level) return;

	//model_queue_render();
}

Uint8 level_line_test(Level *level, GFC_Edge3d edge, GFC_Vector3D *contact)
{
	Mesh *mesh;
	MeshPrimitive *prim;
	GFC_Triangle3D t;
	GFC_Vector3D out = {-1};
	int i, j, c;

	if ((!level) || (!level->terrain->prim) || (!level->sky->prim)) return out;

	c = gfc_list_count(level->terrain->mesh->primitives);
	for (i = 0; i < c; i++)
	{
		prim = gfc_list_get_nth(level->terrain->mesh->primitives, i);
		if (!prim) continue;
		if (!prim->objData) continue;
		for (j = 0; j < prim->objData->face_count; j++)
		{
			t.a = prim->objData->faceVertices[prim->objData->outFace[j]->verts[0]].vertex;
			t.b = prim->objData->faceVertices[prim->objData->outFace[j]->verts[1]].vertex;
			t.c = prim->objData->faceVertices[prim->objData->outFace[j]->verts[2]].vertex;
			if (gfc_tiangle_edge_test(edge, t, &out))
			{
				if (contact) *contact = out;
				return 1;
			}
		}
	}
	return 0;
}

Uint8 level_bounds_test(Level *level, GFC_Box aabb)
{
	if (!level) return 0;

	if (aabb.x < level.x) return 1;
	if (aabb.y < level.y) return 1;
	if (aabb.z < level.z) return 1;

	if (aabb.x + aabb.w > level->bounds.x + level->bounds.w) return 1;
	if (aabb.y + aabb.h > level->bounds.y + level->bounds.h) return 1;
	if (aabb.z + aabb.d > level->bounds.z + level->bounds.d) return 1;

	return 0;
}