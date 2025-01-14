/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-07 16:05:24
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-14 11:41:05
 * @FilePath: \CrystalGraphic\src\vk\vkswapchain.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crvk.h>

CRBOOL _inner_query_swapchain_support_(pcrvk vk)
{
    VkResult err =
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->gpu, vk->surface, &vk->surfaceCapabilities);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to get gpu surface capabilities");
        return CRFALSE;
    }
    CRUINT32 formatCount;
    CRUINT32 presentModeCount;
    VkSurfaceFormatKHR *pFormats;
    VkPresentModeKHR *pPresentModes;
    err = vkGetPhysicalDeviceSurfaceFormatsKHR(vk->gpu, vk->surface, &formatCount, NULL);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to get gpu surface format count");
        return CRFALSE;
    }
    if (formatCount == 0)
    {
        CR_LOG_ERR("auto", "format count gotten is 0");
        return CRFALSE;
    }
    pFormats = CRAlloc(NULL, formatCount * sizeof(VkSurfaceFormatKHR));
    if (!pFormats)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return CRFALSE;
    }
    err = vkGetPhysicalDeviceSurfaceFormatsKHR(vk->gpu, vk->surface, &formatCount, pFormats);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to get gpu surface formats");
        goto FailedtoGetFormats;
    }
    //
    err = vkGetPhysicalDeviceSurfacePresentModesKHR(vk->gpu, vk->surface, &presentModeCount, NULL);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to get gpu surface present mode count");
        goto FailedtoGetFormats;
    }
    pPresentModes = CRAlloc(NULL, presentModeCount * sizeof(VkPresentModeKHR));
    if (!pPresentModes)
    {
        CR_LOG_ERR("auto", "bad alloc");
        goto FailedtoGetFormats;
    }
    err = vkGetPhysicalDeviceSurfacePresentModesKHR(vk->gpu, vk->surface, &presentModeCount, pPresentModes);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to get gpu surface present mode count");
        goto FailedtoGetFormats;
    }
    //选择颜色模式，优先SRGB
    if (formatCount == 1)
    {
        if (pFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            vk->surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            vk->surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
            vk->flags |= CRVK_FLAG_1_SRGB;
        }
        else vk->surfaceFormat = pFormats[0];
    }
    else
    {
        vk->surfaceFormat = pFormats[0];
        for (CRUINT32 i = 0; i < formatCount; i++)
        {
            if (pFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && pFormats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
            {
                vk->surfaceFormat = pFormats[i];
                vk->flags |= CRVK_FLAG_1_SRGB;
            }
        }
    }
    //选择呈现模式
    vk->presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (CRUINT32 i = 0; i < presentModeCount; i++)
    {
        if (pPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            vk->presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    //
    CRAlloc(pPresentModes, 0);
    CRAlloc(pFormats, 0);
    return CRTRUE;
FailedtoGetPresentModes:
    CRAlloc(pPresentModes, 0);
FailedtoGetFormats:
    CRAlloc(pFormats, 0);
    return CRFALSE;
}

void _inner_choose_swapchain_extent_(pcrvk vk)
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult err =
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->gpu, vk->surface, &capabilities);
    if (err)
        CR_LOG_ERR("auto", "failed to get gpu surface capabilities");
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        vk->swapchainExtent = capabilities.currentExtent;
        vk->w = vk->swapchainExtent.width;
        vk->h = vk->swapchainExtent.height;
    }
    else
    {
        vk->swapchainExtent.width = vk->w;
        vk->swapchainExtent.height = vk->h;
    }
}

CRBOOL _inner_load_swapchain_images_(pcrvk vk)
{
    CRUINT32 imageCount = 0;
    VkResult err =
    vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &imageCount, NULL);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to get swapchain image count");
        return CRFALSE;
    }
    if (imageCount == 0)
    {
        CR_LOG_ERR("auto", "error swapchain image count");
        return CRFALSE;
    }
    vk->swapchainImageCount = imageCount;
    vk->swapchainImages = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkImage));
    if (!vk->swapchainImages)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return CRFALSE;
    }
    err = vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &imageCount, vk->swapchainImages);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to get swapchain image");
        return CRFALSE;
    }
    return CRTRUE;
}

void _inner_unload_swapchain_images_(pcrvk vk)
{
    CRAlloc(vk->swapchainImages, 0);
}

CRBOOL _inner_create_swapchain_image_views_(pcrvk vk)
{
    vk->swapchainImageViews = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkImageView));
    if (!vk->swapchainImageViews)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return CRFALSE;
    }
    CRUINT32 i;
    for (i = 0; i < vk->swapchainImageCount; i++)
    {
        if (!createImageView(
            vk->device,
            vk->swapchainImages[i],
            vk->surfaceFormat.format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            &vk->swapchainImageViews[i])
        )
        {
            CR_LOG_ERR("auto", "failed to create swapchain image view %d", i);
            goto Failed;
        }
    }
    return CRTRUE;
