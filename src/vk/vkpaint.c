/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-08 23:13:29
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-13 17:51:34
 * @FilePath: \CrystalGraphic\src\vk\vkpaint.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crvk.h>

CRBOOL _inner_create_synchronization_(pcrvk vk)
{
    CR_LOG_IFO("auto", "create vulkan synchronization");
    vk->imageAvailableSemaphores = createSemaphores(vk->device, vk->maxFrames);
    if (!vk->imageAvailableSemaphores)
        goto FailedtoCreateWaitSemaphores;
    vk->renderFinishedSemaphores = createSemaphores(vk->device, vk->maxFrames);
    if (!vk->renderFinishedSemaphores)
        goto FailedtoCreateSignalSemaphores;
    vk->frontFences = createFences(vk->device, vk->maxFrames);
    if (!vk->frontFences)
        goto FailedtoCreateFrontFences;
    return CRTRUE;
FailedtoCreateFrontFences:
    deleteSemaphores(vk->device, vk->renderFinishedSemaphores, vk->maxFrames);
FailedtoCreateSignalSemaphores:
    deleteSemaphores(vk->device, vk->imageAvailableSemaphores, vk->maxFrames);
FailedtoCreateWaitSemaphores:
    return CRFALSE;
}

void _inner_destroy_synchronization_(pcrvk vk)
{
    CR_LOG_IFO("auto", "destroy vulkan synchronization");
    deleteFences(vk->device, vk->frontFences, vk->maxFrames);
    deleteSemaphores(vk->device, vk->renderFinishedSemaphores, vk->maxFrames);
    deleteSemaphores(vk->device, vk->imageAvailableSemaphores, vk->maxFrames);
}

void _inner_paint_(pcrvk vk)
{
    vk->flags |= CRVK_FLAG_1_RENDERING;
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_INITED))
    {
        vk->flags &= CRVK_FLAG_0_RENDERING;
        return;
    }
    CRLock(vk->itemLock);
    VkResult err = 0;
    vkWaitForFences(vk->device, 1, &vk->frontFences[vk->currentFrame], VK_TRUE, UINT64_MAX);
    CRUINT32 imageIndex = 0;
    err = vkAcquireNextImageKHR(vk->device, vk->swapchain, UINT64_MAX, vk->imageAvailableSemaphores[vk->currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR || CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_RESIZE))
    {
        vk->flags &= CRVK_FLAG_0_RESIZE;
        CR_LOG_DBG("auto", "swapchain needs recreate");
        vkDeviceWaitIdle(vk->device);
        vkQueueWaitIdle(vk->graphicsQueue);
        vkQueueWaitIdle(vk->presentQueue);
        //
        _inner_destroy_synchronization_(vk);
        _inner_destroy_swapchain_(vk);
        _inner_create_swapchain_(vk);
        _inner_create_synchronization_(vk);
        //
        _inner_update_global_descriptor_(vk);
        CRUnlock(vk->itemLock);
        return;
    }
    else if (err) CR_LOG_ERR("auto", "failed to acquire next image");

    VkPipelineStageFlags pipelineStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    if (CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_CMDRECORD))
    {
        vk->flags &= CRVK_FLAG_0_CMDRECORD;
        vkWaitForFences(vk->device, vk->maxFrames, vk->frontFences, VK_TRUE, UINT64_MAX);
        _inner_record_commands_(vk);
    }
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = VK_NULL_HANDLE,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vk->imageAvailableSemaphores[vk->currentFrame],
        .pWaitDstStageMask = &pipelineStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &vk->drawBuffers[imageIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &vk->renderFinishedSemaphores[vk->currentFrame]
    };
    vkResetFences(vk->device, 1, &vk->frontFences[vk->currentFrame]);
    if (CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_FLUSH))
    {
        vk->flags &= CRVK_FLAG_0_FLUSH;
        _inner_update_global_descriptor_(vk);
    }
    _inner_update_uniform_buffer_(vk, imageIndex);
    err = vkQueueSubmit(vk->graphicsQueue, 1, &submitInfo, vk->frontFences[vk->currentFrame]);
    if (err) CR_LOG_ERR("auto", "failed to submit commands");
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = VK_NULL_HANDLE,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vk->renderFinishedSemaphores[vk->currentFrame],
        .swapchainCount = 1,
        .pSwapchains = &vk->swapchain,
        .pImageIndices = &imageIndex,
        .pResults = VK_NULL_HANDLE
    };
    vkQueuePresentKHR(vk->presentQueue, &presentInfo);

    vk->currentFrame = (vk->currentFrame + 1) % vk->maxFrames;
    CRUnlock(vk->itemLock);
    vk->flags &= CRVK_FLAG_0_RENDERING;
}