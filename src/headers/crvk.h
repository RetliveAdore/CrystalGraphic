/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-05 23:10:40
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-14 11:27:51
 * @FilePath: \CrystalGraphic\src\headers\crvk.h
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CR_VK_H_
#define _INCLUDE_CR_VK_H_

#include "crvkutilities.h"

extern char _binary_out_objs_shader_default_vert_spv_start;
extern char _binary_out_objs_shader_default_vert_spv_end;
extern int _binary_out_objs_shader_default_vert_spv_size;

extern char _binary_out_objs_shader_default_frag_spv_start;
extern char _binary_out_objs_shader_default_frag_spv_end;
extern int _binary_out_objs_shader_default_frag_spv_size;

#define CR_TEXTURE_FORMAT VK_FORMAT_R8G8B8A8_UNORM

//表示内部各种状态的标志位定义
#define CRVK_FLAG_0_0           (CRUINT64)(0)
#define CRVK_FLAG_1_VALIDATION  (CRUINT64)(1)
#define CRVK_FLAG_0_VALIDATION  (~(CRUINT64)(1))
#define CRVK_FLAG_1_SRGB        (CRUINT64)(1 << 1)
#define CRVK_FLAG_0_SRGB        (~(CRUINT64)(1 << 1))
#define CRVK_FLAG_1_INITED      (CRUINT64)(1 << 2)
#define CRVK_FLAG_0_INITED      (~(CRUINT64)(1 << 2))
#define CRVK_FLAG_1_RENDERING   (CRUINT64)(1 << 3)
#define CRVK_FLAG_0_RENDERING   (~(CRUINT64)(1 << 3))
#define CRVK_FLAG_1_ENABLEDEPTH (CRUINT64)(1 << 4)
#define CRVK_FLAG_0_ENABLEDEPTH (~(CRUINT64)(1 << 4))
#define CRVK_FLAG_1_FLUSH       (CRUINT64)(1 << 5)
#define CRVK_FLAG_0_FLUSH       (~(CRUINT64)(1 << 5))
#define CRVK_FLAG_1_CMDRECORD   (CRUINT64)(1 << 6)
#define CRVK_FLAG_0_CMDRECORD   (~(CRUINT64)(1 << 6))
#define CRVK_FLAG_1_RESIZE      (CRUINT64)(1 << 7)
#define CRVK_FLAG_0_RESIZE      (~(CRUINT64)(1 << 7))
#define CRVK_FLAG_1_ANISTROPY   (CRUINT64)(1 << 8)
#define CRVK_FLAG_0_ANISTROPY   (~(CRUINT64)(1 << 8))
#define CRVK_FLAG_1_BLENDGROUP  (CRUINT64)(1 << 9)
#define CRVK_FLAG_0_BLENDGROUP  (~(CRUINT64)(1 << 9))
#define CRVK_FLAG_1_TITLEBAR    (CRUINT64)(1 << 10)
#define CRVK_FLAG_0_TITLEBAR    (~(CRUINT64)(1 << 10))

#define CR_GET_FLAG_1_STAT(f, flag) (((f) & (flag)) == (flag))

