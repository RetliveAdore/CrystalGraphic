/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-06 09:20:12
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-14 21:05:40
 * @FilePath: \CrystalGraphic\src\vk\crvk.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crvk.h>
#include <crmatrix.h>

pcrvk _inner_create_crvk_(
#ifdef CR_WINDOWS
    HWND window,
#elif defined CR_LINUX
    Display *dpy,
    Window window,
#endif
    CRUINT32 w, CRUINT32 h,
    CRUINT32 titlebarHeight,
    CRBOOL depthEnable
)
{
    pcrvk vk = CRAlloc(NULL, sizeof(crvk));
    if (!vk)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return NULL;
    }
    //
    memset(vk, 0, sizeof(crvk));
    vk->itemLock = CRLockCreate();
    if (!vk->itemLock)
    {
        CR_LOG_ERR("auto", "failed to create item lock");
        goto FailedtoCreateLock;
    }
#ifdef CR_BUILD_DEBUG
    vk->flags |= CRVK_FLAG_1_VALIDATION;
#endif
    vk->window = window;
#ifdef CR_LINUX
    vk->dpy = dpy;
#endif
    vk->w = w;
    vk->h = h + titlebarHeight;
    vk->msgCallback = NULL;
    vk->maxFrames = 2;
    vk->currentFrame = 0;
    vk->titlebarHeight = titlebarHeight;
    vk->flags |= depthEnable ? CRVK_FLAG_1_ENABLEDEPTH : CRVK_FLAG_0_0;
    vk->globalZoom = 1.0f;
    //
    if (!_inner_init_vk_device_(vk))
    {
        CR_LOG_ERR("auto", "failed to init vulkan device");
        goto FailedtoCreateDevice;
    }
    if (!_inner_create_logical_device_(vk))
        goto FailedtoCreateLogicalDevice;
    if (!_inner_setup_vulkan_(vk))
    {
        CR_LOG_ERR("auto", "failed to setup vulkan runtime");
        goto FailedtoSetup;
    }
    if (!_inner_create_descriptor_set_layout_(vk))
    {
        CR_LOG_ERR("auto", "failed to create descriptor set layout");
        goto FailedtoCreateDescriptorSetLayout;
    }
    if (!_inner_query_swapchain_image_count_(vk))
    {
        CR_LOG_ERR("auto", "failed to query swapchain image count");        
        goto FailedtoCreateSwapchain;
    }
    if (!_inner_create_swapchain_(vk))
    {
        CR_LOG_ERR("auto", "failed to create swapchain");        
        goto FailedtoCreateSwapchain;
    }
    if (!_inner_create_synchronization_(vk))
    {
        CR_LOG_ERR("auto", "failed to create semaphores");
        goto FailedtoCreateSynchronization;
    }
    if (!_inner_create_trees_(vk))
    {
        CR_LOG_ERR("auto", "failed to create item trees");
        goto FailedtoCreateTrees;
    }
    if (!_inner_create_global_descriptor_(vk))
    {
        CR_LOG_ERR("auto", "failed to create global descriptor");
        goto FailedtoCreateDescriptor;
    }
    CR_LOG_DBG("auto", "create vk succeed");
    vk->flags |= CRVK_FLAG_1_INITED;
    return vk;
FailedtoCreateDescriptor:
    _inner_destroy_trees_(vk);
FailedtoCreateTrees:
    _inner_destroy_synchronization_(vk);
FailedtoCreateSynchronization:
    _inner_destroy_swapchain_(vk);
FailedtoCreateSwapchain:
    _inner_destroy_descriptor_set_layout_(vk);
FailedtoCreateDescriptorSetLayout:
    _inner_setdown_vulkan_(vk);
FailedtoSetup:
    _inner_destroy_logical_device_(vk);
FailedtoCreateLogicalDevice:
    _inner_uninit_vk_device_(vk);
FailedtoCreateDevice:
    CRLockRelease(vk->itemLock);
FailedtoCreateLock:
    CRAlloc(vk, 0);
    return NULL;
}