Failed:
    if (i == 0)
        return CRFALSE;
    i--;
    for (; i >= 0; i--)
        vkDestroyImageView(vk->device, vk->swapchainImageViews[i], NULL);
    return CRFALSE;
}

void _inner_destroy_swapchain_image_views_(pcrvk vk)
{
    for (CRUINT32 i = 0; i < vk->swapchainImageCount; i++)
        vkDestroyImageView(vk->device, vk->swapchainImageViews[i], NULL);
    CRAlloc(vk->swapchainImageViews, 0);
}

static CRBOOL _inner_create_immutable_sampler_(pcrvk vk)
{
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
    vkCreateSampler(vk->device, &samplerInfo, NULL, &vk->vkSampler);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create sampler");
        return CRFALSE;
    }
    return CRTRUE;
}

static void _inner_destroy_immutable_sampler_(pcrvk vk)
{
    vkDestroySampler(vk->device, vk->vkSampler, NULL);
}

CRBOOL _inner_create_descriptor_set_layout_(pcrvk vk)
{
    if (!_inner_create_immutable_sampler_(vk))
        return CRFALSE;
    VkDescriptorSetLayoutBinding layoutBindings[3] = {
        [0] = {
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = NULL
        },
        [1] = {
            .binding = 1,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = NULL
        },
        [2] = {
            .binding = 2,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = &vk->vkSampler
        }
    };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .bindingCount = CR_ARRAY_SIZE(layoutBindings),
        .pBindings = layoutBindings
    };
    VkResult err =
    vkCreateDescriptorSetLayout(vk->device, &layoutInfo, NULL, &vk->descriptorSetLayout);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create descriptor set layout");
        _inner_destroy_immutable_sampler_(vk);
        return CRFALSE;
    }
    return CRTRUE;
}

void _inner_destroy_descriptor_set_layout_(pcrvk vk)
{
    vkDestroyDescriptorSetLayout(vk->device, vk->descriptorSetLayout, NULL);
    _inner_destroy_immutable_sampler_(vk);
}

static CRBOOL _inner_create_renderpass_(pcrvk vk)
{
    VkAttachmentDescription attachmentDescription = {
        .flags = 0,
        .format = vk->surfaceFormat.format,
        .samples = vk->msaaSamples,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkAttachmentDescription depthDescription = {
        .flags = 0,
        .format = vk->depthFormat,
        .samples = vk->msaaSamples,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    VkAttachmentDescription colorAttachmentResolve = {
        .flags = 0,
        .format = vk->surfaceFormat.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentDescription attachments[3] = {
        attachmentDescription, depthDescription, colorAttachmentResolve
    };

    VkAttachmentReference attachmentReference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference depthAttachmentReference = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference resolveAttachmentReference = {
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpassDescription = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = VK_NULL_HANDLE,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentReference,
        .pResolveAttachments = &resolveAttachmentReference,
        .pDepthStencilAttachment = &depthAttachmentReference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = VK_NULL_HANDLE
    };

    VkSubpassDependency subpassDependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };

    VkRenderPassCreateInfo renderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .attachmentCount = CR_ARRAY_SIZE(attachments),
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency
    };

    VkResult err =
    vkCreateRenderPass(vk->device, &renderPassCreateInfo, VK_NULL_HANDLE, &vk->renderPass);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create render pass");
        return CRFALSE;
    }
    return CRTRUE;
}

static void _inner_destroy_renderpass_(pcrvk vk)
{
    vkDestroyRenderPass(vk->device, vk->renderPass, NULL);
}

static const char* default_vert = &_binary_out_objs_shader_default_vert_spv_start;
static const CRUINT64 default_vert_size = (CRUINT64)&_binary_out_objs_shader_default_vert_spv_size;

static const char* default_frag = &_binary_out_objs_shader_default_frag_spv_start;
static const CRUINT64 default_frag_size = (CRUINT64)&_binary_out_objs_shader_default_frag_spv_size;