//
typedef struct {
#ifdef CR_WINDOWS
    HWND window;
#elif defined CR_LINUX
    Window window;
    Display *dpy;
#endif
    CRUINT32 titlebarHeight;
    CRINT32 w, h;

    //标志位
    CRUINT64 flags;
    
    //设备相关
    
    // CRBOOL validation;
    // CRBOOL SRGB;
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkDebugReportCallbackEXT msgCallback;
    PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
    PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
    CRUINT32 queueFamilyCount;
    VkQueueFamilyProperties *queueFamilyProperties;
    CRUINT32 graphicsQueueFamilyIndex;
    CRUINT32 presentQueueFamilyIndex;
    VkDevice device;
    VkFormat inFormat;
    
    //队列相关

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;

    //指令池

    VkCommandPool drawCommandPool;
    VkCommandPool presentCommandPool;  //Optional

    //交换链-缓冲相关

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    CRUINT32 imageArrayLayers;
    VkExtent2D swapchainExtent;
    VkSwapchainKHR swapchain;
    CRUINT32 swapchainImageCount;
    //交换链图像及视图
    VkImage *swapchainImages;
    VkImageView *swapchainImageViews;
    //渲染管线
    VkSampler vkSampler;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;  //此项勿轻易改动
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    //帧缓冲
    VkFramebuffer *framebuffers;
    //指令缓冲
    VkCommandBuffer *drawBuffers;
    VkCommandBuffer *presentBuffers;

    //图像呈现相关

    CRUINT32 maxFrames;
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *frontFences;
    //
    CRUINT32 currentFrame;

    //互锁，避免在释放资源之后还在绘制
    CRLOCK itemLock;
    //

    //全局描述符
    VkBuffer *globalUniformBuffers;
    VkDeviceMemory *globalUniformDeviceMemories;
    //默认纹理
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView *textureImageViews;

    /**
     * 思路是：
     * 一个Uniform代表了一个实体，而这个实体可以自由选择网格（meshs）
     * 以及纹理（Textures），这样可以用同一个网格和纹理多处绘图。
     * 而且这些网格和纹理是可以自由组合的
     */

    CRRBTREE objectsTree;
    CRRBTREE meshesTree;
    CRRBTREE texturesTree;

    /**
     * 深度缓冲
     */

    VkFormat depthFormat;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    /**
     * 多重采样
     */

    VkSampleCountFlagBits msaaSamples;
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
    
    //
    CRGLOBALPROP globalProp;
    //
    CRRBTREE blendGroupTree;
}crvk, *pcrvk;

typedef struct {
    CRUINT32 w, h;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView *textureImageViews;
    VkSampler textureSampler;
    pcrvk vk;
}crvktexture, *pcrvktexture;

typedef struct {
    CRUINT64 vertexSize;
    CRUINT64 indexSize;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    pcrvk vk;
}crvkmesh, *pcrvkmesh;

typedef struct {
    pcrvk vk;
    //
    CRITEMPROP prop;
    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformDeviceMemories;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;
}crvkobject, *pcrvkobj;

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
);
void _inner_destroy_crvk_(pcrvk vk);
void _inner_paint_(pcrvk vk);
void _inner_resize_vk_(pcrvk vk, CRUINT32 w, CRUINT32 h);

/**
 * 内部使用的函数
 */

//物理设备
CRBOOL _inner_init_vk_device_(pcrvk vk);
void _inner_uninit_vk_device_(pcrvk vk);

//逻辑设备
CRBOOL _inner_create_logical_device_(pcrvk vk);
void _inner_destroy_logical_device_(pcrvk vk);

//队列相关
CRBOOL _inner_setup_vulkan_(pcrvk vk);
void _inner_setdown_vulkan_(pcrvk vk);

//指令相关
CRBOOL _inner_create_command_pool_(pcrvk vk);
void _inner_destroy_command_pool_(pcrvk vk);

//扩展功能相关
CRBOOL _inner_create_extension_resources_(pcrvk vk);
void _inner_destroy_extension_resources_(pcrvk vk);

//描述符布局相关
CRBOOL _inner_create_descriptor_set_layout_(pcrvk vk);
void _inner_destroy_descriptor_set_layout_(pcrvk vk);

//交换链相关
CRBOOL _inner_query_swapchain_image_count_(pcrvk vk);
CRBOOL _inner_create_swapchain_(pcrvk vk);
void _inner_destroy_swapchain_(pcrvk vk);

//指令缓冲相关
CRBOOL _inner_create_command_buffers_(pcrvk vk);
void _inner_destroy_command_buffers_(pcrvk vk);
//
void _inner_record_commands_(pcrvk vk);

//绘制图像相关
CRBOOL _inner_create_synchronization_(pcrvk vk);
void _inner_destroy_synchronization_(pcrvk vk);

//图像实体相关
CRBOOL _inner_create_trees_(pcrvk vk);
void _inner_destroy_trees_(pcrvk vk);

//全局描述符
CRBOOL _inner_create_global_descriptor_(pcrvk vk);
void  _inner_destroy_global_descriptor_(pcrvk vk);
void _inner_update_global_descriptor_(pcrvk vk);

//uniform相关
void _inner_update_uniform_buffer_(pcrvk vk, CRUINT32 imageIndex);

//

void _inner_set_vk_global_prop_(pcrvk vk, CRGLOBALPROP *pProp);
void _inner_set_titlebar_(pcrvk vk, CRBOOL draw);

#endif