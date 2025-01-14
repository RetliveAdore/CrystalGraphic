/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-09 14:27:51
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-14 12:44:54
 * @FilePath: \CrystalGraphic\src\graphicitems.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crwindow.h>
#include <crmatrix.h>

static CRUINT64 currentID;
static CRDYNAMIC availableID;

static inline CRUINT64 _inner_get_id_()
{
    CRUINT64 back = 0;
    CRDynPop(availableID, &back, DYN_MODE_64);
    if (back) return back;
    return currentID++;
}

static inline void _inner_put_id_(CRUINT64 id)
{
    CRDynPush(availableID, &id, DYN_MODE_64);
}

/**
 * 网格实体相关
 */

static CRBOOL _inner_create_mesh_buffer_(
    pcrvkmesh mesh,
    CRDYNAMIC vertexData,
    CRUINT64 vertexSize,
    CRDYNAMIC indexData,
    CRUINT64 indexSize
)
{
    VkBuffer stagingVertexBuffer;
    VkDeviceMemory stagingVertexBufferMemory;
    VkBuffer stagingIndexBuffer;
    VkDeviceMemory stagingIndexBufferMemory;
    //
    CRBOOL back;
    back = createBuffer(
        mesh->vk->gpu,
        mesh->vk->device,
        vertexSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingVertexBuffer,
        &stagingVertexBufferMemory
    );
    if (!back)
    {
        CR_LOG_ERR("auto", "failed to create staging vertex buffer");
        goto FailedtoCreateStagingVertex;
    }
    copyMemory(mesh->vk->device, stagingVertexBufferMemory, vertexData, vertexSize);
    //
    back = createBuffer(
        mesh->vk->gpu,
        mesh->vk->device,
        vertexSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &mesh->vertexBuffer,
        &mesh->vertexBufferMemory
    );
    if (!back)
    {
        CR_LOG_ERR("auto", "failed to create vertex buffer");
        goto FailedtoCreateVertex;
    }
    vkQueueWaitIdle(mesh->vk->graphicsQueue);
    copyBuffer(mesh->vk->drawCommandPool, mesh->vk->graphicsQueue, mesh->vk->device, stagingVertexBuffer, mesh->vertexBuffer, vertexSize);
    //
    ////////////////////////////////
    //
    back = createBuffer(
        mesh->vk->gpu,
        mesh->vk->device,
        indexSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingIndexBuffer,
        &stagingIndexBufferMemory
    );
    if (!back)
    {
        CR_LOG_ERR("auto", "failed to create staging vertex buffer");
        goto FailedtoCreateStagingIndex;
    }
    copyMemory(mesh->vk->device, stagingIndexBufferMemory, indexData, indexSize);
    //
    back = createBuffer(
        mesh->vk->gpu,
        mesh->vk->device,
        vertexSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &mesh->indexBuffer,
        &mesh->indexBufferMemory
    );
    if (!back)
    {
        CR_LOG_ERR("auto", "failed to create index buffer");
        goto FailedtoCreateIndex;
    }
    copyBuffer(mesh->vk->drawCommandPool, mesh->vk->graphicsQueue, mesh->vk->device, stagingIndexBuffer, mesh->indexBuffer, indexSize);
    //释放中转临时缓冲
    destroyBuffer(mesh->vk->device, stagingIndexBuffer, stagingIndexBufferMemory);
    destroyBuffer(mesh->vk->device, stagingVertexBuffer, stagingVertexBufferMemory);
    return CRTRUE;
FailedtoCreateIndex:
    destroyBuffer(mesh->vk->device, stagingIndexBuffer, stagingIndexBufferMemory);
FailedtoCreateStagingIndex:
    destroyBuffer(mesh->vk->device, mesh->vertexBuffer, mesh->vertexBufferMemory);
FailedtoCreateVertex:
    destroyBuffer(mesh->vk->device, stagingVertexBuffer, stagingVertexBufferMemory);
FailedtoCreateStagingVertex:
    return CRFALSE;
}

static void _inner_destroy_mesh_buffer_(pcrvkmesh mesh)
{
    destroyBuffer(mesh->vk->device, mesh->indexBuffer, mesh->indexBufferMemory);
    destroyBuffer(mesh->vk->device, mesh->vertexBuffer, mesh->vertexBufferMemory);
}