CRBOOL _inner_create_graphics_pipeline_(pcrvk vk)
{
    VkShaderModule vertShader = createShaderModule(vk->device, default_vert, default_vert_size);
    VkShaderModule fragShader = createShaderModule(vk->device, default_frag, default_frag_size);
    if (!vertShader)
    {
        CR_LOG_ERR("auto", "failed to create vertex sahder");
        return CRFALSE;
    }
    if (!fragShader)
    {
        CR_LOG_ERR("auto", "failed to create fragment sahder");
        vkDestroyShaderModule(vk->device, vertShader, NULL);
        return NULL;
    }
    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        [0] = {  //vertex shader
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = NULL,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShader,
            .pName = "main",
            .pSpecializationInfo = NULL
        },
        [1] = {  //fragment shader
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = NULL,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShader,
            .pName = "main",
            .pSpecializationInfo = NULL
        }
    };
    //
    VkVertexInputBindingDescription bindingDescriptions[1] = {
        [0] = {
            .binding = 0,
            .stride = sizeof(CRVERTEX3D_ALPHA_UV),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        }
    };
    VkVertexInputAttributeDescription attributeDescriptions[3] = {
        [0] = {
            .binding = 0,
            .location = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(CRVERTEX3D_ALPHA_UV, pos)
        },
        [1] = {
            .binding = 0,
            .location = 1,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = offsetof(CRVERTEX3D_ALPHA_UV, color)
        },
        [2] = {
            .binding = 0,
            .location = 2,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(CRVERTEX3D_ALPHA_UV, uv)
        }
    };
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .vertexBindingDescriptionCount = CR_ARRAY_SIZE(bindingDescriptions),
        .pVertexBindingDescriptions = bindingDescriptions,
        .vertexAttributeDescriptionCount = CR_ARRAY_SIZE(attributeDescriptions),
        .pVertexAttributeDescriptions = attributeDescriptions
    };
    //
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };
    //
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = vk->swapchainExtent.width,
        .height = vk->swapchainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = vk->swapchainExtent
    };
    VkPipelineViewportStateCreateInfo viewPortState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };
    //
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f
    };
    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = vk->msaaSamples,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };
    VkPipelineDepthStencilStateCreateInfo depth = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthTestEnable = CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_ENABLEDEPTH) ? VK_TRUE : VK_FALSE,
        .depthWriteEnable = CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_ENABLEDEPTH) ? VK_TRUE : VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .front = {},
        .back = {},
    };
    //默认开启透明度混合
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState,
        .blendConstants = {01.0f, 0.0f, 0.0f, 0.0f}
    };
    //
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .dynamicStateCount = CR_ARRAY_SIZE(dynamicStates),
        .pDynamicStates = dynamicStates
    };
    //
    //不知道为什么，管线布局必须在当前函数创建
    //只能即创即用，可能是涉及到了栈布局之类的，只要传值再用必然出错。
    VkPipelineLayoutCreateInfo pipelineLayout = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &vk->descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };
    VkResult err =
    vkCreatePipelineLayout(vk->device, &pipelineLayout, NULL, &vk->pipelineLayout);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create pipeline layuot");
        return CRFALSE;
    }
    //
    if (!_inner_create_renderpass_(vk))
        goto FailedtoCreateRenderPass;
    //
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.stageCount = CR_ARRAY_SIZE(shaderStages),
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputStateCreateInfo,
		.pInputAssemblyState = &inputAssembly,
		.pTessellationState = VK_NULL_HANDLE,
		.pViewportState = &viewPortState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depth,
		.pColorBlendState = &colorBlendStateCreateInfo,
		.pDynamicState = VK_NULL_HANDLE,
		.layout = vk->pipelineLayout,
		.renderPass =  vk->renderPass,
		.subpass =  0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
    };
    err =
    vkCreateGraphicsPipelines(vk->device, NULL, 1, &pipelineInfo, NULL, &vk->graphicsPipeline);
    if (err)
    {
        CR_LOG_ERR("auto", "failed to create graphics pipeline");
        goto FailedtoCreatePipeline;
    }
    //
    vkDestroyShaderModule(vk->device, vertShader, NULL);
    vkDestroyShaderModule(vk->device, fragShader, NULL);
    return CRTRUE;
FailedtoCreatePipeline:
    _inner_destroy_renderpass_(vk);
FailedtoCreateRenderPass:
    vkDestroyPipelineLayout(vk->device, vk->pipelineLayout, NULL);
FailedtoCreatePipelineLayout:
    vkDestroyShaderModule(vk->device, vertShader, NULL);
    vkDestroyShaderModule(vk->device, fragShader, NULL);
    return CRFALSE;
}

void _inner_destroy_graphics_pipeline_(pcrvk vk)
{
    vkDestroyPipeline(vk->device, vk->graphicsPipeline, NULL);
    _inner_destroy_renderpass_(vk);
    vkDestroyPipelineLayout(vk->device, vk->pipelineLayout, NULL);
}

