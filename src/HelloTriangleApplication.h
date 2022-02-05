#ifndef __HELLO_TRIANGLE_APPLICATION__
#define __HELLO_TRIANGLE_APPLICATION__

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <stdio.h>

#ifndef bool
    #include <stdbool.h>
#endif

#include "utils.h"

#define entry(x) puts("[Entry]: "x" {")
#define ok(x) puts("} // "x)
#define fail(x,err,params) printf("[Error]: "err"\n} // "x, params)
#define method(x) puts("\t[Method]: "x" {")
#define ok_method(x) puts("\t} // "x)
#define fail_method(x,err,params) printf("\t[Error]: "err"\n} // "x, params)

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

typedef struct {
    int width;
    int height;
    const char *title;
    const char *applicationName;
    const char *engineName;

    const char *deviceExtensions[ DEVICE_EXTENSION_COUNT ];
    GLFWwindow *window;
    VkInstance vkInstance;
    VkPhysicalDevice vkPhysicalDevice;
    VkDevice vkDevice;
    VkQueue vkGraphicsQueue;
    VkQueue vkPresentationQueue;
    VkSurfaceKHR vkSurfaceKHR;
    
    VkSwapchainKHR vkSwapchainKHR;
    VkImage *swapChainImages;
    uint32_t swapChainImagesLength;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    void ( *Run )( void );
    void ( *InitWindow )( void );
    void ( *MainLoop )( void );
    void ( *Cleanup )( void );
    
    VkResult ( *InitVulkan )( void );
    VkResult ( *CreateVulkanInstance )( void );
    VkResult ( *CreateSurface )( void );
    VkResult ( *PickPhysicalDevice )( void );
    VkResult ( *CreateLogicalDevice )( void );
    VkResult ( *CreateSwapChain )( void );
    
    void ( *ClearFeatures )( VkPhysicalDeviceFeatures* );
    void ( *GetDriverVersion )( char*, uint32_t, uint32_t );
    bool ( *IsDeviceSuitable )( VkPhysicalDevice );
    bool ( *CheckDeviceExtensionSupport )( VkPhysicalDevice );
    QueueFamilyIndices ( *FindQueueFamilies )( VkPhysicalDevice );
    SwapChainSupportDetails ( *QuerySwapChainSupport )( VkPhysicalDevice );
    VkSurfaceFormatKHR ( *ChooseSwapSurfaceFormat )( const VkSurfaceFormatKHR*, uint32_t );
    VkPresentModeKHR ( *ChooseSwapPresentMode )( const VkPresentModeKHR*, uint32_t );
    VkExtent2D ( *ChooseSwapExtent )( const VkSurfaceCapabilitiesKHR );
} AppProperties;

extern AppProperties app;

#endif
