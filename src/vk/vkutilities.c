/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-05 23:39:33
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-12 16:56:38
 * @FilePath: \CrystalGraphic\src\vk\vkutilities.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crvkutilities.h>

CRBOOL checkInstanceLayers(const char *layerName)
{
    VkResult err = 0;
	CRUINT32 propertyCount;
	VkLayerProperties *pProperties;
    err = vkEnumerateInstanceLayerProperties(&propertyCount, NULL);
	if (err)
    {
        CR_LOG_WAR("auto", "failed to get vulkan instance property count");
        return CRFALSE;
    }
    if (propertyCount == 0)
    {
        CR_LOG_WAR("auto", "no vulkan instance property found");
        return CRFALSE;
    }
    pProperties = CRAlloc(NULL, propertyCount * sizeof(VkLayerProperties));
    if (!pProperties)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return CRFALSE;
    }
    err = vkEnumerateInstanceLayerProperties(&propertyCount, pProperties);
	if (err)
    {
        CR_LOG_WAR("auto", "failed to get vulkan instance properties");
        CRAlloc(pProperties, 0);
        return CRFALSE;
    }
    CRBOOL layerFound = CRFALSE;
    for (CRUINT32 i = 0; i < propertyCount; i++)
	{
		if (!strcmp(pProperties[i].layerName, layerName))
			layerFound = CRTRUE;
	}
    if (!layerFound)
        CR_LOG_WAR("auto", "required validation layer not found");
    CRAlloc(pProperties, 0);
    return layerFound;
}

CRBOOL createInstance(
    VkInstance *pInstance,
    CRUINT32 apiVersion,
    const char **extensions,
    CRUINT32 extensionCount,
    const char **layers,
    CRUINT32 layerCount
)
{
    VkApplicationInfo applicationInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = "CrystalGraphic",
		.applicationVersion = 0,
		.pEngineName = "CrystalEngine",
		.engineVersion = 0,
		.apiVersion = apiVersion
	};
    VkInstanceCreateInfo instanceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = layerCount,
		.ppEnabledLayerNames = layers,
		.enabledExtensionCount = extensionCount,
		.ppEnabledExtensionNames = extensions
	};
    VkResult err =
    vkCreateInstance(&instanceCreateInfo, NULL, pInstance);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create vulkan instance");
        return CRFALSE;
    }
    return CRTRUE;
}

void destroyInstance(VkInstance *pInstance)
{
    vkDestroyInstance(*pInstance, NULL);
}

static CRBOOL checkGPUSuitable(VkPhysicalDevice gpu)
{
    VkPhysicalDeviceProperties prop;
    VkPhysicalDeviceFeatures feat;
    vkGetPhysicalDeviceProperties(gpu, &prop);
    vkGetPhysicalDeviceFeatures(gpu, &feat);
    return prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && feat.geometryShader;
}

