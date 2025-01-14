/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-07 11:20:24
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-13 18:03:04
 * @FilePath: \CrystalGraphic\src\vk\vkdevice.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crvk.h>

//这个验证层是需要安装SDK才有的
const char *validationLayerNames[] = {
	"VK_LAYER_KHRONOS_validation"
};

const char *extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef CR_WINDOWS
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined CR_LINUX
	VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME
};

const char *swapchainExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VKAPI_ATTR VkBool32 VKAPI_CALL dbgFunc(
	VkFlags msgFlags,
	VkDebugReportObjectTypeEXT objType,
    uint64_t srcObject,
	size_t location,
	int32_t msgCode,
    const char *pLayerPrefix,
	const char *pMsg,
	void *pUserData
)
{
    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        CRPrint(CR_TC_LIGHTRED, "ERROR: [%s] Code %d : %s\n", pLayerPrefix, msgCode, pMsg);
	else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        CRPrint(CR_TC_LIGHTRED, "WARNING: [%s] Code %d : %s\n", pLayerPrefix, msgCode, pMsg);
	else
        return VK_FALSE;

    /*
     * false indicates that layer should not bail-out of an
     * API call that had validation failures. This may mean that the
     * app dies inside the driver due to invalid parameter(s).
     * That's what would happen without validation layers, so we'll
     * keep that behavior here.
     */
    return VK_FALSE;
}

static void _inner_check_gpu_properties_(pcrvk vk)
{
    VkPhysicalDeviceProperties prop;
    vkGetPhysicalDeviceProperties(vk->gpu, &prop);
#ifdef CR_BUILD_DEBUG
    CRPrint(CR_TC_GREEN, "选中的显卡型号: %s\n", prop.deviceName);
#endif
    vk->msaaSamples = getMaxSampleCount(prop.limits.framebufferColorSampleCounts);
#ifdef CR_BUILD_DEBUG
    CRPrint(CR_TC_GREEN, "最大支持抗锯齿采样数: %d\n", vk->msaaSamples);
#endif
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(vk->gpu, &features);
    if (features.samplerAnisotropy)
    {
        vk->flags |= CRVK_FLAG_1_ANISTROPY;
    #ifdef CR_BUILD_DEBUG
        CRPrint(CR_TC_GREEN, "支持各向异性纹理采样\n");
    #endif
    }
    else
    {
        vk->flags &= CRVK_FLAG_0_ANISTROPY;
    #ifdef CR_BUILD_DEBUG
        CRPrint(CR_TC_LIGHTMAGENTA, "不支持各向异性纹理采样\n");
    #endif
    }
}

static void _inner_create_validation_debug_callback_(pcrvk vk)
{
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_VALIDATION)) return;
    vk->CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vk->instance, "vkCreateDebugReportCallbackEXT");
    vk->DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vk->instance, "vkDestroyDebugReportCallbackEXT");
    if (!vk->CreateDebugReportCallback || !vk->DestroyDebugReportCallback)
    {
        CR_LOG_WAR("auto", "failed to get PFN_vkCreateDebugReportCallbackEXT or PFN_vkDestroyDebugReportCallbackEXT");
        return;
    }
    VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
        .pNext = NULL,
        .pfnCallback = dbgFunc,
        .pUserData = NULL,
        .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
    };
    vk->CreateDebugReportCallback(vk->instance, &dbgCreateInfo, NULL, &vk->msgCallback);
}

static void _inner_destroy_validation_debug_callback_(pcrvk vk)
{
    if (vk->msgCallback)
        vk->DestroyDebugReportCallback(vk->instance, vk->msgCallback, NULL);
}

static CRBOOL _inner_get_queue_(pcrvk vk)
{
    vk->queueFamilyProperties = getQueueProps(vk->gpu, &vk->queueFamilyCount);
    if (!vk->queueFamilyProperties)
    {
        CR_LOG_ERR("auto", "failed to get queue family properties");
        return CRFALSE;
    }
    CRBOOL found = CRFALSE;
    for (CRUINT32 i = 0; i < vk->queueFamilyCount; i++)
    {
        if (vk->queueFamilyProperties[i].queueCount > 0 && vk->queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            vk->graphicsQueueFamilyIndex = i;
            found = CRTRUE;
        }
    }
    if (!found)
    {
        CR_LOG_ERR("auto", "cannot find a graphics queue");
        return CRFALSE;
    }
    //
    return CRTRUE;
}

static void _inner_clear_queue_(pcrvk vk)
{
    vk->queueFamilyCount = 0;
    if (vk->queueFamilyProperties)
        CRAlloc(vk->queueFamilyProperties, 0);
}