static CRGRAPHICMESH _inner_add_graphic_mesh_(
    pcrvk vk,
    CRDYNAMIC vertexData,
    CRUINT64 vertexSize,
    CRDYNAMIC indexData,
    CRUINT64 indexSize
)
{
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED)) return NULL;
    CRLock(vk->itemLock);
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED))
    {
        CRUnlock(vk->itemLock);
        return NULL;
    }
    pcrvkmesh mesh = CRAlloc(NULL, sizeof(crvkmesh));
    if (!mesh)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return NULL;
    }
    mesh->vk = vk;
    mesh->vertexSize = vertexSize;
    mesh->indexSize = indexSize;
    if (!_inner_create_mesh_buffer_(mesh, vertexData, vertexSize, indexData, indexSize))
        goto FailedtoCreateBuffer;
    //
    CRTreePut(vk->meshesTree, mesh, (CRUINT64)mesh);
    CRUnlock(vk->itemLock);
    return mesh;
FailedtoCreateBuffer:
    CRAlloc(mesh, 0);
    CRUnlock(vk->itemLock);
    return NULL;
}

static void _inner_meshes_clear_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    pcrvkmesh mesh = (pcrvkmesh)data;
    pcrvk vk = (pcrvk)user;
    //
    _inner_destroy_mesh_buffer_(mesh);
    CRAlloc(mesh, 0);
}

void _inner_remove_graphic_mesh_(pcrvk vk, CRGRAPHICMESH mesh)
{
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED)) return;
    CRLock(vk->itemLock);
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED))
    {
        CRUnlock(vk->itemLock);
        return;
    }
    vkQueueWaitIdle(vk->graphicsQueue);
    if (!CRTreeGet(vk->meshesTree, NULL, (CRUINT64)mesh))
        _inner_meshes_clear_callback_(mesh, vk, 0);
    CRUnlock(vk->itemLock);
}

CRAPI void CRAddMesh(
    CRWINDOW window,
    CRUINT32 count,
    CRDYNAMIC *pVertexData,
    CRUINT64 *pVertexSize,
    CRDYNAMIC *pIndexData,
    CRUINT64 *pIndexSize,
    CRGRAPHICMESH *pBackMeshes
)
{
    if (!window)
    {
        CR_LOG_ERR("auto", "invalid window");
        return;
    }
    if (!pBackMeshes)
    {
        CR_LOG_ERR("auto", "invalid ->pBackMeshes");
        return;
    }
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    CR_LOG_DBG("auto", "add mesh, count: %d", count);
    for (CRUINT32 i = 0; i < count; i++)
    {
        pBackMeshes[i] =
        _inner_add_graphic_mesh_(
            pInner->vk,
            pVertexData[i],
            pVertexSize[i],
            pIndexData[i],
            pIndexSize[i]
        );
    }
}

CRAPI void CRRemoveMesh(CRWINDOW window, CRUINT32 count, CRGRAPHICMESH *pMeshes)
{
    if (!window)
    {
        CR_LOG_ERR("auto", "invalid window");
        return;
    }
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    CR_LOG_DBG("auto", "remove mesh, count: %d", count);
    for (CRUINT32 i = 0; i < count; i++)
        _inner_remove_graphic_mesh_(pInner->vk, pMeshes[i]);
}

/**
 * 纹理实体相关
 */