static CRUINT32 getPhysicalDeviceTotalMemory(VkPhysicalDeviceMemoryProperties *pPhysicalDeviceMemoryProperties)
{
    CRUINT32 physicalDeviceTotalMemory = 0;
    for (int i = 0; i < pPhysicalDeviceMemoryProperties->memoryHeapCount; i++)
    {
        if ((pPhysicalDeviceMemoryProperties->memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
            physicalDeviceTotalMemory += pPhysicalDeviceMemoryProperties->memoryHeaps[i].size;
    }
    return physicalDeviceTotalMemory;
}

static CRUINT32 getBestPhysicalDeviceIndex(VkPhysicalDevice *pGPUS, CRUINT32 physicalDeviceNumber)
{
    VkPhysicalDeviceProperties *gpuProperties = CRAlloc(NULL, physicalDeviceNumber * sizeof(VkPhysicalDeviceProperties));
    if (!gpuProperties)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return 0;
    }
    VkPhysicalDeviceMemoryProperties *gpuMemoryProperties = CRAlloc(NULL, physicalDeviceNumber * sizeof(VkPhysicalDeviceMemoryProperties));
    if (!gpuMemoryProperties)
    {
        CR_LOG_ERR("auto", "bad alloc");
        CRAlloc(gpuProperties, 0);
        return 0;
    }
    CRUINT32 *discreteGPUIndices = CRAlloc(NULL, physicalDeviceNumber * sizeof(CRUINT32));
    if (!discreteGPUIndices)
    {
        CR_LOG_ERR("auto", "bad alloc");
        CRAlloc(gpuProperties, 0);
        CRAlloc(gpuMemoryProperties, 0);
        return 0;
    }
    CRUINT32 *integratedGPUIndices = CRAlloc(NULL, physicalDeviceNumber * sizeof(CRUINT32));
    if (!integratedGPUIndices)
    {
        CR_LOG_ERR("auto", "bad alloc");
        CRAlloc(gpuProperties, 0);
        CRAlloc(gpuMemoryProperties, 0);
        CRAlloc(discreteGPUIndices, 0);
        return 0;
    }

    CRUINT32 discreteGPUNumber = 0, integratedGPUNumber = 0;

    for (CRUINT32 i = 0; i < physicalDeviceNumber; i++)
    {
        vkGetPhysicalDeviceProperties(pGPUS[i], &gpuProperties[i]);
        vkGetPhysicalDeviceMemoryProperties(pGPUS[i], &gpuMemoryProperties[i]);

        if (gpuProperties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            discreteGPUIndices[discreteGPUNumber] = i;
            discreteGPUNumber++;
        }
        if (gpuProperties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            integratedGPUIndices[integratedGPUNumber] = i;
            integratedGPUNumber++;
        }
    }

    CRUINT32 bestPhysicalDeviceIndex = 0;
    VkDeviceSize bestPhysicalDeviceMemory = 0;

    if (discreteGPUNumber != 0)
    {
        for (CRUINT32 i = 0; i < discreteGPUNumber; i++)
        {
            if (bestPhysicalDeviceMemory < getPhysicalDeviceTotalMemory(&gpuMemoryProperties[discreteGPUIndices[i]]))
            {
                bestPhysicalDeviceMemory = getPhysicalDeviceTotalMemory(&gpuMemoryProperties[discreteGPUIndices[i]]);
                bestPhysicalDeviceIndex = discreteGPUIndices[i];
            }
        }
    }
    else if (integratedGPUNumber != 0)
    {
        for (CRUINT32 i = 0; i < integratedGPUNumber; i++)
        {
            if (bestPhysicalDeviceMemory < getPhysicalDeviceTotalMemory(&gpuMemoryProperties[integratedGPUIndices[i]]))
            {
                bestPhysicalDeviceMemory = getPhysicalDeviceTotalMemory(&gpuMemoryProperties[integratedGPUIndices[i]]);
                bestPhysicalDeviceIndex = integratedGPUIndices[i];
            }
        }
    }

    CRAlloc(discreteGPUIndices, 0);
    CRAlloc(integratedGPUIndices, 0);
    CRAlloc(gpuMemoryProperties, 0);
    CRAlloc(gpuProperties, 0);

    return bestPhysicalDeviceIndex;
}

CRBOOL getGPU(VkInstance instance, VkPhysicalDevice *pGPU)
{
    VkResult err = 0;
    CRUINT32 gpuNumber = 0;
    err = vkEnumeratePhysicalDevices(instance, &gpuNumber, NULL);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to get number of gpu");
        return CRFALSE;
    }
    if (gpuNumber == 0)
    {
        CR_LOG_ERR("auto", "no gpu found, check your driver");
        return CRFALSE;
    }
    VkPhysicalDevice *GPUs = CRAlloc(NULL, gpuNumber * sizeof(VkPhysicalDevice));
    if (!GPUs)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return CRFALSE;
    }
    err = vkEnumeratePhysicalDevices(instance, &gpuNumber, GPUs);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to get gpus");
        goto End;
    }
    *pGPU = GPUs[getBestPhysicalDeviceIndex(GPUs, gpuNumber)];
    CRAlloc(GPUs, 0);
    return CRTRUE;
End:
    CRAlloc(GPUs, 0);
    return CRTRUE;
}

VkSampleCountFlagBits getMaxSampleCount(VkSampleCountFlagBits samples)
{
    if (samples & VK_SAMPLE_COUNT_64_BIT)
        return VK_SAMPLE_COUNT_64_BIT;
    else if (samples & VK_SAMPLE_COUNT_32_BIT)
        return VK_SAMPLE_COUNT_32_BIT;
    else if (samples & VK_SAMPLE_COUNT_16_BIT)
        return VK_SAMPLE_COUNT_16_BIT;
    else if (samples & VK_SAMPLE_COUNT_8_BIT)
        return VK_SAMPLE_COUNT_8_BIT;
    else if (samples & VK_SAMPLE_COUNT_4_BIT)
        return VK_SAMPLE_COUNT_4_BIT;
    else if (samples & VK_SAMPLE_COUNT_2_BIT)
        return VK_SAMPLE_COUNT_2_BIT;
    else
        return VK_SAMPLE_COUNT_1_BIT;
}

