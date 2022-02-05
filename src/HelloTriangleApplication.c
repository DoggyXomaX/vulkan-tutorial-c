#include "HelloTriangleApplication.h"

void Run( void );
void InitWindow( void );
void MainLoop( void );
void Cleanup( void );
VkResult InitVulkan( void );
VkResult CreateVulkanInstance( void );
VkResult CreateSurface( void );
VkResult PickPhysicalDevice( void );
VkResult CreateLogicalDevice( void );
void ClearFeatures( VkPhysicalDeviceFeatures* );
void GetDriverVersion( char*, uint32_t, uint32_t );
bool IsDeviceSuitable( VkPhysicalDevice );
QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice );

AppProperties app = { 
    .width = 400,
    .height = 400,
    .title = "App Window",
    .applicationName = "Hello Triangle",
    .engineName = "Test Engine",

    .window = NULL,
    .vkPhysicalDevice = VK_NULL_HANDLE,

    .Run = Run,
    .InitWindow = InitWindow,
    .MainLoop = MainLoop,
    .Cleanup = Cleanup,
    .InitVulkan = InitVulkan,
    .CreateSurface = CreateSurface,
    .CreateVulkanInstance = CreateVulkanInstance,
    .PickPhysicalDevice = PickPhysicalDevice,
    .CreateLogicalDevice = CreateLogicalDevice,
    .ClearFeatures = ClearFeatures,
    .GetDriverVersion = GetDriverVersion,
    .IsDeviceSuitable = IsDeviceSuitable,
    .FindQueueFamilies = FindQueueFamilies
};

