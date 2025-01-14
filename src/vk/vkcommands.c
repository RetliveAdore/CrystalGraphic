/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-08 17:17:19
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-14 12:29:22
 * @FilePath: \CrystalGraphic\src\vk\vkcommands.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crvk.h>

CRBOOL _inner_create_command_pool_(pcrvk vk)
{
    VkCommandPoolCreateInfo drawPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = vk->graphicsQueueFamilyIndex
    };
    VkCommandPoolCreateInfo presentPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = vk->presentQueueFamilyIndex
    };
    //此处不管有几个队列，都创建三个命令池
    VkResult err =
    vkCreateCommandPool(vk->device, &drawPoolInfo, NULL, &vk->drawCommandPool);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create drawCommandPool");
        goto FailedDraw;
    }
    vkCreateCommandPool(vk->device, &presentPoolInfo, NULL, &vk->presentCommandPool);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create presentCommandPool");
        return CRFALSE;
    }
    return CRTRUE;
FaildePresent:
    vkDestroyCommandPool(vk->device, vk->drawCommandPool, NULL);
FailedDraw:
    return CRFALSE;
}

void _inner_destroy_command_pool_(pcrvk vk)
{
    vkDestroyCommandPool(vk->device, vk->presentCommandPool, NULL);
    vkDestroyCommandPool(vk->device, vk->drawCommandPool, NULL);
}

CRBOOL _inner_create_command_buffers_(pcrvk vk)
{
    CRUINT32 i;
    vk->drawBuffers = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkCommandBuffer));
    if (!vk->drawBuffers)
    {
        CR_LOG_ERR("auto", "bad alloc");
        goto FailedtoAllocDraw;
    }
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = vk->drawCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = vk->swapchainImageCount
    };
    VkResult err =
    vkAllocateCommandBuffers(vk->device, &allocInfo, vk->drawBuffers);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to allocate drawing command buffers");
        goto FailedtoCreateDraw;
    }
    //
    vk->presentBuffers = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkCommandBuffer));
    if (!vk->drawBuffers)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return CRFALSE;
    }
    allocInfo.commandPool = vk->presentCommandPool;
    err = vkAllocateCommandBuffers(vk->device, &allocInfo, vk->presentBuffers);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to allocate presenting command buffers");
        goto FailedtoCreatePresent;
    }
    //
    _inner_record_commands_(vk);
    return CRTRUE;
FailedtoAllocTransfer:
    vkFreeCommandBuffers(vk->device, vk->drawCommandPool, vk->swapchainImageCount, vk->drawBuffers);
FailedtoCreatePresent:
    CRAlloc(vk->presentBuffers, 0);
FailedtoAllocPresent:
    vkFreeCommandBuffers(vk->device, vk->drawCommandPool, vk->swapchainImageCount, vk->drawBuffers);
FailedtoCreateDraw:
    CRAlloc(vk->drawBuffers, 0);
FailedtoAllocDraw:
    return CRFALSE;
}

void _inner_destroy_command_buffers_(pcrvk vk)
{
    vkFreeCommandBuffers(vk->device, vk->presentCommandPool, vk->swapchainImageCount, vk->presentBuffers);
    CRAlloc(vk->presentBuffers, 0);
    vkFreeCommandBuffers(vk->device, vk->drawCommandPool, vk->swapchainImageCount, vk->drawBuffers);
    CRAlloc(vk->drawBuffers, 0);
}

static void _inner_paint_items_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    pcrvkobj obj = (pcrvkobj)data;
    pcrvkmesh mesh = obj->prop.mesh;
    pcrvk vk = obj->vk;
    if (!mesh) return;
    CRUINT32 i = (CRUINT32)(CRUINT64)user;
    VkBuffer vertexBuffers[1] = { mesh->vertexBuffer };
    VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers(vk->drawBuffers[i], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(vk->drawBuffers[i], mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(vk->drawBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vk->pipelineLayout, 0, 1, &obj->descriptorSets[i], 0, NULL);
    vkCmdDrawIndexed(vk->drawBuffers[i], GET_INSTANCE_COUNT(mesh->indexSize, CRUINT32), 1, 0, 0, 0);
}

static void _inner_paint_groups_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    CRBLENDGROUP *pGroup = data;
    for (CRUINT64 i = 0; i < pGroup->count; i++)
    {
        pcrvkobj obj = (pcrvkobj)(pGroup->itemList[i]);
        pcrvkmesh mesh = obj->prop.mesh;
        pcrvk vk = obj->vk;
        if (!mesh) return;
        CRUINT32 i = (CRUINT32)(CRUINT64)user;
        VkBuffer vertexBuffers[1] = { mesh->vertexBuffer };
        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(vk->drawBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(vk->drawBuffers[i], mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(vk->drawBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vk->pipelineLayout, 0, 1, &obj->descriptorSets[i], 0, NULL);
        vkCmdDrawIndexed(vk->drawBuffers[i], GET_INSTANCE_COUNT(mesh->indexSize, CRUINT32), 1, 0, 0, 0);
    }
    //
}

void _inner_record_commands_(pcrvk vk)
{
    CR_LOG_IFO("auto", "recording commands");
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext= NULL,
        .flags= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo = NULL
    };
    VkClearValue clearValues[3] = {
        [0] = {
            .color = {0.0f, 0.0f, 0.0f, 1.0f}
        },
        [1] = {
            .depthStencil = {1.0f, 0}
        },
        [2] = {
            .color = {0.0f, 0.0f, 0.0f, 1.0f}
        },
    };
    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = NULL,
        .renderPass = vk->renderPass,
        .renderArea = {
            .offset = { 0, 0 },
            .extent = vk->swapchainExtent
        },
        .clearValueCount = CR_ARRAY_SIZE(clearValues),
        .pClearValues = clearValues
    };
    VkResult err;
    for (CRUINT32 i = 0; i < vk->swapchainImageCount; i++)
    {
        err = vkBeginCommandBuffer(vk->drawBuffers[i], &beginInfo);
        if (err) CR_LOG_ERR("auto", "failed to begin command buffer %d", i);
        //
        renderPassInfo.framebuffer = vk->framebuffers[i];
        vkCmdBeginRenderPass(vk->drawBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        //
        vkCmdBindPipeline(vk->drawBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vk->graphicsPipeline);
        if (CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_BLENDGROUP))
            CRTreeIterator(vk->blendGroupTree, _inner_paint_groups_callback_, (CRLVOID)(CRUINT64)i);
        else
            CRTreeIterator(vk->objectsTree, _inner_paint_items_callback_, (CRLVOID)(CRUINT64)i);
        //
        vkCmdEndRenderPass(vk->drawBuffers[i]);
        
        //
        err = vkEndCommandBuffer(vk->drawBuffers[i]);
        if (err) CR_LOG_ERR("auto", "failed to end command buffer %d", i);
    }
}