CRBOOL checkGPULayers(VkPhysicalDevice gpu, const char *layerName)
{
	VkResult err = 0;
	CRUINT32 propertyCount;
	VkLayerProperties *pProperties;
	err = vkEnumerateDeviceLayerProperties(gpu, &propertyCount, NULL);
	if (err) CR_LOG_ERR("auto", "failed to get GPU property count");
	if (propertyCount == 0)
	{
		CR_LOG_WAR("auto", "no GPU properties found");
		return CRFALSE;
	}
	pProperties = CRAlloc(NULL, propertyCount * sizeof(VkLayerProperties));
	if (!pProperties)
	{
		CR_LOG_ERR("auto", "bad alloc");
		return CRFALSE;
	}
	err = vkEnumerateDeviceLayerProperties(gpu, &propertyCount, pProperties);
	if (err)
	{
		CR_LOG_ERR("auto", "failed to get GPU properties");
		CRAlloc(pProperties, 0);
        return CRFALSE;
	}
	CRBOOL layerFound = CRFALSE;
	for (CRUINT32 i = 0; i < propertyCount; i++)
	{
		if (!strcmp(pProperties[i].layerName, layerName))
			layerFound = CRTRUE;
	}
	CRAlloc(pProperties, 0);
    //
    if (!layerFound)
	    CR_LOG_WAR("auto", "required validation layer not found");
    return layerFound;
}

VkQueueFamilyProperties* getQueueProps(VkPhysicalDevice gpu, CRUINT32 *pCount)
{
    VkQueueFamilyProperties *pQueueFamilyProps;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, pCount, NULL);
    if (*pCount == 0)
    {
        CR_LOG_ERR("auto", "queue family count is 0");
        return NULL;
    }
    pQueueFamilyProps = CRAlloc(NULL, *pCount * sizeof(VkQueueFamilyProperties));
    if (!pQueueFamilyProps)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return NULL;
    }
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, pCount, pQueueFamilyProps);
    return pQueueFamilyProps;
}

CRBOOL checkQueueFamilySupport(VkQueueFamilyProperties prop, VkQueueFlagBits requiredFlag)
{
    return prop.queueCount > 0 && prop.queueFlags & requiredFlag;
}

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
)
{
    VkSharingMode imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CRUINT32 queueFamilyIndexCount = 0;
    CRUINT32 *pQueueFamilyIndices = NULL;
    CRUINT32 queueFamilyIndicesI[] = { graphicFamilyIndex, presentFamilyIndex };
    CRUINT32 queueFamilyIndicesII[] = { graphicFamilyIndex, presentFamilyIndex };
    if (graphicFamilyIndex != presentFamilyIndex)
    {
        imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        queueFamilyIndexCount = 2;
        pQueueFamilyIndices = queueFamilyIndicesI;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .surface = surface,
        .minImageCount = pSurfaceCapabilities->minImageCount + 1,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = imageSharingMode,
        .queueFamilyIndexCount = queueFamilyIndexCount,
        .pQueueFamilyIndices = pQueueFamilyIndices,
        .preTransform = pSurfaceCapabilities->currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapchain
    };

    VkSwapchainKHR swapchain;
    VkResult err = 0;
    err = vkCreateSwapchainKHR(device, &swapchainCreateInfo, VK_NULL_HANDLE, &swapchain);
    if (err) CR_LOG_ERR("auto", "failed to create swapchain");
    if (oldSwapchain)
        vkDestroySwapchainKHR(device, oldSwapchain, NULL);
    return swapchain;
}

CRBOOL createImageView(
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectMask,
    VkImageView *pImageView
)
{
    VkResult err = 0;
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        }
    };
    err = vkCreateImageView(device, &viewInfo, NULL, pImageView);
    if (err) return CRFALSE;
    return CRTRUE;
}

VkShaderModule createShaderModule(VkDevice device, const char* codes, const CRUINT64 size)
{
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pCode = (const CRUINT32*)codes,
        .codeSize = size
    };
    VkShaderModule module;
    VkResult err =
    vkCreateShaderModule(device, &createInfo, NULL, &module);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create shader module");
        return NULL;
    }
    return module;
}


