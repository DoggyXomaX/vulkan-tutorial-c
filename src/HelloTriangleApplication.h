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

#define debug_entry(x) puts(":::: "x" ::::")
#define debug_method(x) puts("\t>>>> "x)

typedef struct {
    bool isSet;
    uint32_t value;
} FamilyIndex;

typedef struct {
    bool error;
    FamilyIndex graphicsFamily;
} QueueFamilyIndices;

typedef struct {
    int width;
    int height;
    const char *title;
    const char *applicationName;
    const char *engineName;
    GLFWwindow *window;
    VkInstance vkInstance;
    VkPhysicalDevice vkPhysicalDevice;
    VkDevice vkDevice;
    VkQueue vkGraphicsQueue;
    VkSurfaceKHR vkSurfaceKHR;

    void ( *Run )( void );
    void ( *InitWindow )( void );
    void ( *MainLoop )( void );
    void ( *Cleanup )( void );
    VkResult ( *InitVulkan )( void );
    VkResult ( *CreateVulkanInstance )( void );
    VkResult ( *CreateSurface )( void );
    VkResult ( *PickPhysicalDevice )( void );
    VkResult ( *CreateLogicalDevice )( void );
    void ( *ClearFeatures )( VkPhysicalDeviceFeatures* );
    void ( *GetDriverVersion )( char*, uint32_t, uint32_t );
    bool ( *IsDeviceSuitable )( VkPhysicalDevice );
    QueueFamilyIndices ( *FindQueueFamilies )( VkPhysicalDevice );
} AppProperties;

extern AppProperties app;

#endif
