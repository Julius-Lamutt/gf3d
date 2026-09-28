#ifndef __MODEL_H__
#define __MODEL_H__

#include "gf3d_mesh.h"

typedef struct
{
	GFC_Matrix4     model;
	GFC_Matrix4     view;
	GFC_Matrix4     proj;
	GFC_Vector4D    color;
} MeshUBO;

typedef struct
{
	int					_refCount;
	Texture				*texture;			/*<texture memory pointer*/
	Mesh				*mesh;				/*<GPU handles for mesh data*/
	VkDescriptorSet		*descriptorSet;		/*<descriptor sets used for this model to render*/
} Model;

/*
* @brief initialize model manager
*/
void model_init_system(Uint32 modelCount);

/*
* @brief initialize model manager
*/
void model_close();

#endif