VkSemaphore *createSemaphores(VkDevice device, CRUINT32 maxFrames)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        VK_NULL_HANDLE,
        0
    };

    VkSemaphore *semaphore = CRAlloc(NULL, maxFrames * sizeof(VkSemaphore));
	if (!semaphore)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return NULL;
    }
    VkResult err = 0;
    CRUINT32 i;
    for (i = 0; i < maxFrames; i++)
    {
        err = vkCreateSemaphore(device, &semaphoreCreateInfo, VK_NULL_HANDLE, &semaphore[i]);
        if (err)
        {
            CR_LOG_ERR("auto", "failed to create semaphore %d", i);
            goto Failed;
        }
    }
    return semaphore;
Failed:
    if (i == 0) goto End;
    i--;
    for (; i >= 0; i--)
        vkDestroySemaphore(device, semaphore[i], NULL);
End:
    CRAlloc(semaphore, 0);
    return NULL;
}

void deleteSemaphores(VkDevice device, VkSemaphore *pSemaphores, CRUINT32 maxFrames)
{
    for (uint32_t i = 0; i < maxFrames; i++)
        vkDestroySemaphore(device, pSemaphores[i], VK_NULL_HANDLE);
    CRAlloc(pSemaphores, 0);
}

VkFence *createFences(VkDevice device, CRUINT32 maxImages)
{
    VkFenceCreateInfo fenceCreateInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        VK_NULL_HANDLE,
        VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkFence *fence = CRAlloc(NULL, maxImages * sizeof(VkFence));
	if (!fence)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return NULL;
    }
    VkResult err = 0;
    CRUINT32 i;
    for (i = 0; i < maxImages; i++)
    {
        err = vkCreateFence(device, &fenceCreateInfo, VK_NULL_HANDLE, &fence[i]);
        if (err)
        {
            CR_LOG_ERR("auto", "failed to create fence %d", i);
            goto Failed;
        }
    }
    return fence;
Failed:
    if (i == 0) goto End;
    i--;
    for (;i >= 0; i--)
        vkDestroyFence(device, fence[i], NULL);
End:
    CRAlloc(fence, 0);
    return NULL;
}

void deleteFences(VkDevice device, VkFence *pFences, CRUINT32 maxImages)
{
    for (uint32_t i = 0; i < maxImages; i++)
        vkDestroyFence(device, pFences[i], VK_NULL_HANDLE);
    CRAlloc(pFences, 0);
}

static void copyDynMemory(VkDevice device, VkDeviceMemory *pDeviceMemory, CRUINT64 size, CRDYNAMIC srcData)
{
    VkResult err = 0;
    char* data;
    err = vkMapMemory(device, *pDeviceMemory, 0, size, 0, (void**)&data);
    if (err) CR_LOG_ERR("auto", "failed to map memory");
    for (CRUINT64 i = 0; i < size; i++)
        CRDynSeek(srcData, &data[i], i, DYN_MODE_8);
    vkUnmapMemory(device, *pDeviceMemory);
}

CRUINT32 findMemoryType(VkPhysicalDevice gpu, CRUINT32 filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);
    for (CRUINT32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (
            filter & (1 << i) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties
        )
        return i;
    }
}

CRBOOL createBuffer(
    VkPhysicalDevice gpu,
    VkDevice device,
    CRUINT64 size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory
)
{
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VkResult err =
    vkCreateBuffer(device, &bufferInfo, NULL, pBuffer);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create buffer");
        goto FailedtoCreateBuffer;
    }
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *pBuffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(gpu, memRequirements.memoryTypeBits, properties)
    };
    err = vkAllocateMemory(device, &allocInfo, NULL, pBufferMemory);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to allocate memory");
        goto FailedtoAllocateMemory;
    }
    err = vkBindBufferMemory(device, *pBuffer, *pBufferMemory, 0);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to bind buffer memory");
        goto FailedtoBindBufferMemory;
    }
    return CRTRUE;
FailedtoBindBufferMemory:
    vkFreeMemory(device, *pBufferMemory, NULL);
FailedtoAllocateMemory:
    vkDestroyBuffer(device, *pBuffer, NULL);
FailedtoCreateBuffer:
    return CRFALSE;
}

