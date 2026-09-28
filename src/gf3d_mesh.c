#include "simple_logger.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_commands.h"
#include "gf3d_obj_load.h"
#include "gf3d_mesh.h"

#define MESH_ATTRIBUTE_COUNT 2

typedef struct
{
    Uint32      meshCount;      /**<maximum concurrent meshes supported*/
    Mesh        *meshList;      /**<pre-allocated space for meshes*/
    Pipeline    *pipe;          /**<pipeline associated with mesh manager*/
    VkDevice    device;         /**<logical vulcan device*/
    VkVertexInputAttributeDescription   attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
} MeshManager;

static MeshManager mesh_manager = {0};

/**
* @brief closes the mesh system
*/
void gf3d_mesh_close();

/**
* @brief delete a mesh and its primitives
* @param mesh: the mesh to delete
*/
void gf3d_mesh_delete(Mesh *mesh);

/**
* @brief delete a mesh primitive
* @param prim: the primitive to delete
*/
void gf3d_mesh_primitive_free(MeshPrimitive *prim);

void gf3d_mesh_init(Uint32 mesh_max)
{
    if (mesh_manager.meshCount != 0)
    {
        slog("cannot init mesh system; already initialized");
        return;
    }
    if (mesh_max == 0)
    {
        slog("cannot initialize mesh system for ZERO meshes");
        return;
    }
    mesh_manager.meshList = gfc_allocate_array(sizeof(Mesh), mesh_max);
    if (!mesh_manager.meshList) return;
    mesh_manager.meshCount = mesh_max;
    atexit(gf3d_mesh_close);
}

void gf3d_mesh_close()
{
    int i;
    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        gf3d_mesh_delete(&mesh_manager.meshList[i]);
    }
    free(mesh_manager.meshList);
    memset(&mesh_manager, 0, sizeof(MeshManager));
}

Mesh* gf3d_mesh_new()
{
    int i;
    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        if ((mesh_manager.meshList[i]._refCount == 0) && (strlen(mesh_manager.meshList[i].filename) == 0))
        {
            mesh_manager.meshList[i].primitives = gfc_list_new();
            if (mesh_manager.meshList[i].primitives == NULL)
            {
                slog("cannot allocate more memory for a new mesh");
                return NULL;
            }
            mesh_manager.meshList[i]._refCount = 1;
            return &mesh_manager.meshList[i];
        }
    }
    return NULL;
}

void gf3d_mesh_free(Mesh* mesh)
{
    if (!mesh) return;

}

void gf3d_mesh_delete(Mesh* mesh)
{
    int i, c;
    MeshPrimitive* prim;

    if (!mesh) return;

    c = gfc_list_count(mesh->primitives);
    for (i = 0; i < c; i++)
    {
        prim = gfc_list_nth(mesh->primitives, i);
        if (!prim) continue;
        gf3d_mesh_primitive_free(prim);
    }
}

Mesh* gf3d_mesh_get_by_filename(const char* filename)
{
    int i;

    if (!filename)
    {
        return NULL;
    }

    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        if (strlen(mesh_manager.meshList[i].filename == 0)) continue;
        if (gfc_strlcmp(mesh_manager.meshList[i].filename, filename) == 0)
        {
            return &mesh_manager.meshList[i];
        }
    }
    return NULL;
}

int gf3d_mesh_obj_buffer_create(Mesh* mesh)
{
    int i, c;
    MeshPrimitive *prim;

    if (!mesh) return 0;
    if (gf3d_mesh_primitive_buffer_create(prim))
    {
        gfc_list_append(mesh->primitives, prim);
        return 1;
    }
    return 0;
}

int gf3d_mesh_buffer_create(Mesh* mesh)
{
    int i, c;
    MeshPrimitive *prim;

    if (!mesh) return 0;
    c = gfc_list_count(mesh->primitives);
    for (i = 0; i < c; i++)
    {
        prim = gfc_list_get_nth(mesh->primitives, i);
        if (!prim) continue;
        if (!gf3d_mesh_primitive_buffer_create(prim)) return 0;
    }
}

int gf3d_mesh_primitive_buffer_create(MeshPrimitive *prim)
{
    void *data = NULL;
    Uint32 bufferSize = 0;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if ((!prim) || (prim->objData)) return 0;

    // Face Buffers

    bufferSize = sizeof(Face) * prim->objData->face_count + 1;

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(mesh_manager.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, prim->objData, (size_t) bufferSize);
    vkUnmapMemory(mesh_manager.device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->faceBuffer, &prim->faceBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->faceBuffer, bufferSize);

    vkDestroyBuffer(mesh_manager.device, stagingBuffer, NULL);
    vkFreeMemory(mesh_manager.device, stagingBufferMemory, NULL);

    // Vertex Buffers

    bufferSize = sizeof(Vertex) * prim->objData->face_vert_count;

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(mesh_manager.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, prim->objData, (size_t) bufferSize);
    vkUnmapMemory(mesh_manager.device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->faceBuffer, &prim->faceBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->vertexBuffer, bufferSize);

    vkDestroyBuffer(mesh_manager.device, stagingBuffer, NULL);
    vkFreeMemory(mesh_manager.device, stagingBufferMemory, NULL);
}