CRBOOL _inner_init_vk_device_(pcrvk vk)
{
    CR_LOG_IFO("auto", "init vulkan device");
    if (CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_VALIDATION))
    {
        if (!checkInstanceLayers(validationLayerNames[0]))
            vk->flags &= CRVK_FLAG_0_VALIDATION;
    }
    CRUINT32 extensionCount = CR_ARRAY_SIZE(extensions);
    const char **layers = validationLayerNames;
    CRUINT32 layerCount = CR_ARRAY_SIZE(validationLayerNames);
    if (!CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_VALIDATION))
    {
        extensionCount--;
        layers = NULL;
        layerCount = 0;
    }
    if (!createInstance(&vk->instance, VK_API_VERSION_1_0, extensions, extensionCount, layers, layerCount))
        goto FailedtoCreateInstance;
    if (!getGPU(vk->instance, &vk->gpu))
        goto FailedtoGetGPU;
    _inner_check_gpu_properties_(vk);
    if (CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_VALIDATION))
    {
        if (!checkGPULayers(vk->gpu, validationLayerNames[0]))
            vk->flags &= CRVK_FLAG_0_VALIDATION;
    }
    //获取相关接口，设置调试回调函数
    _inner_create_validation_debug_callback_(vk);
    //获取队列族
    if (!_inner_get_queue_(vk))
        goto FailedtoGetQueue;
    return CRTRUE;
    //错误处理
FailedtoGetQueue:
    //无需额外操作
FailedtoGetGPU:
    destroyInstance(&vk->instance);
FailedtoCreateInstance:
    return CRFALSE;
}

void _inner_uninit_vk_device_(pcrvk vk)
{
    CR_LOG_IFO("auto", "uninit vulkan device");
    _inner_clear_queue_(vk);
    _inner_destroy_validation_debug_callback_(vk);
    destroyInstance(&vk->instance);
}

CRBOOL _inner_create_logical_device_(pcrvk vk)
{
    CR_LOG_IFO("auto", "create vulkan logical device");
    VkDeviceQueueCreateInfo *pDeviceQueueCreateInfo = CRAlloc(NULL, vk->queueFamilyCount * sizeof(VkDeviceQueueCreateInfo));
    if (!pDeviceQueueCreateInfo)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return CRFALSE;
    }
    float **ppQueuePriorities = CRAlloc(NULL, vk->queueFamilyCount * sizeof(float*));
    if (!ppQueuePriorities)
    {
        CR_LOG_ERR("auto", "bad allloc");
        CRAlloc(pDeviceQueueCreateInfo, 0);
        return CRFALSE;
    }
    CRUINT32 i;
    //这里把所有队列全部加入，无论是否用到。
    //但是通常只加入图形队列一个。
    for (i = 0; i < vk->queueFamilyCount; i++)
    {
        ppQueuePriorities[i] = CRAlloc(NULL, vk->queueFamilyProperties[i].queueCount * sizeof(float));
        if (!ppQueuePriorities[i])
        {
            CR_LOG_ERR("auto", "bad alloc");
            goto FailedandClear;
        }
        for (CRUINT32 j = 0; j < vk->queueFamilyProperties[i].queueCount; j++)
            ppQueuePriorities[i][j] = 1.0f;
        pDeviceQueueCreateInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        pDeviceQueueCreateInfo[i].pNext = NULL;
        pDeviceQueueCreateInfo[i].flags = 0;
        pDeviceQueueCreateInfo[i].queueFamilyIndex = i;
        pDeviceQueueCreateInfo[i].queueCount = vk->queueFamilyProperties[i].queueCount;
        pDeviceQueueCreateInfo[i].pQueuePriorities = ppQueuePriorities[i];
    }
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(vk->gpu, &features);
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .queueCreateInfoCount = vk->queueFamilyCount,
        .pQueueCreateInfos = pDeviceQueueCreateInfo,
        .enabledLayerCount = CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_VALIDATION) ? CR_ARRAY_SIZE(validationLayerNames) : 0,
        .ppEnabledLayerNames = CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_VALIDATION) ? validationLayerNames : NULL,
        1,
        swapchainExtensions,
        &features
    };
    VkResult err =
    vkCreateDevice(vk->gpu, &deviceCreateInfo, NULL, &vk->device);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create logical device");
        goto AllClear;
    }
    //
    for (i = 0; i < vk->queueFamilyCount; i++)
        CRAlloc(ppQueuePriorities[i], 0);
    CRAlloc(pDeviceQueueCreateInfo, 0);
    CRAlloc(ppQueuePriorities, 0);
    return CRTRUE;
FailedandClear:
    if (i == 0) goto End;
    i--;
AllClear:
    for (;i >= 0; i--)
    {
        CRAlloc(ppQueuePriorities[i], 0);
    }
End:
    CRAlloc(pDeviceQueueCreateInfo, 0);
    CRAlloc(ppQueuePriorities, 0);
    return CRFALSE;
}

void _inner_destroy_logical_device_(pcrvk vk)
{
    CR_LOG_IFO("auto", "destroy vulkan logical device");
    vkDestroyDevice(vk->device, NULL);
}