void destroyBuffer(VkDevice device, VkBuffer buffer, VkDeviceMemory bufferMemory)
{
    vkFreeMemory(device, bufferMemory, NULL);
    vkDestroyBuffer(device, buffer, NULL);
}

void copyMemory(VkDevice device, VkDeviceMemory memory, CRDYNAMIC data, CRUINT64 size)
{
    char *mapData;
    VkResult err =
    vkMapMemory(device, memory, 0, size, 0, (void**)&mapData);
    if (err) CR_LOG_ERR("auto", "failed to map memory");
    for (CRUINT32 i = 0; i < size; i++)
        CRDynSeek(data, &mapData[i], i, DYN_MODE_8);
    vkUnmapMemory(device, memory);
}

static VkCommandBuffer beginSingleTimeCommand(VkCommandPool commandPool, VkDevice device)
{
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = 1
    };
    VkCommandBuffer commandBuffer;
    VkResult err =
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to allocate single time command buffer");
        return NULL;
    }
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = NULL
    };
    err = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to begin single time command buffer");
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        return NULL;
    }
    return commandBuffer;
}

static void endSingleTimeCommand(VkCommandPool commandPool, VkQueue queue, VkDevice device, VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = NULL,
        .waitSemaphoreCount = 0,
        .pWaitDstStageMask = NULL
    };
    VkResult err =
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    if (err) CR_LOG_WAR("auto", "failed to submit single time command");
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void copyBuffer(
    VkCommandPool commandPool,
    VkQueue queue,
    VkDevice device,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    CRUINT64 size
)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommand(commandPool, device);
    if (!commandBuffer) return;
    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    endSingleTimeCommand(commandPool, queue, device, commandBuffer);
}

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
)
{
    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = {
            .width = w,
            .height = h,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = format,
        .tiling = tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = numSamples
    };
    VkResult err =
    vkCreateImage(device, &imageInfo, NULL, pImage);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create texture image");
        goto FailedtoCreateTextureImage;
    }
    //
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *pImage, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(
            gpu,
            memRequirements.memoryTypeBits,
            properties
        )
    };
    err = vkAllocateMemory(device, &allocInfo, NULL, pImageMemory);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to allocatte texture memory");
        goto FailedtoAllocateTextureMemory;
    }
    err = vkBindImageMemory(device, *pImage, *pImageMemory, 0);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to bind image memory");
        goto FailedtoBindImageMemory;
    }
    return CRTRUE;
FailedtoBindImageMemory:
    vkFreeMemory(device, *pImageMemory, NULL);
FailedtoAllocateTextureMemory:
    vkDestroyImage(device, *pImage, NULL);
FailedtoCreateTextureImage:
    return CRFALSE;
}

void destroyImage(VkDevice device, VkImage image, VkDeviceMemory imageMemory)
{
    vkFreeMemory(device, imageMemory, NULL);
    vkDestroyImage(device, image, NULL);
}

void copyBufferToImage(
    VkCommandPool commandPool,
    VkQueue queue,
    VkDevice device,
    VkBuffer buffer,
    VkImage image,
    CRUINT32 w,
    CRUINT32 h
)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommand(commandPool, device);
    //
    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {
            .width = w,
            .height = h,
            .depth = 1
        }
    };
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    //
    endSingleTimeCommand(commandPool, queue, device, commandBuffer);
}

void transitionImageLayout(
    VkCommandPool commandPool,
    VkQueue queue,
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout
)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommand(commandPool, device);
    //
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .srcAccessMask = 0,
        .dstAccessMask = 0
    };
    //
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else
    {
        CR_LOG_WAR("auto", "unsupported layout transition");
        goto Skip;
    }
    //
    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);
    //
Skip:
    endSingleTimeCommand(commandPool, queue, device, commandBuffer);
}

CRBOOL findSupportedFormat(
    VkPhysicalDevice gpu,
    CRUINT32 count,
    const VkFormat *candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features,
    VkFormat *pFormat
)
{
    VkFormatProperties props;
    for (CRUINT32 i = 0; i < count; i++)
    {
        vkGetPhysicalDeviceFormatProperties(gpu, candidates[i], &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures &features) == features)
        {
            *pFormat = candidates[i];
            return CRTRUE;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL&& (props.optimalTilingFeatures &features) == features)
        {
            *pFormat = candidates[i];
            return CRTRUE;
        }
    }
    CR_LOG_ERR("auto", "failed to find supported format");
    return CRFALSE;
}

CRBOOL hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}