CRGRAPHICTEXTURE _inner_add_graphic_texture_(
    pcrvk vk,
    CRDYNAMIC data,
    CRUINT32 w,
    CRUINT32 h,
    CRUINT64 size
)
{
    if (w * h * sizeof(CR_PIXEL) != size)
    {
        CR_LOG_ERR("auto", "mismatched size: w * h * sizeof(CR_PIXEL) != size");
        return NULL;
    }
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED)) return NULL;
    CRLock(vk->itemLock);
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED))
    {
        CRUnlock(vk->itemLock);
        return NULL;
    }
    pcrvktexture tex = CRAlloc(NULL, sizeof(crvktexture));
    if (!tex)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return NULL;
    }
    tex->vk = vk;
    //
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CRBOOL back =
    createBuffer(
        vk->gpu,
        vk->device,
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer,
        &stagingBufferMemory
    );
    if (!back)
    {
        CR_LOG_ERR("auto", "failed to create staging buffer");
        goto FailedtoCreateStagingBuffer;
    }
    copyMemory(vk->device, stagingBufferMemory, data, size);
    //创建纹理图像
    back = createImage(
        vk->device, vk->gpu,
        VK_SAMPLE_COUNT_1_BIT,
        w, h, CR_TEXTURE_FORMAT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &tex->textureImage,
        &tex->textureImageMemory
    );
    if (!back)
    {
        CR_LOG_ERR("auto", "failed to create texture image");
        goto FailedtoCreateTextureImage;
    }
    transitionImageLayout(
        vk->drawCommandPool,
        vk->graphicsQueue,
        vk->device,
        tex->textureImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );
    copyBufferToImage(
        vk->drawCommandPool,
        vk->graphicsQueue,
        vk->device,
        stagingBuffer,
        tex->textureImage,
        w, h
    );
    transitionImageLayout(
        vk->drawCommandPool,
        vk->graphicsQueue,
        vk->device,
        tex->textureImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    //创建纹理图像视图
    tex->textureImageViews = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkImageView));
    if (!tex->textureImageViews)
    {
        CR_LOG_ERR("auto", "bad alloc");
        goto FailedtoAllocView;        
    }
    CRUINT32 i;
    for (i = 0; i < vk->swapchainImageCount; i++)
    {
        if (!createImageView(vk->device, tex->textureImage, CR_TEXTURE_FORMAT, VK_IMAGE_ASPECT_COLOR_BIT, &tex->textureImageViews[i]))
        {
            CR_LOG_ERR("auto", "failed to create image view %d", i);
            goto FailedtoCreateView;
        }
    }
    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_ANISTROPY) ? VK_TRUE : VK_FALSE,
        .maxAnisotropy = CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_ANISTROPY) ? 16.0f : 1.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f
    };
    VkResult err =
    vkCreateSampler(vk->device, &samplerInfo, NULL, &tex->textureSampler);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create sampler");
        goto FailedtoCreateSampler;
    }
    //
    CRTreePut(vk->texturesTree, tex, (CRUINT64)tex);
    destroyBuffer(vk->device, stagingBuffer, stagingBufferMemory);
    CRUnlock(vk->itemLock);
    return tex;
FailedtoCreateSampler:
    for (i = 0; i < vk->swapchainImageCount; i++)
        vkDestroyImageView(vk->device, tex->textureImageViews[i], NULL);
    goto FailedtoAllocView;
FailedtoCreateView:
    if (i == 0) goto FailedtoAllocView;
    i--;
    for (; i >= 0; i--)
        vkDestroyImageView(vk->device, tex->textureImageViews[i], NULL);
FailedtoAllocView:
    destroyImage(vk->device, tex->textureImage, tex->textureImageMemory);
FailedtoCreateTextureImage:
    destroyBuffer(vk->device, stagingBuffer, stagingBufferMemory);
FailedtoCreateStagingBuffer:
    CRAlloc(tex, 0);
    CRUnlock(vk->itemLock);
    return NULL;
}

static void _inner_texture_clear_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    pcrvktexture tex = (pcrvktexture)data;
    pcrvk vk = (pcrvk)user;
    //
    vkDestroySampler(vk->device, tex->textureSampler, NULL);
    for (CRUINT32 i = 0; i < vk->swapchainImageCount; i++)
        vkDestroyImageView(vk->device, tex->textureImageViews[i], NULL);
    CRAlloc(tex->textureImageViews, 0);
    destroyImage(vk->device, tex->textureImage, tex->textureImageMemory);
    CRAlloc(tex, 0);
}

void _inner_remove_graphic_texture_(pcrvk vk, CRGRAPHICTEXTURE texture)
{
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED)) return;
    CRLock(vk->itemLock);
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED))
    {
        CRUnlock(vk->itemLock);
        return;
    }
    vkQueueWaitIdle(vk->graphicsQueue);
    if (!CRTreeGet(vk->texturesTree, NULL, (CRUINT64)texture))
        _inner_texture_clear_callback_(texture, vk, 0);
    CRUnlock(vk->itemLock);
}

CRAPI void CRAddTexture(
    CRWINDOW window,
    CRUINT32 count,
    CRUINT32 *pW,
    CRUINT32 *pH,
    CRUINT64 *pSizes,
    CRDYNAMIC *pDatas,
    CRGRAPHICTEXTURE *pBackTextures
)
{
    if (!window)
    {
        CR_LOG_ERR("auto", "invalid window");
        return;
    }
    if (!pBackTextures)
    {
        CR_LOG_ERR("auto", "invalid ->ppBackTextures");
        return;
    }
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    CR_LOG_DBG("auto", "add texture, count: %d", count);
    for (CRUINT32 i = 0; i < count; i++)
    {
        pBackTextures[i] =
        _inner_add_graphic_texture_(
            pInner->vk,
            pDatas[i],
            pW[i],
            pH[i],
            pSizes[i]
        );
    }
}

