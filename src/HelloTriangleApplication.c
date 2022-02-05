#include "HelloTriangleApplication.h"

/* VISIBILITY */
void Run( void );
void InitWindow( void );
void MainLoop( void );
void Cleanup( void );
VkResult InitVulkan( void );
VkResult CreateVulkanInstance( void );
VkResult CreateSurface( void );
VkResult PickPhysicalDevice( void );
VkResult CreateLogicalDevice( void );
VkResult CreateSwapChain( void );
void ClearFeatures( VkPhysicalDeviceFeatures* );
void GetDriverVersion( char*, uint32_t, uint32_t );
bool IsDeviceSuitable( VkPhysicalDevice );
bool CheckDeviceExtensionSupport( VkPhysicalDevice );
QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice );
SwapChainSupportDetails QuerySwapChainSupport( VkPhysicalDevice );
VkSurfaceFormatKHR ChooseSwapSurfaceFormat( const VkSurfaceFormatKHR*, uint32_t );
VkPresentModeKHR ChooseSwapPresentMode( const VkPresentModeKHR*, uint32_t );
VkExtent2D ChooseSwapExtent( const VkSurfaceCapabilitiesKHR );
VkResult CreateImageViews( void );

/* APP */
AppProperties app = { 
    .width = 400,
    .height = 400,
    .title = "App Window",
    .applicationName = "Hello Triangle",
    .engineName = "Test Engine",

    .deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    },
    .window = NULL,
    .vkPhysicalDevice = VK_NULL_HANDLE,
    
    .swapChainImages = NULL,
    .swapChainImageLength = 0,
    .swapChainImageViews = NULL,

    .Run = Run,
    .InitWindow = InitWindow,
    .MainLoop = MainLoop,
    .Cleanup = Cleanup,

    .InitVulkan = InitVulkan,
    .CreateSurface = CreateSurface,
    .CreateVulkanInstance = CreateVulkanInstance,
    .PickPhysicalDevice = PickPhysicalDevice,
    .CreateLogicalDevice = CreateLogicalDevice,
    .CreateSwapChain = CreateSwapChain,
    .CreateImageViews = CreateImageViews,

    .ClearFeatures = ClearFeatures,
    .GetDriverVersion = GetDriverVersion,
    .IsDeviceSuitable = IsDeviceSuitable,
    .CheckDeviceExtensionSupport = CheckDeviceExtensionSupport,
    .FindQueueFamilies = FindQueueFamilies,
    .QuerySwapChainSupport = QuerySwapChainSupport,
    .ChooseSwapSurfaceFormat = ChooseSwapSurfaceFormat,
    .ChooseSwapPresentMode = ChooseSwapPresentMode,
    .ChooseSwapExtent = ChooseSwapExtent
};

/* ENTRIES */
void Run( void ) {
    entry( "Run" );

    app.InitWindow();

    VkResult result = app.InitVulkan();
    if ( result != VK_SUCCESS ) {
        fail( "Run", "Vulkan Error %d\n", result );
    } else {
        app.MainLoop();
        ok( "Run" );
    }

    app.Cleanup();
}
void InitWindow() {
    entry( "InitWindow" );

    glfwInit();

    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    app.window = glfwCreateWindow( app.width, app.height, app.title, NULL, NULL );

    ok( "InitWindow" );
}
void MainLoop() {
    entry( "MainLoop" );

    while ( !glfwWindowShouldClose( app.window ) ) {
        glfwPollEvents();
    }

    ok( "MainLoop" );
}
void Cleanup() {
    entry( "Cleanup" );

    puts( "Cleaning Swap chain image views..." );
    if ( app.swapChainImageViews ) {
        for ( int i = 0; i < app.swapChainImageLength; i++ ) {
            vkDestroyImageView( app.vkDevice, app.swapChainImageViews[ i ], NULL );
        }
        free( app.swapChainImageViews );
    }

    puts( "Cleaning Swap chain images..." );
    if ( app.swapChainImages ) {
        free( app.swapChainImages );
    }

    puts( "Cleaning Vulkan and glfw..." );
    vkDestroySwapchainKHR( app.vkDevice, app.vkSwapchainKHR, NULL );
    vkDestroyDevice( app.vkDevice, NULL );
    vkDestroySurfaceKHR( app.vkInstance, app.vkSurfaceKHR, NULL );
    vkDestroyInstance( app.vkInstance, NULL );
    glfwDestroyWindow( app.window );
    glfwTerminate();

    ok( "Cleanup" );
}

