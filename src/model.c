#include "simple_logger.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_commands.h"
#include "gf2d_sprite.h"

#include "model.h"

typedef struct
{
	Model		*modelList;
	Uint32		modelCount;
    Pipeline    *pipe;
    Texture     *defaultTexture;
} ModelManager;

static ModelManager model_manager = {0};

void model_close();
void model_delete(Model *model);

void model_init_system(Uint32 modelCount)
{
    Uint32 count;

    if (model_manager.modelCount != 0)
    {
        slog("cannot initialize model system, already initialized!");
        return;
    }
    if (modelCount == 0)
    {
        slog("canot initialize model system for ZERO models");
        return;
    }

    model_manager.modelList = (Model *)gfc_allocate_array(sizeof(Model), modelCount);
    if (!model_manager.modelList)
    {
        slog("failed to allocate model list");
        return;
    }
    model_manager.modelCount = modelCount;

    // mesh cannot get attributes unless mesh manager's device is set from model!!
    gf3d_mesh_set_device(gf3d_vgraphics_get_default_logical_device());

    gf3d_mesh_get_attribute_descriptions(&count);
    model_manager.pipe = gf3d_pipeline_create_from_config(
        gf3d_vgraphics_get_default_logical_device(),
        "config/model_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        modelCount,
        gf3d_mesh_get_bind_description(),
        gf3d_mesh_get_attribute_descriptions(NULL),
        count,
        sizeof(ModelUBO),
        VK_INDEX_TYPE_UINT32
    ); 
    if (!model_manager.pipe)
    {
        slog("exit status 1: failed to make model pipeline!");
        slog_sync();
        model_close();
        return;
    }

    model_manager.defaultTexture = gf3d_texture_load("images/default.png");
    if (!model_manager.defaultTexture)
    {
        slog("we are so screwed: no defualt texture :(");
    }

    slog("model manager initialized");
    atexit(model_close);
}

void model_close()
{
    int i;

    for (i = 0; i < model_manager.modelCount; i++)
    {
        if (model_manager.modelList[i]._refCount) model_delete(&model_manager.modelList[i]);
    }
    free(model_manager.modelList);
    if (model_manager.defaultTexture)
    {
        gf3d_texture_free(model_manager.defaultTexture);
    }
    if (model_manager.pipe)
    {
        gf3d_pipeline_free(model_manager.pipe);
    }
    memset(&model_manager, 0, sizeof(ModelManager));
    slog("model manager closed");
}

Model *model_new()
{
    int i;
    for (i = 0; i < model_manager.modelCount; i++)
    {
        if (model_manager.modelList[i]._refCount) continue;
        model_manager.modelList[i]._refCount = 1;
        return &model_manager.modelList[i];
    }
    slog("model_new: no more space for additional models");
    return NULL;
}

void model_free(Model *model)
{
    if (!model) return;

    model->_refCount--;
    if (model->_refCount <= 0) model_delete(model);
}

void model_delete(Model *model)
{
    if (!model) return;

    gf3d_texture_free(model->texture);
    gf3d_mesh_free(model->mesh);
    memset(model, 0, sizeof(model));
}

Model *model_load(const char *filename)
{
    Model *model;
    Mesh *mesh;
    Texture *texture;
    SJson *json, *data;
    const char *str1, *str2;

    if (!filename) return NULL;

    model = model_get_by_filename(filename);
    if (model)
    {
        model->_refCount++;
        return model;
    }
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to get an empty space in memory for model %s", filename);
        return NULL;
    }
    data = sj_object_get_value(json, "model");
    if (!data)
    {
        slog("failed to get model information from file %s", filename);
        sj_free(json);
        return NULL;
    }
    str1 = sj_object_get_string(data, "obj");
    if (!str1)
    {
        slog("failed to find obj data in file %s", filename);
        sj_free(json);
        return NULL;
    }
    mesh = gf3d_mesh_load_obj(str1);
    if (!mesh)
    {
        slog("failed to parse obj data for model file %s", filename);
        sj_free(json);
        return NULL;
    }
    str2 = sj_object_get_string(data, "texture");
    if (str2)
    {
        texture = gf3d_texture_load(str2);
        if (!texture) texture = model_manager.defaultTexture;
    }
    else texture = model_manager.defaultTexture;

    model = model_new();
    if (!model)
    {
        slog("failed to get an empty space in memory for model %s", filename);
        return NULL;
    }
    gfc_line_cpy(model->filename, filename);
    model->texture = texture;
    model->mesh = mesh;
    return model;
}

Model *model_get_by_filename(const char *filename)
{
    int i;
    if (!filename) return NULL;
    for (i = 0; i < model_manager.modelCount; i++)
    {
        if (model_manager.modelList[i]._refCount == 0) continue;
        if (gfc_strlcmp(model_manager.modelList[i].filename, filename) == 0)
        {
            return &model_manager.modelList[i];
        }
    }
    return NULL;
}

Pipeline *model_get_pipeline()
{
    if (!model_manager.pipe)
    {
        slog("model pipeline does not exist");
        return NULL;
    }
    return model_manager.pipe;
}

ModelUBO model_get_ubo(GFC_Matrix4 modelMat, GFC_Color colorMod)
{
    ModelUBO ubo = {0};
    GFC_Matrix4 *view = NULL, proj;

    gfc_matrix4_copy(ubo.model, modelMat);
    view = gf3d_vgraphics_get_view_matrix();
    gfc_matrix4_copy(ubo.view, *view);
    gf3d_vgraphics_get_projection_matrix(&proj);
    gfc_matrix4_copy(ubo.proj, proj);
    ubo.color = gfc_color_to_vector4f(colorMod);
    
    return ubo;
}

void model_queue_render(Model *model, GFC_Matrix4 mat, GFC_Color colorMod)
{
    ModelUBO ubo;
    if (!model) return;
    ubo = model_get_ubo(mat, colorMod);

    gf3d_mesh_queue_render(model->mesh, model_manager.pipe, &ubo, model->texture);
}