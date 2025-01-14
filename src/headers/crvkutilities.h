/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-05 23:36:37
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-12 16:27:23
 * @FilePath: \CrystalGraphic\src\headers\crvkutilities.h
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CRVK_UTILITIES_H_
#define _INCLUDE_CRVK_UTILITIES_H_

#include <CrystalCore.h>
#ifndef CR_BUILD_DEBUG
#  ifdef CR_LOG_IFO
#  undef CR_LOG_IFO
#  define CR_LOG_IFO(...)
#  endif

#  ifdef CR_LOG_DBG
#  undef CR_LOG_DBG
#  define CR_LOG_DBG(...)
#  endif

#  ifdef CR_LOG_WAR
#  undef CR_LOG_WAR
#  define CR_LOG_WAR(...)
#  endif

#  ifdef CR_LOG_ERR
#  undef CR_LOG_ERR
#  define CR_LOG_ERR(...)
#  endif
#endif
#include <Defs.h>

#ifdef CR_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#elif defined CR_LINUX
#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#endif
#include <vulkan/vulkan.h>
#include <string.h>

/**
 * 内部隐藏接口
 */
typedef void*(*CRALLOC)(void *ptr, CRUINT64 size);
#define CRAlloc ((CRALLOC)CRCoreFunList[10])

/**
 * 检查设备以及创建vulkan句柄
 */

CRBOOL checkInstanceLayers(const char *layerName);
//
CRBOOL createInstance(
    VkInstance *pInstance,
    CRUINT32 apiVersion,
    const char **extensions,
    CRUINT32 extensionCount,
    const char **layers,
    CRUINT32 layerCount
);
void destroyInstance(VkInstance *pInstance);
//
CRBOOL getGPU(VkInstance instance, VkPhysicalDevice *pGPU);
//
VkSampleCountFlagBits getMaxSampleCount(VkSampleCountFlagBits samples);
//
CRBOOL checkGPULayers(VkPhysicalDevice gpu, const char *layerName);
//
VkQueueFamilyProperties* getQueueProps(VkPhysicalDevice gpu, CRUINT32 *pCount);

/**
 * 创建交换链
 */

VkSwapchainKHR createSwapchain(
    VkDevice device,
    VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR *pSurfaceCapabilities,
    VkSurfaceFormatKHR surfaceFormat,
    VkExtent2D swapchainExtent,
    VkPresentModeKHR presentMode,
    CRUINT32 graphicFamilyIndex,
    CRUINT32 presentFamilyIndex,
    VkSwapchainKHR oldSwapchain
);
//
CRBOOL createImageView(
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectMask,
    VkImageView *pImageView
);
//
VkShaderModule createShaderModule(VkDevice device, const char* codes, const CRUINT64 size);

/**
 * 绘制相关
 */
VkSemaphore *createSemaphores(VkDevice device, CRUINT32 maxFrames);
void deleteSemaphores(VkDevice device, VkSemaphore *pSemaphores, CRUINT32 maxFrames);
//
VkFence *createFences(VkDevice device, CRUINT32 maxImages);
void deleteFences(VkDevice device, VkFence *pFences, CRUINT32 maxImages);
//
CRUINT32 findMemoryType(VkPhysicalDevice gpu, CRUINT32 filter, VkMemoryPropertyFlags properties);
//
CRBOOL createBuffer(
    VkPhysicalDevice gpu,
    VkDevice device,
    CRUINT64 size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory
);
void destroyBuffer(VkDevice device, VkBuffer buffer, VkDeviceMemory bufferMemory);
void copyMemory(VkDevice device, VkDeviceMemory memory, CRDYNAMIC data, CRUINT64 size);
void copyBuffer(
    VkCommandPool commandPool,
    VkQueue queue,
    VkDevice device,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    CRUINT64 size
);
//
CRBOOL createImage(
    VkDevice device,
    VkPhysicalDevice gpu,
    VkSampleCountFlagBits numSamples,
    CRUINT32 w,
    CRUINT32 h,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage *pImage,
    VkDeviceMemory *pImageMemory
);
void destroyImage(VkDevice device, VkImage image, VkDeviceMemory imageMemory);
//
void copyBufferToImage(
    VkCommandPool commandPool,
    VkQueue queue,
    VkDevice device,
    VkBuffer buffer,
    VkImage image,
    CRUINT32 w,
    CRUINT32 h
);
void transitionImageLayout(
    VkCommandPool commandPool,
    VkQueue queue,
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout
);

//拓展相关
CRBOOL findSupportedFormat(
    VkPhysicalDevice gpu,
    CRUINT32 count,
    const VkFormat *candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features,
    VkFormat *pFormat
);
CRBOOL hasStencilComponent(VkFormat format);

#endif