void _inner_destroy_crvk_(pcrvk vk)
{
    vk->flags &= CRVK_FLAG_0_INITED;
    while (CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_RENDERING)) CRSleep(1);
    CRLock(vk->itemLock);
    CRUnlock(vk->itemLock);
    vkDeviceWaitIdle(vk->device);
    vkQueueWaitIdle(vk->graphicsQueue);
    vkQueueWaitIdle(vk->presentQueue);
    _inner_destroy_global_descriptor_(vk);
    _inner_destroy_trees_(vk);
    _inner_destroy_synchronization_(vk);
    _inner_destroy_swapchain_(vk);
    _inner_destroy_descriptor_set_layout_(vk);
    _inner_setdown_vulkan_(vk);
    _inner_destroy_logical_device_(vk);
    _inner_uninit_vk_device_(vk);
    CRLockRelease(vk->itemLock);
    CRAlloc(vk, 0);
}

void _inner_resize_vk_(pcrvk vk, CRUINT32 w, CRUINT32 h)
{
    vk->w = w;
    vk->h = h;
    vk->flags |= CRVK_FLAG_1_RESIZE;
}

const CR_PIXEL whitePixel = {
    .r = 255,
    .g = 255,
    .b = 255,
    .a = 255
};

static CRBOOL _inner_create_default_texture_(pcrvk vk)
{
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CRBOOL back =
    createBuffer(
        vk->gpu,
        vk->device,
        sizeof(whitePixel),
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
    char *mapData;
    VkResult err =
    vkMapMemory(vk->device, stagingBufferMemory, 0, sizeof(whitePixel), 0, (void**)&mapData);
    if (err) CR_LOG_ERR("auto", "failed to map memory");
    memcpy(mapData, &whitePixel, sizeof(whitePixel));
    vkUnmapMemory(vk->device, stagingBufferMemory);
    //创建纹理图像
    back = createImage(
        vk->device, vk->gpu,
        VK_SAMPLE_COUNT_1_BIT,
        1, 1, CR_TEXTURE_FORMAT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &vk->textureImage,
        &vk->textureImageMemory
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
        vk->textureImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );
    copyBufferToImage(
        vk->drawCommandPool,
        vk->graphicsQueue,
        vk->device,
        stagingBuffer,
        vk->textureImage,
        1, 1
    );
    transitionImageLayout(
        vk->drawCommandPool,
        vk->graphicsQueue,
        vk->device,
        vk->textureImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    //创建纹理图像视图
    vk->textureImageViews = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkImageView));
    if (!vk->textureImageViews)
    {
        CR_LOG_ERR("auto", "bad alloc");
        goto FailedtoAllocView;        
    }
    CRUINT32 i;
    for (i = 0; i < vk->swapchainImageCount; i++)
    {
        if (!createImageView(vk->device, vk->textureImage, CR_TEXTURE_FORMAT, VK_IMAGE_ASPECT_COLOR_BIT, &vk->textureImageViews[i]))
        {
            CR_LOG_ERR("auto", "failed to create image view %d", i);
            goto FailedtoCreateView;
        }
    }
    destroyBuffer(vk->device, stagingBuffer, stagingBufferMemory);
    return CRTRUE;
FailedtoCreateView:
    if (i == 0) goto FailedtoAllocView;
    i--;
    for (; i >= 0; i--)
        vkDestroyImageView(vk->device, vk->textureImageViews[i], NULL);
    CRAlloc(vk->textureImageViews, 0);
FailedtoAllocView:
    destroyImage(vk->device, vk->textureImage, vk->textureImageMemory);
FailedtoCreateTextureImage:
    destroyBuffer(vk->device, stagingBuffer, stagingBufferMemory);
FailedtoCreateStagingBuffer:
    return CRFALSE;
}

static void _inner_destroy_default_texture_(pcrvk vk)
{
    for (CRUINT32 i = 0; i < vk->swapchainImageCount; i++)
        vkDestroyImageView(vk->device, vk->textureImageViews[i], NULL);
    CRAlloc(vk->textureImageViews, 0);
    destroyImage(vk->device, vk->textureImage, vk->textureImageMemory);
}

