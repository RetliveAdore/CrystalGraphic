/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-12 12:42:52
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-13 18:01:59
 * @FilePath: \CrystalGraphic\src\vk\vkextension.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crvk.h>

const VkFormat depthFormats[] = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT
};

CRBOOL _inner_create_extension_resources_(pcrvk vk)
{
    //深度缓冲相关
    CRPrint(CR_TC_GREEN, "深度缓冲%s\n", CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_ENABLEDEPTH) ? "开启" : "关闭");
    #ifdef CR_BUILD_DEBUG
    #endif
    if (!findSupportedFormat(
            vk->gpu,
            CR_ARRAY_SIZE(depthFormats),
            depthFormats,
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
            &vk->depthFormat
        )
    )
    {
        CR_LOG_WAR("auto", "no suitable depth format found");
        vk->flags &= CRVK_FLAG_0_ENABLEDEPTH;
        #ifdef CR_BUILD_DEBUG
        CRPrint(CR_TC_LIGHTMAGENTA, "深度缓冲关闭\n");
        #endif
        return CRFALSE;
    }
    //
    CRBOOL back =
    createImage(
        vk->device, vk->gpu,
        vk->msaaSamples,
        vk->swapchainExtent.width,
        vk->swapchainExtent.height,
        vk->depthFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &vk->depthImage,
        &vk->depthImageMemory
    );
    if (!back)
    {
        CR_LOG_ERR("auto", "failed to create depth image");
        goto FailedtoCreateImage;
    }
    back = createImageView(vk->device, vk->depthImage, vk->depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &vk->depthImageView);
    if (!back)
    {
        CR_LOG_ERR("auto", "failed to create depth imageview");
        goto FailedtoCreateView;
    }
    transitionImageLayout(
        vk->drawCommandPool,
        vk->graphicsQueue,
        vk->device,
        vk->depthImage,
        vk->depthFormat,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    );
    //多重采样相关
    back = createImage(
        vk->device, vk->gpu,
        vk->msaaSamples,
        vk->swapchainExtent.width,
        vk->swapchainExtent.height,
        vk->surfaceFormat.format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &vk->colorImage,
        &vk->colorImageMemory
    );
    if (!back)
    {
        CR_LOG_ERR("auto", "failed to create msaa image");
        goto FailedtoCreateMSAAImage;
    }
    back = createImageView(vk->device, vk->colorImage, vk->surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, &vk->colorImageView);
    if (!back)
    {
        CR_LOG_ERR("auto", "failed to create msaa image view");
        goto FailedtoCreateMSAAView;
    }
    transitionImageLayout(
        vk->drawCommandPool,
        vk->graphicsQueue,
        vk->device,
        vk->colorImage,
        vk->surfaceFormat.format,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );
    return CRTRUE;
FailedtoCreateMSAAView:
    destroyImage(vk->device, vk->colorImage, vk->colorImageMemory);
FailedtoCreateMSAAImage:
    vkDestroyImageView(vk->device, vk->depthImageView, NULL);
FailedtoCreateView:
    destroyImage(vk->device, vk->depthImage, vk->depthImageMemory);
FailedtoCreateImage:
    vk->flags &= CRVK_FLAG_0_ENABLEDEPTH;
    return CRFALSE;
}

void _inner_destroy_extension_resources_(pcrvk vk)
{
    vkDestroyImageView(vk->device, vk->colorImageView, NULL);
    destroyImage(vk->device, vk->colorImage, vk->colorImageMemory);
    vkDestroyImageView(vk->device, vk->depthImageView, NULL);
    destroyImage(vk->device, vk->depthImage, vk->depthImageMemory);
}