CRAPI void CRRemoveTexture(CRWINDOW window, CRUINT32 count, CRGRAPHICTEXTURE *pTextures)
{
    if (!window)
    {
        CR_LOG_ERR("auto", "invalid window");
        return;
    }
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    CR_LOG_DBG("auto", "remove texture, count: %d", count);
    for (CRUINT32 i = 0; i < count; i++)
        _inner_remove_graphic_texture_(pInner->vk, pTextures[i]);
}

/**
 * 绘制实体相关
 */

static CRBOOL _inner_create_descriptor_sets_(pcrvkobj obj)
{
    obj->descriptorSets = CRAlloc(NULL, obj->vk->swapchainImageCount * sizeof(VkDescriptorSet));
    if (!obj->descriptorSets)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return CRFALSE;
    }
    VkDescriptorPoolSize poolSizes[3] = {
        [0] = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = obj->vk->swapchainImageCount
        },
        [1] = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = obj->vk->swapchainImageCount
        },
        [2] = {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = obj->vk->swapchainImageCount
        }
    };
    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .poolSizeCount = CR_ARRAY_SIZE(poolSizes),
        .pPoolSizes = poolSizes,
        .maxSets = obj->vk->swapchainImageCount
    };
    VkResult err =
    vkCreateDescriptorPool(obj->vk->device, &poolInfo, NULL, &obj->descriptorPool);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create descriptor pool");
        goto FailedtoCreateDescriptorPool;
    }
    //descriptor sets
    VkDescriptorSetLayout *layouts = CRAlloc(NULL, obj->vk->swapchainImageCount * sizeof(VkDescriptorSetLayout));
    if (!layouts)
    {
        CR_LOG_ERR("auto", "bad alloc");
        goto FailedtoAllocLayouts;
    }
    for (CRUINT32 i = 0; i < obj->vk->swapchainImageCount; i++)
        layouts[i] = obj->vk->descriptorSetLayout;
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = NULL,
        .descriptorPool = obj->descriptorPool,
        .descriptorSetCount = obj->vk->swapchainImageCount,
        .pSetLayouts = layouts,
    };
    err = vkAllocateDescriptorSets(obj->vk->device, &allocInfo, obj->descriptorSets);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to allocate descriptor sets");
        goto FailedtoAllocateSets;
    }
    pcrvktexture tex = obj->prop.texture;
    for (CRUINT32 i = 0; i < obj->vk->swapchainImageCount; i++)
    {
        VkDescriptorBufferInfo bufferInfosI[1] = {
            [0] = {
                .buffer = obj->uniformBuffers[i],
                .offset = 0,
                .range = sizeof(CR_UBO)
            }
        };
        VkDescriptorBufferInfo bufferInfosII[1] = {
            [0] = {
                .buffer = obj->vk->globalUniformBuffers[i],
                .offset = 0,
                .range = sizeof(CR_GLOBAL_UBO)
            }
        };
        VkDescriptorImageInfo imageInfo[1] = {
            [0] = {
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView = tex ? tex->textureImageViews[i] : obj->vk->textureImageViews[i],
                .sampler = tex ? tex->textureSampler : obj->vk->vkSampler
            }
        };
        VkWriteDescriptorSet descriptorWrites[3] = {
            [0] = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = NULL,
                .dstSet = obj->descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = CR_ARRAY_SIZE(bufferInfosI),
                .pBufferInfo = bufferInfosI,
                .pImageInfo = NULL,
                .pTexelBufferView = NULL
            },
            [1] = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = NULL,
                .dstSet = obj->descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = CR_ARRAY_SIZE(bufferInfosII),
                .pBufferInfo = bufferInfosII,
                .pImageInfo = NULL,
                .pTexelBufferView = NULL
            },
            [2] = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = NULL,
                .dstSet = obj->descriptorSets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = CR_ARRAY_SIZE(imageInfo),
                .pBufferInfo = NULL,
                .pImageInfo = imageInfo,
                .pTexelBufferView = NULL
            }
        };
        vkUpdateDescriptorSets(obj->vk->device, CR_ARRAY_SIZE(descriptorWrites), descriptorWrites, 0, NULL);
    }
    CRAlloc(layouts, 0);
    return CRTRUE;
