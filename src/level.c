#include "simple_logger.h"

#include "gf3d_obj_load.h"

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
	level->bounds = gfc_box_from_config(sj_object_get_value(json, "bounds"));
	str = sj_object_get_string(json, "music");
	if (str)
	{
		level->bgMusic = gfc_sound_load_music(str);
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
		Mix_HaltMusic();
		Mix_FreeMusic(level->bgMusic);
	}
	if (level->json) sj_free(level->json);
	free(level);
}

void level_draw(Level *level)
{
	GFC_Matrix4 mat, mat2;

	if (!level) return;

	gfc_matrix4_identity(mat);
	gfc_matrix4_from_vectors(
		mat2,
		gfc_vector3d(0, 0 ,0),
		gfc_vector3d(0, 0, 0),
		gfc_vector3d(1, 1, 1)
	);
	model_queue_render(level->sky, mat, GFC_COLOR_BLUE);
	model_queue_render(level->terrain, mat2, GFC_COLOR_GREEN);
}

Uint8 level_line_test(Level *level, GFC_Edge3D edge, GFC_Vector3D *contact)
{
	MeshPrimitive *prim;
	GFC_Triangle3D t;
	GFC_Vector3D out = {-1};
	int i, j, c;

	if ((!level) || (!level->terrain->mesh) || (!level->sky->mesh)) return 0;

	c = gfc_list_count(level->terrain->mesh->primitives);
	for (i = 0; i < c; i++)
	{
		prim = gfc_list_get_nth(level->terrain->mesh->primitives, i);
		if (!prim) continue;
		if (!prim->objData) continue;
		for (j = 0; j < prim->objData->face_count; j++)
		{
			t.a = prim->objData->faceVertices[prim->objData->outFace[j].verts[0]].vertex;
			t.b = prim->objData->faceVertices[prim->objData->outFace[j].verts[1]].vertex;
			t.c = prim->objData->faceVertices[prim->objData->outFace[j].verts[2]].vertex;
			if (gfc_triangle_edge_test(edge, t, &out))
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

	if (aabb.x < level->bounds.x) return 1;
	if (aabb.y < level->bounds.y) return 1;
	if (aabb.z < level->bounds.z) return 1;

	if (aabb.x + aabb.w > level->bounds.x + level->bounds.w) return 1;
	if (aabb.y + aabb.h > level->bounds.y + level->bounds.h) return 1;
	if (aabb.z + aabb.d > level->bounds.z + level->bounds.d) return 1;

	return 0;
}