#include "simple_logger.h"

#include "draw_3d.h"

typedef struct
{
	Model *cube;
	Model *sphere;
} Draw3dData;

Draw3dData draw_3d_data = {0};

void draw_3d_close();

void draw_3d_close()
{
	model_free(draw_3d_data.cube);
	model_free(draw_3d_data.sphere);
}

void draw_3d_init()
{
	draw_3d_data.cube = model_load("models/cube.model");
	draw_3d_data.sphere = model_load("models/sphere.model");
	atexit(draw_3d_close);
}

void draw_3d_cube(GFC_Box cube)
{
	GFC_Matrix4 mat;

	gfc_matrix4_from_vectors(
		mat,
		gfc_vector3d(cube.x - (cube.w * 0.5), cube.y - (cube.h * 0.5), cube.z - (cube.d * 0.5)),
		gfc_vector3d(0, 0, 0),
		gfc_vector3d(cube.w, cube.h, cube.d)
	);

	model_queue_render(draw_3d_data.cube, mat, GFC_COLOR_MAGENTA);
}

void draw_3d_sphere(GFC_Sphere sphere)
{
	GFC_Matrix4 mat;

	gfc_matrix4_from_vectors(
		mat,
		gfc_vector3d(sphere.x - sphere.r, sphere.y - sphere.r, sphere.z - sphere.r),
		gfc_vector3d(0, 0, 0),
		gfc_vector3d(sphere.r, sphere.r, sphere.r)
	);

	model_queue_render(draw_3d_data.sphere, mat, GFC_COLOR_MAGENTA);
}