FailedtoAllocateSets:
    CRAlloc(layouts, 0);
FailedtoAllocLayouts:
    vkDestroyDescriptorPool(obj->vk->device, obj->descriptorPool, NULL);
FailedtoCreateDescriptorPool:
    CRAlloc(obj->descriptorSets, 0);
    return CRFALSE;
}

static void _inner_destroy_descriptor_sets_(pcrvkobj obj)
{
    vkDestroyDescriptorPool(obj->vk->device, obj->descriptorPool, NULL);
    CRAlloc(obj->descriptorSets, 0);
}

CRGRAPHICITEM _inner_add_graphic_item_(pcrvk vk, CRITEMPROP *pProp)
{
    pcrvkobj obj = CRAlloc(NULL, sizeof(crvkobject));
    if (!obj)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return NULL;
    }
    obj->vk = vk;
    memcpy(&obj->prop, pProp, sizeof(CRITEMPROP));
    //
    obj->uniformBuffers = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkBuffer));
    if (!obj->uniformBuffers)
    {
        CR_LOG_ERR("auto", "bad alloc");
        goto FailedtoAllocUniformBuffers;
    }
    obj->uniformDeviceMemories = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkDeviceMemory));
    if (!obj->uniformDeviceMemories)
    {
        CR_LOG_ERR("auto", "bad alloc");
        goto FailedtoAllocateUniformDeviceMemories;
    }
    CRUINT32 i; 
    for (i = 0; i < vk->swapchainImageCount; i++)
    {
        if (!createBuffer(
                vk->gpu,
                vk->device,
                sizeof(CR_UBO),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &obj->uniformBuffers[i],
                &obj->uniformDeviceMemories[i]
            )
        )
        {
            CR_LOG_ERR("auto", "failed to create buffer %d", i);
            goto FailedtoCreateUniformBuffer;
        }
    }
    //
    if (!_inner_create_descriptor_sets_(obj))
    {
        CR_LOG_ERR("auto", "failed to create descriptor pool");
        goto FailedtoCreateDescriptorPool;
    }
    //
    CRTreePut(vk->objectsTree, obj, (CRUINT64)obj);
    return obj;
FailedtoCreateDescriptorPool:
    for (i = 0; i < vk->swapchainImageCount; i++)
        destroyBuffer(vk->device, obj->uniformBuffers[i], obj->uniformDeviceMemories[i]);
    goto FailedtoAllocateUniformDeviceMemories;
FailedtoCreateUniformBuffer:
    if (i == 0) goto FailedtoAllocateUniformDeviceMemories;
    i--;
    for (; i >= 0; i--)
        destroyBuffer(vk->device, obj->uniformBuffers[i], obj->uniformDeviceMemories[i]);
    CRAlloc(obj->uniformDeviceMemories, 0);
FailedtoAllocateUniformDeviceMemories:
    CRAlloc(obj->uniformBuffers, 0);
FailedtoAllocUniformBuffers:
    CRAlloc(obj, 0);
    return NULL;
}

static void _inner_update_uniform_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    pcrvkobj obj = (pcrvkobj)data;
    pcrvk vk = obj->vk;
    CRUINT64 i = (CRUINT64)user;
    //
    CR_UBO ubo;
    //
    _inner_set_zoom_(&ubo.model, obj->prop.zoom);
    crMat4RotateZ(&ubo.model, obj->prop.angle);
    crMat4MoveXY(&ubo.model, obj->prop.x, obj->prop.y);
    //
    ubo.color[0] = obj->prop.color.r;
    ubo.color[1] = obj->prop.color.g;
    ubo.color[2] = obj->prop.color.b;
    ubo.color[3] = obj->prop.color.a;
    //
    ubo.uvPos[0] = obj->prop.uvX;
    ubo.uvPos[1] = obj->prop.uvY;
    //
    void *mapData;
    VkResult err =
    vkMapMemory(vk->device, obj->uniformDeviceMemories[i], 0, sizeof(CR_UBO), 0, &mapData);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to map memory, key: %d, i: %d", key, i);
        return;
    }
    memcpy(mapData, &ubo, sizeof(CR_UBO));
    vkUnmapMemory(vk->device, obj->uniformDeviceMemories[i]);
}