/* VULKAN ENTRIES */
VkResult InitVulkan() {
    entry( "InitVulkan" );

    VkResult result = app.CreateVulkanInstance();
    if ( result != VK_SUCCESS ) {
        fail( "InitVulkan", "failed to create vulkan instance!\n", NULL );
        return result;
    }

    result = app.CreateSurface();
    if ( result != VK_SUCCESS ) {
        fail( "InitVulkan", "failed to create window surface!\n", NULL );
        return result;
    }

    result = app.PickPhysicalDevice();
    if ( result != VK_SUCCESS ) {
        fail( "InitVulkan", "failed to pick physical device!\n", NULL );
        return result;
    }

    result = app.CreateLogicalDevice();
    if ( result != VK_SUCCESS ) {
        fail( "InitVulkan", "failed to create logical device!\n", NULL );
        return result;
    }

    result = app.CreateSwapChain();
    if ( result != VK_SUCCESS ) {
        fail( "InitVulkan", "failed to create swap chain!\n", NULL );
        return result;
    }

    result = app.CreateImageViews();
    if ( result != VK_SUCCESS ) {
        fail( "InitVulkan", "failed to create image views!\n", NULL );
        return result;
    }

    ok( "InitVulkan" );
    return result;
}
VkResult CreateVulkanInstance() {
    entry( "CreateVulkanInstance" );

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

    ok( "CreateVulkanInstance" );

    return vkCreateInstance( &createInfo, NULL, &app.vkInstance );
}
VkResult CreateSurface() {
    entry( "CreateSurface" );

    ok( "CreateSurface" );

    return glfwCreateWindowSurface(
        app.vkInstance,
        app.window,
        NULL,
        &app.vkSurfaceKHR
    );
}
VkResult PickPhysicalDevice() {
    entry( "PickPhysicalDevice" );

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices( app.vkInstance, &deviceCount, NULL );
    if ( deviceCount == 0 ) {
        fail( "PickPhysicalDevice", "No Graphical Devices was found!\n", NULL );
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
        fail( "PickPhysicalDevice", "no supported Graphical Devices was found!\n", NULL );
        return VK_ERROR_DEVICE_LOST;
    }

    ok( "PickPhysicalDevice" );
    return VK_SUCCESS;
}
VkResult CreateLogicalDevice() {
    entry( "CreateLogicalDevice" );

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
        .enabledExtensionCount = DEVICE_EXTENSION_COUNT,
        .ppEnabledExtensionNames = app.deviceExtensions,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL
    };

    VkResult result = vkCreateDevice( app.vkPhysicalDevice, &createInfo, NULL, &app.vkDevice );
    if ( result != VK_SUCCESS ) {
        fail( "CreateLogicalDevice", "failed to create device %d\n", result );
        return result;
    }

    vkGetDeviceQueue( app.vkDevice, indices.graphicsFamily.value, 0, &app.vkGraphicsQueue );
    vkGetDeviceQueue( app.vkDevice, indices.presentationFamily.value, 0, &app.vkPresentationQueue );

    ok( "CreateLogicalDevice" );
    return VK_SUCCESS;
}
VkResult CreateSwapChain() {
    entry( "CreateSwapChain" );

    SwapChainSupportDetails details = app.QuerySwapChainSupport( app.vkPhysicalDevice );

    VkSurfaceFormatKHR surfaceFormat = app.ChooseSwapSurfaceFormat( details.formats, details.formatsLength );
    VkPresentModeKHR presentMode = app.ChooseSwapPresentMode( details.presentModes, details.presentModesLength );
    VkExtent2D extent = app.ChooseSwapExtent( details.capabilities );

    free( details.formats );
    free( details.presentModes );

    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if ( details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount ) {
        imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .surface = app.vkSurfaceKHR,

        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,

        .preTransform = details.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };
    QueueFamilyIndices indices = app.FindQueueFamilies( app.vkPhysicalDevice );
    if ( indices.graphicsFamily.value != indices.presentationFamily.value ) {
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value, indices.presentationFamily.value };
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    VkResult result = vkCreateSwapchainKHR( app.vkDevice, &createInfo, NULL, &app.vkSwapchainKHR );
    if ( result != VK_SUCCESS ) {
        fail( "CreateSwapChain", "vkCreateSwapchainKHR error\n", NULL );
        return result;
    }
    
    result = vkGetSwapchainImagesKHR( app.vkDevice, app.vkSwapchainKHR, &app.swapChainImageLength, NULL );
    if ( result != VK_SUCCESS ) {
        fail( "CreateSwapChain", "vkGetSwapchainImagesKHR\n", NULL );
        return result;
    }

    app.swapChainImages = ( VkImage* )calloc( imageCount, sizeof( VkImage ) );
    result = vkGetSwapchainImagesKHR( app.vkDevice, app.vkSwapchainKHR, &app.swapChainImageLength, app.swapChainImages );
    if ( result != VK_SUCCESS ) {
        fail( "CreateSwapChain", "vkGetSwapchainImagesKHR\n", NULL );
        return result;
    }

    app.swapChainImageFormat = surfaceFormat.format;
    app.swapChainExtent = extent;

    ok( "CreateSwapChain" );
    return VK_SUCCESS;
}
VkResult CreateImageViews() {
    entry( "CreateImageViews" );

    printf( "Creating %d image views\n", app.swapChainImageLength );
    app.swapChainImageViews = ( VkImageView* )calloc( app.swapChainImageLength, sizeof( VkImageView ) );

    for ( int i = 0; i < app.swapChainImageLength; i++ ) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .image = app.swapChainImages[ i ],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = app.swapChainImageFormat,
            .components = { 0, 0, 0, 0 },
            .subresourceRange = { 1, 0, 1, 0, 1 }
        };

        VkResult result = vkCreateImageView( app.vkDevice, &createInfo, NULL, &app.swapChainImageViews[ i ] );
        if ( result != VK_SUCCESS ) {
            fail( "CreateImageViews", "failed create image view on index %d\n", i );
            return result;
        } 
    }

    ok( "CreateImageViews" );
    return VK_SUCCESS;
}

