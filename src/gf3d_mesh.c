#include "simple_logger.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_commands.h"
#include "gf3d_obj_load.h"
#include "model.h"
#include "gf3d_mesh.h"

#define MESH_ATTRIBUTE_COUNT 3

typedef struct
{
    Uint32      meshCount;      /**<maximum concurrent meshes supported*/
    Mesh        *meshList;      /**<pre-allocated space for meshes*/
    VkDevice    device;         /**<logical vulcan device*/
    VkVertexInputAttributeDescription   attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
} MeshManager;

static MeshManager mesh_manager = {0};

void gf3d_mesh_close();
void gf3d_mesh_delete(Mesh *mesh);
void gf3d_mesh_primitive_free(MeshPrimitive *prim);
//int gf3d_mesh_buffer_create(Mesh *mesh);
int gf3d_mesh_obj_buffer_create(Mesh *mesh, MeshPrimitive *prim);
int gf3d_mesh_primitive_buffer_create(MeshPrimitive *prim);


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
    mesh_manager.meshList = (Mesh *)gfc_allocate_array(sizeof(Mesh), mesh_max);
    if (!mesh_manager.meshList)
    {
        slog("failed to allocate mesh list");
        return;
    }
    mesh_manager.meshCount = mesh_max;
    slog("mesh manager initialized");
    atexit(gf3d_mesh_close);
}

void gf3d_mesh_close()
{
    int i;
    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        if (mesh_manager.meshList[i]._refCount > 0) gf3d_mesh_delete(&mesh_manager.meshList[i]);
    }
    free(mesh_manager.meshList);
    memset(&mesh_manager, 0, sizeof(MeshManager));
    slog("mesh manager closed");
}