void _inner_update_uniform_buffer_(pcrvk vk, CRUINT32 imageIndex)
{
    CRTreeIterator(vk->objectsTree, _inner_update_uniform_callback_, (CRLVOID)(CRUINT64)imageIndex);
}

static void _inner_object_clear_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    pcrvkobj obj = (pcrvkobj)data;
    pcrvk vk = (pcrvk)user;
    _inner_destroy_descriptor_sets_(obj);
    for (CRUINT32 i = 0; i < vk->swapchainImageCount; i++)
        destroyBuffer(vk->device, obj->uniformBuffers[i], obj->uniformDeviceMemories[i]);
    CRAlloc(obj->uniformDeviceMemories, 0);
    CRAlloc(obj->uniformBuffers, 0);
    CRAlloc(obj, 0);
}

void _inner_remove_graphic_item_(pcrvk vk, CRGRAPHICITEM item)
{
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED)) return;
    CRLock(vk->itemLock);
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED))
    {
        CRUnlock(vk->itemLock);
        return;
    }
    vkQueueWaitIdle(vk->graphicsQueue);
    if (!CRTreeGet(vk->objectsTree, NULL, (CRUINT64)item))
        _inner_object_clear_callback_(item, vk, 0);
    CRUnlock(vk->itemLock);
}

CRAPI void CRAddGraphicItem(
    CRWINDOW window,
    CRUINT32 itemCount,
    CRGRAPHICITEM *pItemBack,
    CRITEMPROP *pProps
)
{
    if (!window)
    {
        CR_LOG_ERR("auto", "invalid window");
        return;
    }
    if (!pProps)
    {
        CR_LOG_ERR("auto", "no properties given");
        return;
    }
    if (!pItemBack)
    {
        CR_LOG_WAR("auto", "invalid back array");
        return;
    }
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    CR_LOG_DBG("auto", "add graphic item, count: %d", itemCount);
    for (CRUINT32 i = 0; i < itemCount; i++)
        if (pItemBack) pItemBack[i] = _inner_add_graphic_item_(pInner->vk, &pProps[i]);
    pInner->vk->flags |= CRVK_FLAG_1_CMDRECORD;
}

CRAPI void CRRemoveGraphicItem(CRWINDOW window, CRUINT32 itemCount, CRGRAPHICITEM *pItem)
{
    if (!window)
    {
        CR_LOG_ERR("auto", "invalid window");
        return;
    }
    CR_LOG_DBG("auto", "remove graphic item, count: %d", itemCount);
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    for (CRUINT32 i = 0; i < itemCount; i++)
        _inner_remove_graphic_item_(pInner->vk, pItem[i]);
    pInner->vk->flags |= CRVK_FLAG_1_CMDRECORD;
}

CRAPI void CRUpdateItemProp(CRUINT32 itemCount, CRGRAPHICITEM *pItems, CRITEMPROP *pProps)
{
    for (CRUINT32 i = 0; i < itemCount; i++)
    {
        pcrvkobj obj = (pcrvkobj)pItems[i];
        memcpy(&obj->prop, &pProps[i], sizeof(CRITEMPROP));
    }
}

/**
 * 创建渲染组
 */

static void _inner_add_blend_group_(pcrvk vk, CRBLENDGROUP *pGroup)
{
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED)) return;
    CRLock(vk->itemLock);
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED))
    {
        CRUnlock(vk->itemLock);
        return;
    }
    CRBLENDGROUP *pNew = CRAlloc(NULL, sizeof(CRBLENDGROUP));
    if (!pNew)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return;
    }
    pNew->count = pGroup->count;
    pNew->level = pGroup->level;
    pNew->itemList = CRAlloc(NULL, pNew->count * sizeof(CRBLENDGROUP));
    if (!pNew->itemList)
    {
        CR_LOG_ERR("auto", "bad alloc");
        CRAlloc(pNew, 0);
        return;
    }
    memcpy(pNew->itemList, pGroup->itemList, pNew->count * sizeof(CRBLENDGROUP));
    CRBLENDGROUP *pOld;
    if (!CRTreeGet(vk->blendGroupTree, (CRLVOID*)&pOld, pNew->level))
    {
        CRAlloc(pOld->itemList, 0);
        CRAlloc(pOld, 0);
    }
    if (CRTreePut(vk->blendGroupTree, pNew, pNew->level))
    {
        CR_LOG_ERR("auto", "faile to put group");
        CRAlloc(pNew->itemList, 0);
        CRAlloc(pNew, 0);
        return;
    }
    vk->flags |= CRVK_FLAG_1_BLENDGROUP;
    CRUnlock(vk->itemLock);
}