CRBOOL _inner_create_global_descriptor_(pcrvk vk)
{
    //创建缓冲
    vk->globalUniformBuffers = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkBuffer));
    if (!vk->globalUniformBuffers)
    {
        CR_LOG_ERR("auto", "bad alloc");
        goto FailedtoAllocUniformBuffer;
    }
    vk->globalUniformDeviceMemories = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkDeviceMemory));
    if (!vk->globalUniformDeviceMemories)
    {
        CR_LOG_ERR("auto", "bad alloc");
        goto FailedtoAllocMemories;
    }
    CRUINT32 i; 
    for (i = 0; i < vk->swapchainImageCount; i++)
    {
        if (!createBuffer(
                vk->gpu,
                vk->device,
                sizeof(CR_GLOBAL_UBO),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &vk->globalUniformBuffers[i],
                &vk->globalUniformDeviceMemories[i]
            )
        )
        {
            CR_LOG_ERR("auto", "failed to create buffer %d", i);
            goto FailedtoCreateUniformBuffer;
        }
    }
    //创建默认纹理
    if (!_inner_create_default_texture_(vk))
    {
        CR_LOG_ERR("auto", "failed to create default texture");
        goto FailedtoCreateDefaultTexture;
    }
    _inner_update_global_descriptor_(vk);
    return CRTRUE;
FailedtoCreateDefaultTexture:
    for (i = 0; i < vk->swapchainImageCount; i++)
        destroyBuffer(vk->device, vk->globalUniformBuffers[i], vk->globalUniformDeviceMemories[i]);
    goto FailedtoAllocMemories;
FailedtoCreateUniformBuffer:
    if (i == 0) goto FailedtoAllocMemories;
    i--;
    for (; i >= 0; i--)
        destroyBuffer(vk->device, vk->globalUniformBuffers[i], vk->globalUniformDeviceMemories[i]);
    CRAlloc(vk->globalUniformDeviceMemories, 0);
FailedtoAllocMemories:
    CRAlloc(vk->globalUniformBuffers, 0);
FailedtoAllocUniformBuffer:
    return CRFALSE;
}

void  _inner_destroy_global_descriptor_(pcrvk vk)
{
    _inner_destroy_default_texture_(vk);
    for (CRUINT32 i = 0; i < vk->swapchainImageCount; i++)
        destroyBuffer(vk->device, vk->globalUniformBuffers[i], vk->globalUniformDeviceMemories[i]);
    CRAlloc(vk->globalUniformDeviceMemories, 0);
    CRAlloc(vk->globalUniformBuffers, 0);
}

void _inner_update_global_descriptor_(pcrvk vk)
{
    //以短边为基准
    float ratioX = 1.0f;
    float ratioY = 1.0f;
    if (vk->w > vk->h)
        ratioX = (float)(vk->h) / (float)(vk->w);
    else if (vk->w < vk->h)
        ratioX = (float)(vk->w) / (float)(vk->h);
    CR_GLOBAL_UBO gUbo;
    //
    _inner_setup_mat_4_(&gUbo.view);
    //
    gUbo.ratio[0] = ratioX;
    gUbo.ratio[1] = ratioY;
    gUbo.ratio[2] = 1.0f;
    gUbo.ratio[3] = vk->globalZoom;
    gUbo.colorFlag[0] = CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_SRGB) ? 0.0f : 1.0f;
    //
    void *mapData;
    VkResult err;
    for (CRUINT32 i = 0; i < vk->swapchainImageCount; i++)
    {
        err = vkMapMemory(vk->device, vk->globalUniformDeviceMemories[i], 0, sizeof(CR_UBO), 0, &mapData);
        if (err)
        {
            CR_LOG_ERR("auto", "failed to map memory, %d", i);
            continue;
        }
        memcpy(mapData, &gUbo, sizeof(CR_GLOBAL_UBO));
        vkUnmapMemory(vk->device, vk->globalUniformDeviceMemories[i]);
    }
}

void _inner_set_vk_zoom_(pcrvk vk, float zoom)
{
    vk->globalZoom = zoom;
    vk->flags |= CRVK_FLAG_1_FLUSH;
}

void _inner_set_titlebar_(pcrvk vk, CRBOOL draw)
{
    if (draw)
        vk->flags |= CRVK_FLAG_1_TITLEBAR;
    else
        vk->flags &= CRVK_FLAG_0_TITLEBAR;
    vk->flags != CRVK_FLAG_1_CMDRECORD;
}