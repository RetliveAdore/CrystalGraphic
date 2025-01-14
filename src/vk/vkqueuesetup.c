/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-07 14:45:07
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-11 13:20:41
 * @FilePath: \CrystalGraphic\src\vk\vkqueuesetup.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crvk.h>

static void _inner_get_queue_(pcrvk vk)
{
    vkGetDeviceQueue(vk->device, vk->graphicsQueueFamilyIndex, 0, &vk->graphicsQueue);
    vkGetDeviceQueue(vk->device, vk->presentQueueFamilyIndex, 0, &vk->presentQueue);    
}

static CRBOOL _inner_check_present_support_(pcrvk vk)
{
    CRBOOL found = CRFALSE;
    for (CRUINT32 i = 0; i < vk->queueFamilyCount; i++)
    {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(vk->gpu, i, vk->surface, &presentSupport);
        if (presentSupport && (vk->queueFamilyProperties[i].queueCount > 0))
        {
            vk->presentQueueFamilyIndex = i;
            found = CRTRUE;
            break;
        }
    }
    if (!found)
    {
        CR_LOG_ERR("auto", "cannot find a present queue");
        return CRFALSE;
    }
    return CRTRUE;
}

static CRBOOL _inner_create_surface_(pcrvk vk)
{
VkResult err = 0;
#ifdef CR_WINDOWS
    VkWin32SurfaceCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .hinstance = GetModuleHandle(NULL),
        .hwnd = vk->window
    };
    err = vkCreateWin32SurfaceKHR(vk->instance, &createInfo, NULL, &vk->surface);
#elif defined CR_LINUX
    VkXlibSurfaceCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .window = vk->window,
        .dpy = vk->dpy
    };
    err = vkCreateXlibSurfaceKHR(vk->instance, &createInfo, NULL, &vk->surface);
#endif
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create surface");
        return CRFALSE;
    }
    return CRTRUE;
}

static void _inner_destroy_surface_(pcrvk vk)
{
    vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);    
}

CRBOOL _inner_setup_vulkan_(pcrvk vk)
{
    CR_LOG_IFO("auto", "vulkan setting up");
    if (!_inner_create_surface_(vk))
        goto FailedtoCreate;
    if (!_inner_check_present_support_(vk))
        goto NotSupport;
    _inner_get_queue_(vk);
    if (!_inner_create_command_pool_(vk))
        goto NotSupport;
    return CRTRUE;
NotSupport:
    _inner_destroy_surface_(vk);
FailedtoCreate:
    return CRFALSE;
}

void _inner_setdown_vulkan_(pcrvk vk)
{
    CR_LOG_IFO("auto", "vulkan setting down");
    _inner_destroy_command_pool_(vk);
    _inner_destroy_surface_(vk);
}