static void _inner_group_clear_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    CRBLENDGROUP *pGroup = data;
    pcrvk vk = (pcrvk)user;
    CRAlloc(pGroup->itemList, 0);
    CRAlloc(pGroup, 0);
}

static void _inner_remove_blend_group_(pcrvk vk, CRUINT64 level)
{
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED)) return;
    CRLock(vk->itemLock);
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED))
    {
        CRUnlock(vk->itemLock);
        return;
    }
    CRBLENDGROUP *pGroup;
    if (!CRTreeGet(vk->blendGroupTree, (CRLVOID*)&pGroup, level))
    {
        CRAlloc(pGroup->itemList, 0);
        CRAlloc(pGroup, 0);
    }
    CRUnlock(vk->itemLock);
}

CRAPI void CRAddBlendGroup(CRWINDOW window, CRUINT32 count, CRBLENDGROUP *pGroups)
{
    if (!window)
    {
        CR_LOG_ERR("auto", "invalid window");
        return;
    }
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    CR_LOG_DBG("auto", "add blend group, count: %d", count);
    for (CRUINT32 i = 0; i < count; i++)
        _inner_add_blend_group_(pInner->vk, &pGroups[i]);
    pInner->vk->flags |= CRVK_FLAG_1_CMDRECORD;
}

CRAPI void CRRemoveBlendGroup(CRWINDOW window, CRUINT32 count, CRUINT64 *pLevels)
{
    if (!window)
    {
        CR_LOG_ERR("auto", "invalid window");
        return;
    }
    CR_LOG_DBG("auto", "remove blend group, count: %d", count);
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    for (CRUINT32 i = 0; i < count; i++)
        _inner_remove_blend_group_(pInner->vk, pLevels[i]);
    pInner->vk->flags |= CRVK_FLAG_1_CMDRECORD;
}

/**
 * 创建实体树
 */

CRBOOL _inner_create_trees_(pcrvk vk)
{
    currentID = 1;
    availableID = CRDyn(0);
    if (!availableID)
    {
        CR_LOG_ERR("auto", "failed to create availablle id pool");
        goto FailedtoInitID;
    }
    CR_LOG_IFO("auto", "create graphic items tree");
    vk->objectsTree = CRTree();
    if (!vk->objectsTree)
    {
        CR_LOG_ERR("auto", "failed to create objects tree");
        goto FailedtoCreateObjectsTree;
    }
    vk->meshesTree = CRTree();
    if (!vk->meshesTree)
    {
        CR_LOG_ERR("auto", "failed to create meshes tree");
        goto FailedtoCreateMeshesTree;
    }
    vk->texturesTree = CRTree();
    if (!vk->texturesTree)
    {
        CR_LOG_ERR("auto", "failed to create meshes tree");
        goto FailedtoCreateTexturesTree;
    }
    vk->blendGroupTree = CRTree();
    if (!vk->blendGroupTree)
    {
        CR_LOG_ERR("auto", "failed to create blend groups tree");
        goto FailedtoCreateBlendGroupsTree;
    }
    return CRTRUE;
FailedtoCreateBlendGroupsTree:
    CRFreeTree(vk->texturesTree);
FailedtoCreateTexturesTree:
    CRFreeTree(vk->meshesTree);
FailedtoCreateMeshesTree:
    CRFreeTree(vk->objectsTree);
FailedtoCreateObjectsTree:
FailedtoInitID:
    return CRFALSE;
}

void _inner_destroy_trees_(pcrvk vk)
{
    CR_LOG_IFO("auto", "destroy graphic items tree");
    CRTreeIterator(vk->blendGroupTree, _inner_group_clear_callback_, vk);
    CRFreeTree(vk->blendGroupTree);
    CRTreeIterator(vk->texturesTree, _inner_texture_clear_callback_, vk);
    CRFreeTree(vk->texturesTree);
    CRTreeIterator(vk->meshesTree, _inner_meshes_clear_callback_, vk);
    CRFreeTree(vk->meshesTree);
    CRTreeIterator(vk->objectsTree, _inner_object_clear_callback_, vk);
    CRFreeTree(vk->objectsTree);
    CRFreeDyn(availableID);
}