static CRBOOL _inner_create_frame_buffers_(pcrvk vk)
{
    vk->framebuffers = CRAlloc(NULL, vk->swapchainImageCount * sizeof(VkFramebuffer));
    if (!vk->framebuffers)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return CRFALSE;
    }
    CRUINT32 i;
    for (i = 0; i < vk->swapchainImageCount; i++)
    {
        VkImageView attachments[3] = {
            vk->colorImageView,
            vk->depthImageView,
            vk->swapchainImageViews[i]
        };
        VkFramebufferCreateInfo frameBufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .renderPass = vk->renderPass,
            .attachmentCount = CR_ARRAY_SIZE(attachments),
            .pAttachments = attachments,
            .width = vk->swapchainExtent.width,
            .height = vk->swapchainExtent.height,
            .layers = 1
        };
        VkResult err =
        vkCreateFramebuffer(vk->device, &frameBufferInfo, NULL, &vk->framebuffers[i]);
        if (err)
        {
            CR_LOG_ERR("auto", "failed to create frame buffer %d", i);
            goto FailedtoCreateFrameBuffer;
        }
    }
    return CRTRUE;
FailedtoCreateFrameBuffer:
    if (1 == 0) goto End;
    i--;
    for (; i >= 0; i--)
        vkDestroyFramebuffer(vk->device, vk->framebuffers[i], NULL);
    //
End:
    CRAlloc(vk->framebuffers, 0);
    return CRFALSE;
}

static void _inner_destroy_frame_buffers_(pcrvk vk)
{
    for (CRUINT32 i = 0; i < vk->swapchainImageCount; i++)
        vkDestroyFramebuffer(vk->device, vk->framebuffers[i], NULL);
    CRAlloc(vk->framebuffers, 0);
}

CRBOOL _inner_query_swapchain_image_count_(pcrvk vk)
{
    if (!_inner_query_swapchain_support_(vk))
        return CRFALSE;
    #ifdef CR_BUILD_DEBUG
    CRPrint(CR_TC_LIGHTCYAN, "%s原生SRGB呈现色彩空间\n", CR_GET_FLAG_1_STAT(vk->flags, CRVK_FLAG_1_SRGB) ? "支持" : "不支持");
    #endif
    vk->swapchainImageCount = vk->surfaceCapabilities.minImageCount + 1;
    return CRTRUE;
}

CRBOOL _inner_create_swapchain_(pcrvk vk)
{
    CR_LOG_IFO("auto", "create swapchain");
    _inner_choose_swapchain_extent_(vk);
    if (!_inner_create_extension_resources_(vk))
        goto FailedtoCreateDepth;
    if (vk->surfaceCapabilities.maxImageCount > 0 && vk->swapchainImageCount > vk->surfaceCapabilities.maxImageCount)
        vk->swapchainImageCount = vk->surfaceCapabilities.maxImageCount;
    vk->imageArrayLayers = 1;
    vk->swapchain = createSwapchain(
        vk->device,
        vk->surface,
        &vk->surfaceCapabilities,
        vk->surfaceFormat,
        vk->swapchainExtent,
        vk->presentMode,
        vk->graphicsQueueFamilyIndex,
        vk->presentQueueFamilyIndex,
        NULL
    );
    if (!vk->swapchain)
        goto FailedoCreateSwapchain;
    if (!_inner_load_swapchain_images_(vk))
        goto FailedtoLoadImages;
    if (!_inner_create_swapchain_image_views_(vk))
        goto FailedtoCreateView;
    if (!_inner_create_graphics_pipeline_(vk))
        goto FailedtoCreatePipeline;
    if (!_inner_create_frame_buffers_(vk))
        goto FailedtoCreateFrameBuffer;
    if (!_inner_create_command_buffers_(vk))
        goto FailedtoCreateCommandBuffer;
    return CRTRUE;
FailedtoCreateCommandBuffer:
    _inner_destroy_frame_buffers_(vk);
FailedtoCreateFrameBuffer:
    _inner_destroy_graphics_pipeline_(vk);
FailedtoCreatePipeline:
    _inner_destroy_swapchain_image_views_(vk);
FailedtoCreateView:
    _inner_unload_swapchain_images_(vk);
FailedtoLoadImages:
    vkDestroySwapchainKHR(vk->device, vk->swapchain, NULL);
FailedoCreateSwapchain:
    _inner_destroy_extension_resources_(vk);
FailedtoCreateDepth:
    return CRFALSE;
}

void _inner_destroy_swapchain_(pcrvk vk)
{
    CR_LOG_IFO("auto", "destroy swapchain");
    _inner_destroy_command_buffers_(vk);
    _inner_destroy_frame_buffers_(vk);
    _inner_destroy_graphics_pipeline_(vk);
    _inner_destroy_swapchain_image_views_(vk);
    _inner_unload_swapchain_images_(vk);
    vkDestroySwapchainKHR(vk->device, vk->swapchain, NULL);
    _inner_destroy_extension_resources_(vk);
}