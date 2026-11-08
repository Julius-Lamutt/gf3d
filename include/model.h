#ifndef __MODEL_H__
#define __MODEL_H__

#include "gf3d_mesh.h"

typedef enum
{
	PT_NONE,
	PT_MODEL,
	PT_SKY,
	PT_OUTLINE
} PipelineType;

typedef struct
{
	GFC_Matrix4     model;
	GFC_Matrix4     view;
	GFC_Matrix4     proj;
	GFC_Vector4D    color;
} ModelUBO;

typedef struct
{
	int					_refCount;
	GFC_TextLine		filename;			/**<name of the model*/
	Texture				*texture;			/**<texture memory pointer*/
	Mesh				*mesh;				/**<GPU handles for mesh data*/
	Uint8				pipeline;			/**<type of pipeline*/
} Model;

/**
* @brief initialize model manager
* @param modelCount the maximum number of models that can be supported
*/
void model_init_system(Uint32 modelCount);

/**
* @brief create a new model
* @return a new model
*/
Model *model_new();

/**
* @brief free a model
* @param model the model to free
*/
void model_free(Model *model);

/**
* @brief load a model from a json file
* @param filename name of the model file to load
*/
Model *model_load(const char *filename);

/**
* @brief find an existing model by filename
* @param filename name of the model to find
*/
Model *model_get_by_filename(const char *filename);

/*
* @brief get the model pipeline
* @return NULL if pipeline does not exist, the model pipeline otherwise
*/
Pipeline *model_get_pipeline();

/*
* @brief get the sky pipeline
* @return NULL if pipeline does not exist, the sky pipeline otherwise
*/
Pipeline *model_get_sky_pipeline();

/**
* @brief given a model matrix and basic color, build the modelUBO needed to render a model
* @param modelMat the model Matrix
* @param colorMod the color for the UBO
*/
ModelUBO model_get_ubo(GFC_Matrix4 modelMat, GFC_Color colorMod);

/*
* @brief queue a render for a model
* @param model the model to render
* @param mat the model matrix
* @param colorMod the color mod for the render
*/
void model_queue_render(Model *model, GFC_Matrix4 mat, GFC_Color colorMod);

#endif
