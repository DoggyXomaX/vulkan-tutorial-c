#ifndef __HELLO_TRIANGLE_APPLICATION__
#define __HELLO_TRIANGLE_APPLICATION__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <GLFW/glfw3native.h>

#include <stdio.h>
#include <stdbool.h>
#include "utils.h"

#define entry(x) puts("[Entry] "x)
#define ok(x) puts("~ "x)
#define fail(x,err,params) printf("[Error] "err"\n~ "x, params)
#define method(x) puts("\t[Method] "x)
#define ok_method(x) puts("\t~ "x)
#define fail_method(x,err,params) printf("\t[Error] "err"\n~ "x, params)

typedef struct {
    bool isSet;
    uint32_t value;
} FamilyIndex;

typedef struct {
    bool error;
    FamilyIndex graphicsFamily;
    FamilyIndex presentationFamily;
} QueueFamilyIndices;

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t formatsLength;
    VkPresentModeKHR *presentModes;
    uint32_t presentModesLength;
} SwapChainSupportDetails;

#define DEVICE_EXTENSION_COUNT 1
#define VALIDATION_LAYER_COUNT 1
#define FILE_CHUNK_SIZE 8192

#ifdef NDEBUG
    #define ENABLE_VALIDATION_LAYERS false
#else
    #define ENABLE_VALIDATION_LAYERS true
#endif

typedef struct {
    int argc;
    char **argv;

    int width;
    int height;
    const char *title;
    const char *applicationName;
    const char *engineName;

    const char *deviceExtensions[ DEVICE_EXTENSION_COUNT ];
    const char *validationLayers[ VALIDATION_LAYER_COUNT ];
    GLFWwindow *window;
    VkInstance vkInstance;
    VkPhysicalDevice vkPhysicalDevice;
    VkDevice vkDevice;
    VkQueue vkGraphicsQueue;
    VkQueue vkPresentationQueue;
    VkSurfaceKHR vkSurfaceKHR;
    
    VkSwapchainKHR vkSwapchainKHR;
    VkImage *swapChainImages;
    VkImageView *swapChainImageViews;
    uint32_t swapChainImageLength;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkFramebuffer *swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    void ( *Run )( int, char** );
} AppProperties;

extern AppProperties app;

#endif