void Run( void ) {
    debug_entry( "Run" );

    app.InitWindow();

    VkResult result = app.InitVulkan();
    if ( result != VK_SUCCESS ) {
        printf( "Vulkan Error code: %d\n", result );
    } else {
        app.MainLoop();
    }

    app.Cleanup();
}
void InitWindow() {
    debug_entry( "InitWindow" );

    glfwInit();

    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    app.window = glfwCreateWindow( app.width, app.height, app.title, NULL, NULL );
}
void MainLoop() {
    debug_entry( "MainLoop" );

    while ( !glfwWindowShouldClose( app.window ) ) {
        glfwPollEvents();
    }
}
void Cleanup() {
    debug_entry( "Cleanup" );

    vkDestroyDevice( app.vkDevice, NULL );
    vkDestroySurfaceKHR( app.vkInstance, app.vkSurfaceKHR, NULL );
    vkDestroyInstance( app.vkInstance, NULL );
    glfwDestroyWindow( app.window );
    glfwTerminate();
}
VkResult InitVulkan() {
    debug_entry( "InitVulkan" );

    VkResult result = app.CreateVulkanInstance();
    if ( result != VK_SUCCESS ) {
        puts( "Error: failed to create vulkan instance!" );
        return result;
    }

    result = app.CreateSurface();
    if ( result != VK_SUCCESS ) {
        puts( "Error: failed to create window surface!" );
        return result;
    }

    result = app.PickPhysicalDevice();
    if ( result != VK_SUCCESS ) {
        puts( "Error: failed to pick physical device!" );
        return result;
    }

    result = app.CreateLogicalDevice();
    if ( result != VK_SUCCESS ) {
        puts( "Error: failed to create logical device!" );
        return result;
    }

    return result;
}
VkResult CreateVulkanInstance() {
    debug_entry( "CreateVulkanInstance" );

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = app.applicationName,
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName = app.engineName,
        .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion = VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL
    };

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    puts( "glfwExtensions:" );
    for ( int i = 0; i < glfwExtensionCount; i++ )
        printf( "\t\"%s\"\n", glfwExtensions[ i ] );

    uint32_t vkExtCount = 0;
    vkEnumerateInstanceExtensionProperties( NULL, &vkExtCount, NULL );
    VkExtensionProperties vkExtensions[ vkExtCount ];
    vkEnumerateInstanceExtensionProperties( NULL, &vkExtCount, vkExtensions );
    puts( "vkInstanceExtensions:" );
    for ( int i = 0; i < vkExtCount; i++ ) {
        printf( "\t\"%s\" (v%u.0)\n", 
            vkExtensions[ i ].extensionName, 
            vkExtensions[ i ].specVersion 
        );
    }

    return vkCreateInstance( &createInfo, NULL, &app.vkInstance );
}
VkResult CreateSurface() {
    debug_entry( "CreateSurface" );

    return glfwCreateWindowSurface(
        app.vkInstance,
        app.window,
        NULL,
        &app.vkSurfaceKHR
    );
}
VkResult PickPhysicalDevice() {
    debug_entry( "PickPhysicalDevice" );

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices( app.vkInstance, &deviceCount, NULL );
    if ( deviceCount == 0 ) {
        puts( "Error: No Graphical Devices was found!" );
        return VK_ERROR_DEVICE_LOST;
    }
    VkPhysicalDevice devices[ deviceCount ];
    vkEnumeratePhysicalDevices( app.vkInstance, &deviceCount, devices );
    puts( "devices:" );

    bool isNotSupported = true;
    for ( int i = 0; i < deviceCount; i++ ) {
        bool isSupported = app.IsDeviceSuitable( devices[ i ] );
        if ( isSupported ) {
            app.vkPhysicalDevice = devices[ i ];
            isNotSupported = false;
        }
    }

    if ( isNotSupported ) {
        puts( "Error: no supported Graphical Devices was found!" );
        return VK_ERROR_DEVICE_LOST;
    }

    return VK_SUCCESS;
}
VkResult CreateLogicalDevice() {
    debug_entry( "CreateLogicalDevice" );

    QueueFamilyIndices indices = app.FindQueueFamilies( app.vkPhysicalDevice );
    
    int queueCount = 2;
    VkDeviceQueueCreateInfo queueCreateInfos[ queueCount ];
    uint32_t uniqueQueueFamilies[] = {
        indices.graphicsFamily.value,
        indices.presentationFamily.value
    };

    float queuePriority = 1.0f;
    for ( int i = 0; i < queueCount; i++ ) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = NULL,
            .queueFamilyIndex = uniqueQueueFamilies[ i ],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos[ i ] = queueCreateInfo;
    }
    

    VkPhysicalDeviceFeatures deviceFeatures;
    app.ClearFeatures( &deviceFeatures );
    
    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .pEnabledFeatures = &deviceFeatures,
        .queueCreateInfoCount = queueCount,
        .pQueueCreateInfos = queueCreateInfos,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = NULL,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL
    };

    VkResult result = vkCreateDevice( app.vkPhysicalDevice, &createInfo, NULL, &app.vkDevice );
    if ( result != VK_SUCCESS ) return result;

    vkGetDeviceQueue( app.vkDevice, indices.graphicsFamily.value, 0, &app.vkGraphicsQueue );
    vkGetDeviceQueue( app.vkDevice, indices.presentationFamily.value, 0, &app.vkPresentationQueue );

    return VK_SUCCESS;
}
void ClearFeatures( VkPhysicalDeviceFeatures *pFeatures ) {
    int length = sizeof( VkPhysicalDeviceFeatures ) / sizeof( VkBool32 );
    VkBool32 *p = ( VkBool32* )pFeatures;
    for ( int i = 0; i < length; i++, p++ ) *p = 0;
}
void GetDriverVersion( char *output_str, uint32_t vendor_id, uint32_t driver_version ) {
    // NVIDIA version conventions
    if ( vendor_id == 0x10DE ) {
        sprintf( output_str, "%d.%d.%d.%d",
            ( driver_version >> 22 ) & 0x03FF,
            ( driver_version >> 14 ) & 0x00FF,
            ( driver_version >> 6  ) & 0x00FF,
            ( driver_version >> 0  ) & 0x003F
		);
        return;
    }

    // Intel version conventions
    if ( vendor_id == 0x8086 ) {
        sprintf( output_str, "%d.%d",
            ( driver_version >> 14 ),
            ( driver_version >> 0  ) & 0x3FFF
        );
        return;
    }

    // Vulkan version conventions
    sprintf( output_str, "%d.%d.%d",
        ( driver_version >> 22 ),
        ( driver_version >> 12 ) & 0x03FF,
        ( driver_version >> 0  ) & 0x0FFF
    );
}
bool IsDeviceSuitable( VkPhysicalDevice device ) {
    debug_method( "IsDeviceSuitable" );

    bool isSupported = false;

    QueueFamilyIndices indices = FindQueueFamilies( device );

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties( device, &deviceProperties );
    vkGetPhysicalDeviceFeatures( device, &deviceFeatures );

    isSupported = (
        deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        deviceFeatures.geometryShader &&
        !indices.error && 
        indices.graphicsFamily.isSet && indices.presentationFamily.isSet
    );

    char driverVersion[ 64 ];
    GetDriverVersion( driverVersion, deviceProperties.vendorID, deviceProperties.driverVersion );

    printf( 
        "%s\n"
        "\t%s (Device ID: %u) (Driver version: %s)\n"
        "\tDiscrete: %s\n"
        "\tGeometry Shader support: %s\n",
        isSupported ? "[ OK ]" : "[ ERROR ]",
        deviceProperties.deviceName,
        deviceProperties.deviceID,
        driverVersion,
        deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? "Yes" : "No",
        deviceFeatures.geometryShader ? "Yes" : "No"
    );

    return isSupported;
}
QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice device ) {
    debug_method( "FindQueueFamilies" );

    QueueFamilyIndices indices = {
        .error = true,
        .graphicsFamily = { 
            .isSet = false,
            .value = 0
        },
        .presentationFamily = {
            .isSet = false,
            .value = 0
        }
    };

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, NULL );
    VkQueueFamilyProperties queueFamilyProperties[ queueFamilyCount ];
    vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilyProperties );

    for ( int i = 0; i < queueFamilyCount; i++ ) {
        if ( queueFamilyProperties[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
            indices.graphicsFamily.isSet = true;
            indices.graphicsFamily.value = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR( device, i, app.vkSurfaceKHR, &presentSupport );

        if ( presentSupport ) {
            indices.presentationFamily.isSet = true;
            indices.presentationFamily.value = i;
        }

        if ( indices.graphicsFamily.isSet && indices.presentationFamily.isSet ) {
            indices.error = false;
            break;
        }
    }

    return indices;
}