/* METHODS */
void ClearFeatures( VkPhysicalDeviceFeatures *pFeatures ) {
    method( "ClearFeatures" );

    int length = sizeof( VkPhysicalDeviceFeatures ) / sizeof( VkBool32 );
    VkBool32 *p = ( VkBool32* )pFeatures;
    for ( int i = 0; i < length; i++, p++ ) *p = 0;

    ok_method( "ClearFeatures" );
}
void GetDriverVersion( char *output_str, uint32_t vendor_id, uint32_t driver_version ) {
    method( "GetDriverVersion" );
    // NVIDIA version conventions
    if ( vendor_id == 0x10DE ) {
        sprintf( output_str, "%d.%d.%d.%d",
            ( driver_version >> 22 ) & 0x03FF,
            ( driver_version >> 14 ) & 0x00FF,
            ( driver_version >> 6  ) & 0x00FF,
            ( driver_version >> 0  ) & 0x003F
		);
        ok_method( "GetDriverVersion" );
        return;
    }

    // Intel version conventions
    if ( vendor_id == 0x8086 ) {
        sprintf( output_str, "%d.%d",
            ( driver_version >> 14 ),
            ( driver_version >> 0  ) & 0x3FFF
        );
        ok_method( "GetDriverVersion" );
        return;
    }

    // Vulkan version conventions
    sprintf( output_str, "%d.%d.%d",
        ( driver_version >> 22 ),
        ( driver_version >> 12 ) & 0x03FF,
        ( driver_version >> 0  ) & 0x0FFF
    );
    ok_method( "GetDriverVersion" );
}
bool IsDeviceSuitable( VkPhysicalDevice device ) {
    method( "IsDeviceSuitable" );

    bool isSupported = false;
    bool isExtensionsSupported = app.CheckDeviceExtensionSupport( device );
    bool isSwapChainAdequate = false;

    QueueFamilyIndices indices = FindQueueFamilies( device );
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties( device, &deviceProperties );
    vkGetPhysicalDeviceFeatures( device, &deviceFeatures );

    if ( isExtensionsSupported ) {
        SwapChainSupportDetails details = app.QuerySwapChainSupport( device );
        isSwapChainAdequate = details.formatsLength != 0 && details.presentModesLength != 0;
    }

    isSupported = (
        deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader &&
        !indices.error && indices.graphicsFamily.isSet && indices.presentationFamily.isSet &&
        isExtensionsSupported &&
        isSwapChainAdequate
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

    ok_method( "IsDeviceSuitable" );

    return isSupported;
}
bool CheckDeviceExtensionSupport( VkPhysicalDevice device ) {
    method( "CheckDeviceExtensionSupport" );

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties( device, NULL, &extensionCount, NULL );
    VkExtensionProperties availableExtensions[ extensionCount ];
    vkEnumerateDeviceExtensionProperties( device, NULL, &extensionCount, availableExtensions );

    int checkedExtensionCount = 0;

    for ( int i = 0; i < extensionCount; i++ ) {
        VkExtensionProperties prop = availableExtensions[ i ];
        for ( int j = 0; j < DEVICE_EXTENSION_COUNT; j++ ) {
            if ( strcmp( prop.extensionName, app.deviceExtensions[ j ] ) == 0 ) {
                checkedExtensionCount++;
                break;
            }
        }
        if ( checkedExtensionCount == DEVICE_EXTENSION_COUNT ) {
            ok_method( "CheckDeviceExtensionSupport" );
            return true;
        }
    }

    fail_method( "CheckDeviceExtensionSupport", "your gpu not supported required extensions!\n", NULL );
    return false;
}
QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice device ) {
    method( "FindQueueFamilies" );

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

    ok_method( "FindQueueFamilies" );

    return indices;
}
SwapChainSupportDetails QuerySwapChainSupport( VkPhysicalDevice device ) {
    method( "QuerySwapChainSupport" );

    SwapChainSupportDetails details = {
        .formats = NULL,
        .formatsLength = 0,
        .presentModes = NULL,
        .presentModesLength = 0
    };
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, app.vkSurfaceKHR, &details.capabilities );

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR( device, app.vkSurfaceKHR, &formatCount, NULL );
    if ( formatCount != 0 ) {
        VkSurfaceFormatKHR *formats = ( VkSurfaceFormatKHR* )calloc( formatCount, sizeof( VkSurfaceFormatKHR ) );
        vkGetPhysicalDeviceSurfaceFormatsKHR( device, app.vkSurfaceKHR, &formatCount, formats );
        details.formats = formats;
        details.formatsLength = formatCount;
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR( device, app.vkSurfaceKHR, &presentModeCount, NULL );
    if ( presentModeCount != 0 ) {
        VkPresentModeKHR *presentModes = ( VkPresentModeKHR* )calloc( presentModeCount, sizeof( VkPresentModeKHR ) );
        vkGetPhysicalDeviceSurfacePresentModesKHR( device, app.vkSurfaceKHR, &presentModeCount, presentModes );
        details.presentModes = presentModes;
        details.presentModesLength = presentModeCount;
    }

    ok_method( "QuerySwapChainSupport" );
    return details;
}
VkSurfaceFormatKHR ChooseSwapSurfaceFormat( const VkSurfaceFormatKHR* formats, uint32_t formatCount ) {
    method( "ChooseSwapSurfaceFormat" );

    int selected = -1;
    for ( int i = 0; i < formatCount; i++ ) {
        printf( "\t\t" );
        if ( selected == -1 &&
             formats[ i ].format == VK_FORMAT_B8G8R8A8_SRGB &&
             formats[ i ].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR ) {
            printf( "> " );
            selected = i;
        }
        printf( "Format: %u, ColorSpace: %u\n", formats[ i ].format, formats[ i ].colorSpace );
    }

    ok_method( "ChooseSwapSurfaceFormat" );
    return formats[ selected == -1 ? 0 : selected ];
}
VkPresentModeKHR ChooseSwapPresentMode( const VkPresentModeKHR *presentModes, uint32_t presentModeCount ) {
    method( "ChooseSwapPresentMode" );

    int selected = -1;
    for ( int i = 0; i < presentModeCount; i++ ) {
        printf( "\t\t" );
        if ( selected == -1 && presentModes[ i ] == VK_PRESENT_MODE_FIFO_KHR ) {
            selected = i;
            printf( "> " );
        }
        printf( "Present: %u\n", presentModes[ i ] );
    }

    ok_method( "ChooseSwapPresentMode" );
    return presentModes[ selected == -1 ? 0 : selected ];
}
VkExtent2D ChooseSwapExtent( const VkSurfaceCapabilitiesKHR capabilities ) {
    method( "ChooseSwapExtent" );

    VkExtent2D actualExtent;

    if ( capabilities.currentExtent.width != UINT32_MAX ) {
        actualExtent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize( app.window, &width, &height );

        uint32_t minWidth = capabilities.minImageExtent.width;
        uint32_t minHeight = capabilities.minImageExtent.height;
        uint32_t maxWidth = capabilities.maxImageExtent.width;
        uint32_t maxHeight = capabilities.maxImageExtent.height;
        
        actualExtent.width = clamp( width, minWidth, maxWidth );
        actualExtent.height = clamp( height, minHeight, maxHeight );
    }

    printf( "\t\tExtent size: %ux%u\n", actualExtent.width, actualExtent.height );
    ok_method( "ChooseSwapExtent" );
    return actualExtent;
}