Mesh* gf3d_mesh_load_obj(const char* filename)
{
    Mesh *mesh;
    MeshPrimitive *prim;
    if (!filename) return NULL;

    mesh = gf3d_mesh_get_by_filename(filename);
    if (!mesh)
    {
        mesh->_refCount++;
        return mesh;
    }
    mesh = gf3d_mesh_new();
    if (!mesh)
    {
        slog("failed to allocatea new mesh");
        return NULL;
    }
    prim = gf3d_mesh_primitive_new();
    if (!prim)
    {
        slog("failed to get new primitive for mesh");
        return 0;
    }
    prim->objData = gf3d_obj_load_from_file(filename);
    if (prim->objData)
    {
        slog("failed to parse file %s for mesh", filename);
        return NULL;
    }
    if (!gf3d_mesh_buffer_create(mesh))
    {
        slog("failed to parse file %s, cannot make buffer");
        return NULL;
    }
    return mesh;
}

/**
* @brief allocate a zero initialized mesh primitive
* @return NULL on error or the primitive
*/
MeshPrimitive *gf3d_mesh_primitive_new()
{
    MeshPrimitive *prim = gfc_allocate_array(sizeof(MeshPrimitive), 1);
    if (!prim)
    {
        slog("failed to allocate primitive memory for a mesh");
        return NULL;
    }
    return prim;
}

void gf3d_mesh_primitive_free(MeshPrimitive *prim)
{
    if (!prim) return;
    if (prim->vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device, prim->vertexBuffer, NULL);
    }
    if (prim->faceBufferMemory != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device, prim->faceBufferMemory, NULL);
    }
    if (prim->faceBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device, prim->faceBuffer, NULL);
    }
    free(prim);
}

/**
* @brief get the input attribute descriptions for mesh based rendering
* @param count (optional, output) the number of attributes
* @return a pointer to a vertex input attribute description array
*/
VkVertexInputAttributeDescription *gf3d_mesh_get_attribute_descriptions(Uint32 *count)
{
    if (count)*count = MESH_ATTRIBUTE_COUNT;

    mesh_manager.attributeDescriptions[0].binding = 0;
    mesh_manager.attributeDescriptions[0].location = 0;
    mesh_manager.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[0].offset = offsetof(Vertex, vertex);

    mesh_manager.attributeDescriptions[1].binding = 0;
    mesh_manager.attributeDescriptions[1].location = 1;
    mesh_manager.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[1].offset = offsetof(Vertex, normal);

    mesh_manager.attributeDescriptions[2].binding = 0;
    mesh_manager.attributeDescriptions[2].location = 2;
    mesh_manager.attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[2].offset = offsetof(Vertex, texel);
}

/**
* @brief get the binding description for mesh based rendering
* @return vertex input binding descriptions compatible with mesh data
*/
VkVertexInputBindingDescription *gf3d_mesh_get_bind_description()
{
    mesh_manager.bindingDescription.binding = 0;
    mesh_manager.bindingDescription.stride = sizeof(Vertex);
    mesh_manager.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return &mesh_manager.bindingDescription;
}

/**
* @brief needs to be called once at the beginning of each render frame
*/
void gf3d_mesh_reset_pipes()
{

}

/**
* @brief called to submit all draw commands to the mesh pipelines
*/
void gf3d_mesh_submit_pipe_commands()
{

}

/**
* @brief get the current command buffer for the mesh system
*/
VkCommandBuffer gf3d_mesh_get_model_command_buffer()
{

}


/**
* @brief queue up a render for the current draw frame
* @param mesh the mesh to render
* @param pipe the pipeline to use
* @param uboData the data to use to draw the mesh
* @param texture texture data to use
*/
void gf3d_mesh_queue_render(Mesh *mesh, Pipeline *pipe, void *uboData, Texture *texture)
{

}


/**
* @brief adds a mesh to the render pass rendered as an outline highlight
* @note: must be called within the render pass
* @param mesh the mesh to render
* @param com the command pool to use to handle the request we are rendering with
*/
void gf3d_mesh_render(Mesh *mesh,VkCommandBuffer commandBuffer, VkDescriptorSet *descriptorSet)
{

}

/**
* @brief render a mesh through a given pipeline
*/
void gf3d_mesh_render_generic(Mesh *mesh,Pipeline *pipe,VkDescriptorSet *descriptorSet)
{

}

/**
* @brief create a mesh's internal buffers based on vertices
* @param primitive the mesh primitive to populate
* @note the primitive must have the objData set and it must have be organizes in buffer order
*/
void gf3d_mesh_create_vertex_buffer_from_vertices(MeshPrimitive *primitive)
{

}

/**
* @brief get the pipeline that is used to render basic 3d meshes
* @return NULL on error or the pipeline in question
*/
Pipeline *gf3d_mesh_get_pipeline()
{

}

/**
* @brief given a model matrix and basic color, build the meshUBO needed to render a model
* @param modelMat the model Matrix
* @param colorMod the color for the UBO
*/
MeshUBO gf3d_mesh_get_ubo(GFC_Matrix4 modelMat, GFC_Color colorMod)
{

}