Mesh *gf3d_mesh_new()
{
    int i;
    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        if ((mesh_manager.meshList[i]._refCount == 0) && (strlen(mesh_manager.meshList[i].filename) == 0))
        {
            mesh_manager.meshList[i].primitives = gfc_list_new();
            if (!mesh_manager.meshList[i].primitives)
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

void gf3d_mesh_free(Mesh *mesh)
{
    if (!mesh) return;

    mesh->_refCount--;
    if (mesh->_refCount <= 0) gf3d_mesh_delete(mesh);
}

void gf3d_mesh_delete(Mesh *mesh)
{
    int i, c;
    MeshPrimitive *prim;

    if (!mesh) return;

    c = gfc_list_get_count(mesh->primitives);
    for (i = 0; i < c; i++)
    {
        prim = gfc_list_get_nth(mesh->primitives, i);
        if (!prim) continue;
        gf3d_mesh_primitive_free(prim);
    }
    gfc_list_delete(mesh->primitives);
}

Mesh *gf3d_mesh_load_obj(const char *filename)
{
    Mesh *mesh;
    MeshPrimitive *prim;
    if (!filename) return NULL;

    mesh = gf3d_mesh_get_by_filename(filename);
    if (mesh)
    {
        mesh->_refCount++;
        return mesh;
    }
    mesh = gf3d_mesh_new();
    if (!mesh)
    {
        slog("failed to allocate a new mesh");
        return NULL;
    }
    prim = gf3d_mesh_primitive_new();
    if (!prim)
    {
        slog("failed to get new primitive for mesh");
        return NULL;
    }
    prim->objData = gf3d_obj_load_from_file(filename);
    if (!prim->objData)
    {
        slog("failed to parse file %s for mesh", filename);
        return NULL;
    }
    if (!gf3d_mesh_obj_buffer_create(mesh, prim))
    {
        slog("failed to parse file %s, cannot make buffer");
        return NULL;
    }
    return mesh;
}

Mesh *gf3d_mesh_get_by_filename(const char *filename)
{
    int i;
    if (!filename) return NULL;
    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        if (strlen(mesh_manager.meshList[i].filename) == 0) continue;
        if (gfc_strlcmp(mesh_manager.meshList[i].filename, filename) == 0)
        {
            return &mesh_manager.meshList[i];
        }
    }
    return NULL;
}

/*
int gf3d_mesh_buffer_create(Mesh *mesh)
{
    int i, c;
    MeshPrimitive *prim;

    if (!mesh) return 0;
    c = gfc_list_get_count(mesh->primitives);
    for (i = 0; i < c; i++)
    {
        prim = gfc_list_get_nth(mesh->primitives, i);
        if (!prim) continue;
        if (!gf3d_mesh_obj_buffer_create(mesh, prim)) return 0;
    }
    return 1;
}
*/

int gf3d_mesh_obj_buffer_create(Mesh *mesh, MeshPrimitive *prim)
{
    if ((!mesh) || (!prim)) return 0;
    
    if (gf3d_mesh_primitive_buffer_create(prim)) 
    {
        gfc_list_append(mesh->primitives, prim);
        return 1;
    }
    return 0;
}

int gf3d_mesh_primitive_buffer_create(MeshPrimitive *prim)
{
    void *data;
    Uint32 bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if ((!prim) || (!prim->objData)) return 0;

    // Face Buffers

    data = NULL;
    bufferSize = sizeof(Face) * prim->objData->face_count;

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(mesh_manager.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, prim->objData->outFace, (size_t) bufferSize);
    vkUnmapMemory(mesh_manager.device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->faceBuffer, &prim->faceBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->faceBuffer, bufferSize);

    vkDestroyBuffer(mesh_manager.device, stagingBuffer, NULL);
    vkFreeMemory(mesh_manager.device, stagingBufferMemory, NULL);

    // Vertex Buffers

    data = NULL;
    bufferSize = sizeof(Vertex) * prim->objData->face_vert_count;

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(mesh_manager.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, prim->objData->faceVertices, (size_t) bufferSize);
    vkUnmapMemory(mesh_manager.device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->vertexBuffer, &prim->vertexBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->vertexBuffer, bufferSize);

    vkDestroyBuffer(mesh_manager.device, stagingBuffer, NULL);
    vkFreeMemory(mesh_manager.device, stagingBufferMemory, NULL);

    prim->vertexCount = prim->objData->face_vert_count;
    prim->faceCount = prim->objData->face_count;

    return 1;
}

MeshPrimitive *gf3d_mesh_primitive_new()
{
    MeshPrimitive *prim = gfc_allocate_array(sizeof(MeshPrimitive), 1);
    if (!prim)
    {
        slog("failed to allocate primitive memory for a mesh primitive");
        return NULL;
    }
    return prim;
}

void gf3d_mesh_primitive_free(MeshPrimitive *prim)
{
    if (!prim) return;
    if (prim->vertexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(mesh_manager.device, prim->vertexBufferMemory, NULL);
    }
    if (prim->vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device, prim->vertexBuffer, NULL);
    }
    if (prim->faceBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(mesh_manager.device, prim->faceBufferMemory, NULL);
    }
    if (prim->faceBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device, prim->faceBuffer, NULL);
    }
    if (prim->objData)
    {
        gf3d_obj_free(prim->objData);
    }
    free(prim);
}

VkVertexInputAttributeDescription *gf3d_mesh_get_attribute_descriptions(Uint32 *count)
{
    if (count) *count = MESH_ATTRIBUTE_COUNT;

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
    mesh_manager.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    mesh_manager.attributeDescriptions[2].offset = offsetof(Vertex, texel);

    return mesh_manager.attributeDescriptions;
}

VkVertexInputBindingDescription *gf3d_mesh_get_bind_description()
{
    mesh_manager.bindingDescription.binding = 0;
    mesh_manager.bindingDescription.stride = sizeof(Vertex);
    mesh_manager.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return &mesh_manager.bindingDescription;
}

void gf3d_mesh_set_device(VkDevice device)
{
    mesh_manager.device = device;
}

void gf3d_mesh_queue_render(Mesh *mesh, Pipeline *pipe, void *uboData, Texture *texture)
{
    int i, c;
    MeshPrimitive *prim;

    if (!mesh) return;
    if (!texture) return;
    if (!uboData) return;
    if (!pipe) return;

    c = gfc_list_get_count(mesh->primitives);
    for (i = 0; i < c; i++)
    {
        prim = gfc_list_get_nth(mesh->primitives, i);
        if (!prim) continue;
        gf3d_pipeline_queue_render(
            pipe,
            prim->vertexBuffer,
            prim->vertexCount,
            prim->faceBuffer,
            uboData,
            texture);
    }
}