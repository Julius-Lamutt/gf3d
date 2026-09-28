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
    VkDevice    device;
} ModelManager;

static ModelManager model_manager = {0};

void model_init_system(Uint32 modelCount)
{
    if (model_manager.MeshCount != 0)
    {
        slog("cannot initialize mesh system, already initialized!");
        return;
    }
    if (modelCount == 0)
    {
        slog("canot initialize mesh system for ZERO meshes");
        return;
    }

    gf3d_mesh_get_attribute_descriptions(&count);
    gf3d_mesh.pipe = gf3d_pipeline_create_from_config(
        gf3d_vgraphics_get_default_logical_device(),
        "config/overlay_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        max_sprites, //change
        gf3d_mesh_get_bind_description(),
        gf3d_mesh_get_attribute_descriptions(NULL),
        count,
        sizeof(SpriteUBO),
        VK_INDEX_TYPE_UINT16
    ); 
    atexit(gf3d_model_close);
}

void model_close()
{
    int i, c;
    for (i = 0; i < c; i++)
    {
        model_free(&model_manager.modelList[i]);
    }
    free(model_manager.modelList;
    gf3d_pipeline_free(model_manager.pipe);
    memset(&model_manager, 0, sizeof(ModelManager));
}

void model_free(Model *model)
{
    if (!model) return;
}

void model_render(Mesh *mesh, VkCommandsBuffer, commandBuffer, )
{

}