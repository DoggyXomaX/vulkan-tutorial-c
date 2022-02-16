#include "HelloTriangleApplication.h"

/* PRIVATE VISIBILITY */
void Run( int argc, char *argv[] );
void InitWindow( void );
void MainLoop( void );
VkResult DrawFrame( void );
void Cleanup( void );
VkResult InitVulkan( void );
VkResult CreateVulkanInstance( void );
VkResult CreateSurface( void );
VkResult PickPhysicalDevice( void );
VkResult CreateLogicalDevice( void );
VkResult CreateSwapChain( void );
VkResult CreateGraphicsPipeline( void );
VkResult CreateRenderPass( void );
VkResult CreateFramebuffers( void );
VkResult CreateCommandPool( void );
VkResult CreateCommandBuffers( void );
VkResult CreateSemaphores( void );
void ClearFeatures( VkPhysicalDeviceFeatures* );
void GetDriverVersion( char*, uint32_t, uint32_t );
bool IsDeviceSuitable( VkPhysicalDevice );
bool CheckDeviceExtensionSupport( VkPhysicalDevice );
bool CheckValidationLayerSupport( void );
QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice );
SwapChainSupportDetails QuerySwapChainSupport( VkPhysicalDevice );
VkSurfaceFormatKHR ChooseSwapSurfaceFormat( const VkSurfaceFormatKHR*, uint32_t );
VkPresentModeKHR ChooseSwapPresentMode( const VkPresentModeKHR*, uint32_t );
VkExtent2D ChooseSwapExtent( const VkSurfaceCapabilitiesKHR );
VkResult CreateImageViews( void );
VkShaderModule CreateShaderModule( uint8_t*, uint32_t );
uint8_t *LoadFile( const char*, uint32_t* );

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
    .validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    },
    .window = NULL,
    .vkPhysicalDevice = VK_NULL_HANDLE,
    
    .swapChainImages = NULL,
    .swapChainImageLength = 0,
    .swapChainImageViews = NULL,

    .graphicsPipeline = NULL,

    .Run = Run
};

/* ENTRIES */
void Run( int argc, char *argv[] ) {
    entry( "Run" );

    app.argc = argc;
    app.argv = argv;
    for ( int i = 0; i < argc; i++ ) printf( "argv[%d]: \"%s\"\n", i, argv[ i ] );

    InitWindow();

    VkResult result = InitVulkan();
    if ( result != VK_SUCCESS ) {
        fail( "Run", "Vulkan Error %d\n", result );
    } else {
        MainLoop();
        ok( "Run" );
    }

    Cleanup();
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
        if ( DrawFrame() != VK_SUCCESS ) break;
    }

    ok( "MainLoop" );
}
VkResult DrawFrame() {
    uint32_t imageIndex;
    vkAcquireNextImageKHR(
        app.vkDevice,
        app.vkSwapchainKHR,
        UINT64_MAX,
        app.imageAvailableSemaphore,
        VK_NULL_HANDLE,
        &imageIndex
    );

    VkSemaphore waitSemaphores[] = { app.imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { app.renderFinishedSemaphore };
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &app.commandBuffers[ imageIndex ],
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    VkResult result = vkQueueSubmit( app.vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
    if ( result != VK_SUCCESS ) {
        fail( "DrawFrame", "failed to queue submit.\nError code: %d\n", result );
        return result;
    }

    VkSwapchainKHR swapChains[] = { app.vkSwapchainKHR };

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
        .pResults = NULL
    };

    vkQueuePresentKHR( app.vkPresentationQueue, &presentInfo );

    return VK_SUCCESS;
}
void Cleanup() {
    entry( "Cleanup" );

    puts( "Destroying semaphores" );
    if ( app.renderFinishedSemaphore ) vkDestroySemaphore( app.vkDevice, app.renderFinishedSemaphore, NULL );
    if ( app.imageAvailableSemaphore ) vkDestroySemaphore( app.vkDevice, app.imageAvailableSemaphore, NULL );

    puts( "Destroying command pool" );
    if ( app.commandPool ) vkDestroyCommandPool( app.vkDevice, app.commandPool, NULL );

    puts( "Destroying vk swap chain framebuffers" );
    if ( app.swapChainFramebuffers ) {
        for ( int i = 0; i < app.swapChainImageLength; i++ ) {
            if ( app.swapChainFramebuffers[ i ] ) {
                vkDestroyFramebuffer( app.vkDevice, app.swapChainFramebuffers[ i ], NULL );
            }
        }
        free( app.swapChainFramebuffers );
    }

    puts( "Destroying vk graphics pipeline..." );
    if ( app.graphicsPipeline ) vkDestroyPipeline( app.vkDevice, app.graphicsPipeline, NULL );

    puts( "Destroying vk pipeline layout..." );
    if ( app.pipelineLayout ) vkDestroyPipelineLayout( app.vkDevice, app.pipelineLayout, NULL );

    puts( "Destroying vk render pass..." );
    if ( app.renderPass ) vkDestroyRenderPass( app.vkDevice, app.renderPass, NULL );

    puts( "Cleaning Swap chain image views..." );
    if ( app.swapChainImageViews ) {
        for ( int i = 0; i < app.swapChainImageLength; i++ ) {
            vkDestroyImageView( app.vkDevice, app.swapChainImageViews[ i ], NULL );
        }
        free( app.swapChainImageViews );
    }

    puts( "Cleaning Swap chain images..." );
    if ( app.swapChainImages ) free( app.swapChainImages );

    puts( "Cleaning command buffers..." );
    if ( app.commandBuffers ) free( app.commandBuffers );

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

    VkResult result = CreateVulkanInstance();
    if ( result != VK_SUCCESS ) return result;

    result = CreateSurface();
    if ( result != VK_SUCCESS ) return result;

    result = PickPhysicalDevice();
    if ( result != VK_SUCCESS ) return result;

    result = CreateLogicalDevice();
    if ( result != VK_SUCCESS ) return result;

    result = CreateSwapChain();
    if ( result != VK_SUCCESS ) return result;

    result = CreateImageViews();
    if ( result != VK_SUCCESS ) return result;

    result = CreateRenderPass();
    if ( result != VK_SUCCESS ) return result;

    result = CreateGraphicsPipeline();
    if ( result != VK_SUCCESS ) return result;

    result = CreateFramebuffers();
    if ( result != VK_SUCCESS ) return result;

    result = CreateCommandPool();
    if ( result != VK_SUCCESS ) return result;

    result = CreateCommandBuffers();
    if ( result != VK_SUCCESS ) return result;

    result = CreateSemaphores();
    if ( result != VK_SUCCESS ) return result;

    ok( "InitVulkan" );
    return result;
}
VkResult CreateVulkanInstance() {
    entry( "CreateVulkanInstance" );

    if ( ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport() ) {
        fail( "CreateVulkanInstance", "validation layers requested, but not available!\n", NULL );
        return VK_ERROR_LAYER_NOT_PRESENT;
    }

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

    if ( ENABLE_VALIDATION_LAYERS ) {
        createInfo.enabledLayerCount = VALIDATION_LAYER_COUNT;
        createInfo.ppEnabledLayerNames = app.validationLayers;
    }

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
        bool isSupported = IsDeviceSuitable( devices[ i ] );
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

    QueueFamilyIndices indices = FindQueueFamilies( app.vkPhysicalDevice );
    
    int queueCount = indices.graphicsFamily.value == indices.presentationFamily.value ? 1 : 2;
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[ queueCount ];

    if ( queueCount != 1 ) {
        uint32_t uniqueQueueFamilies[] = {
            indices.graphicsFamily.value,
            indices.presentationFamily.value
        };
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
    } else {
        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = NULL,
            .queueFamilyIndex = indices.graphicsFamily.value,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos[ 0 ] = queueCreateInfo;
    }
    
    VkPhysicalDeviceFeatures deviceFeatures;
    ClearFeatures( &deviceFeatures );
    
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

    SwapChainSupportDetails details = QuerySwapChainSupport( app.vkPhysicalDevice );

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat( details.formats, details.formatsLength );
    VkPresentModeKHR presentMode = ChooseSwapPresentMode( details.presentModes, details.presentModesLength );
    VkExtent2D extent = ChooseSwapExtent( details.capabilities );

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
    QueueFamilyIndices indices = FindQueueFamilies( app.vkPhysicalDevice );
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
VkResult CreateGraphicsPipeline() {
    entry( "CreateGraphicsPipeline" );

    const char VERT_PATH[] = "shaders/vert.spv";
    const char FRAG_PATH[] = "shaders/frag.spv";
    char *vertPath, *fragPath;
    uint8_t *vertProgram, *fragProgram;
    uint32_t vertProgramLength, fragProgramLength;

    vertPath = GetRelativePath( app.argv[ 0 ], VERT_PATH, NULL );
    fragPath = GetRelativePath( app.argv[ 0 ], FRAG_PATH, NULL );
    puts( "Loading vertex shader..." );
    vertProgram = LoadFile( vertPath, &vertProgramLength );
    puts( "Loading fragment shader..." );
    fragProgram = LoadFile( fragPath, &fragProgramLength );
    free( vertPath );
    free( fragPath );

    if ( vertProgram == NULL || fragProgram == NULL ) {
        if ( vertProgram == NULL )
            fail( "CreateGraphicsPipeline", "failed to load vertex shader \"%s\"!\n", VERT_PATH );
        if ( fragProgram == NULL )
            fail( "CreateGraphicsPipeline", "failed to load fragment shader \"%s\"!\n", FRAG_PATH );
        if ( vertProgram != NULL ) free( vertProgram );
        if ( fragProgram != NULL ) free( fragProgram );
        return VK_ERROR_UNKNOWN;
    }

    VkShaderModule vertShaderModule = CreateShaderModule( vertProgram, vertProgramLength );
    VkShaderModule fragShaderModule = CreateShaderModule( fragProgram, fragProgramLength );
    free( vertProgram );
    free( fragProgram );
    if ( vertShaderModule == NULL || fragShaderModule == NULL ) {
        if ( vertShaderModule == NULL )
            fail( "CreateGraphicsPipeline", "failed to create vertex shader module!\n", NULL );
        if ( fragShaderModule == NULL )
            fail( "CreateGraphicsPipeline", "failed to create fragment shader module!\n", NULL );
        if ( vertShaderModule != NULL ) vkDestroyShaderModule( app.vkDevice, vertShaderModule, NULL );
        if ( fragShaderModule != NULL ) vkDestroyShaderModule( app.vkDevice, fragShaderModule, NULL );
        return VK_ERROR_UNKNOWN;
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .pNext = NULL,
        .module = vertShaderModule,
        .pName = "main"
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pNext = NULL,
        .module = fragShaderModule,
        .pName = "main",
        .pSpecializationInfo = NULL
    };
    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = NULL,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = NULL,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = NULL
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = ( float )app.swapChainExtent.width,
        .height = ( float )app.swapChainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = app.swapChainExtent
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = NULL,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = NULL,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = NULL,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = (
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
        ),
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = NULL,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };

    VkResult result = vkCreatePipelineLayout(
        app.vkDevice,
        &pipelineLayoutInfo,
        NULL,
        &app.pipelineLayout
    );
    if ( result != VK_SUCCESS ) {
        vkDestroyShaderModule( app.vkDevice, vertShaderModule, NULL );
        vkDestroyShaderModule( app.vkDevice, fragShaderModule, NULL );
        fail( "CreateGraphicsPipeline", "failed to create graphics pipeline.\nError code: %d\n", result );
        return result;
    }
    puts( "Pipeline layout created!" );

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = NULL,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = NULL,
        .pColorBlendState = &colorBlending,
        .pDynamicState = NULL,
        .pTessellationState = NULL,
        .layout = app.pipelineLayout,
        .renderPass = app.renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0
    };

    puts( "Debugging pipeline info:" );

    ////////////////////////
    printf( "sType == %d\n", pipelineInfo.sType );
    for ( int i = 0; i < pipelineInfo.stageCount; i++ ) {
        printf( "Stages[%d]: %d, %s, %s\n", i, pipelineInfo.pStages[ i ].sType, pipelineInfo.pStages[i].pName, pipelineInfo.pStages[i].pNext );
    }
    ////////////////////////
    
    result = vkCreateGraphicsPipelines(
        app.vkDevice,
        VK_NULL_HANDLE,
        1,
        &pipelineInfo,
        NULL,
        &app.graphicsPipeline
    );
    if ( result != VK_SUCCESS ) {
        fail( "CreateGraphicsPipeline", "failed to create graphics pipeline.\nError code: %d\n", result );
        vkDestroyShaderModule( app.vkDevice, vertShaderModule, NULL );
        vkDestroyShaderModule( app.vkDevice, fragShaderModule, NULL );
        return result;
    }
    puts( "Pipeline created!" );

    vkDestroyShaderModule( app.vkDevice, vertShaderModule, NULL );
    vkDestroyShaderModule( app.vkDevice, fragShaderModule, NULL );
    ok( "CreateGraphicsPipeline" );
    return VK_SUCCESS;
}
VkResult CreateRenderPass() {
    entry( "CreateRenderPass" );

    VkAttachmentDescription colorAttachment = {
        .format = app.swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = NULL,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    VkResult result = vkCreateRenderPass(
        app.vkDevice,
        &renderPassInfo,
        NULL,
        &app.renderPass
    );
    if ( result != VK_SUCCESS ) {
        fail( "CreateRenderPass", "failed to create render pass!\nError code: %d\n", result );
        return result;
    }

    ok( "CreateRenderPass" );
    return VK_SUCCESS;
}
VkResult CreateFramebuffers() {
    entry( "CreateFramebuffers" );

    app.swapChainFramebuffers = calloc( app.swapChainImageLength, sizeof( VkFramebuffer ) );

    for ( uint32_t i = 0; i < app.swapChainImageLength; i++ ) {
        VkImageView attachments[] = { app.swapChainImageViews[ i ] };

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = NULL,
            .renderPass = app.renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = app.swapChainExtent.width,
            .height = app.swapChainExtent.height,
            .layers = 1
        };

        VkResult result = vkCreateFramebuffer( app.vkDevice, &framebufferInfo, NULL, &app.swapChainFramebuffers[ i ] );
        if ( result != VK_SUCCESS ) {
            fail( "CreateFramebuffers", "failed to create framebuffer.\nError code: %d\n", result );
            return result;
        }
    }

    ok( "CreateFramebuffers" );

    return VK_SUCCESS;
}
VkResult CreateCommandPool() {
    entry( "CreateCommandPool" );

    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies( app.vkPhysicalDevice );

    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value,
        .flags = 0
    };

    VkResult result = vkCreateCommandPool( app.vkDevice, &poolInfo, NULL, &app.commandPool );
    if ( result != VK_SUCCESS ) {
        fail( "CreateCommandPool", "failed to create command pool.\nError code: %d\n", result );
        return result;
    }

    ok( "CreateCommandPool" );
    return VK_SUCCESS;
}
VkResult CreateCommandBuffers() {
    entry( "CreateCommandBuffers" );

    app.commandBuffers = calloc( app.swapChainImageLength, sizeof( VkCommandBuffer ) );

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = app.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = ( uint32_t )app.swapChainImageLength
    };

    VkResult result = vkAllocateCommandBuffers( app.vkDevice, &allocInfo, app.commandBuffers );
    if ( result != VK_SUCCESS ) {
        fail( "CreateCommandBuffers", "failed to allocate command buffers\nError code: %d\n", result );
        return result;
    }

    for ( int i = 0; i < app.swapChainImageLength; i++ ) {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = NULL,
            .flags = 0,
            .pInheritanceInfo = NULL
        };

        result = vkBeginCommandBuffer( app.commandBuffers[ i ], &beginInfo );
        if ( result != VK_SUCCESS ) {
            fail( "CreateCommandBuffers", "failed to begin command buffer.\nError code: %d\n", result );
            return result;
        }

        VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};

        VkRenderPassBeginInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = NULL,
            .renderPass = app.renderPass,
            .framebuffer = app.swapChainFramebuffers[ i ],
            .renderArea = {
                .offset = { 0, 0 },
                .extent = app.swapChainExtent
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor
        };

        vkCmdBeginRenderPass( app.commandBuffers[ i ], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
        vkCmdBindPipeline( app.commandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, app.graphicsPipeline );
        vkCmdDraw( app.commandBuffers[ i ], 3, 1, 0, 0 );
        vkCmdEndRenderPass( app.commandBuffers[ i ] );

        result = vkEndCommandBuffer( app.commandBuffers[ i ] );
        if ( result != VK_SUCCESS ) {
            fail( "CreateCommandBuffers", "failed to end up command buffer.\nError code: %d\n", result );
            return result;
        }
    }

    ok( "CreateCommandBuffers" );
    return VK_SUCCESS;
}
VkResult CreateSemaphores() {
    entry( "CreateSemaphores" );

    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL
    };

    VkResult result = vkCreateSemaphore( app.vkDevice, &semaphoreInfo, NULL, &app.imageAvailableSemaphore );
    if ( result != VK_SUCCESS ) {
        fail( "CreateSemaphores", "failed to create imageAvailable semaphore!\nError code: %d\n", result );
        return result;
    }

    result = vkCreateSemaphore( app.vkDevice, &semaphoreInfo, NULL, &app.renderFinishedSemaphore );
    if ( result != VK_SUCCESS ) {
        fail( "CreateSemaphores", "failed to create renderFinished semaphore!\nError code: %d\n", result );
        return result;
    }

    ok( "CreateSemaphores" );
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
    bool isExtensionsSupported = CheckDeviceExtensionSupport( device );
    bool isSwapChainAdequate = false;

    QueueFamilyIndices indices = FindQueueFamilies( device );
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties( device, &deviceProperties );
    vkGetPhysicalDeviceFeatures( device, &deviceFeatures );

    if ( isExtensionsSupported ) {
        SwapChainSupportDetails details = QuerySwapChainSupport( device );
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
bool CheckValidationLayerSupport() {
    method( "CheckValidationLayerSupport" );

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties( &layerCount, NULL );

    VkLayerProperties availableLayers[ layerCount ];
    vkEnumerateInstanceLayerProperties( &layerCount, availableLayers );

    for ( int i = 0; i < VALIDATION_LAYER_COUNT; i++ ) {
        bool layerFound = false;

        for ( int j = 0; j < layerCount; j++ ) {
            if ( strcmp( app.validationLayers[ i ], availableLayers[ j ].layerName ) == 0 ) {
                layerFound = true;
                break;
            } 
        }

        if ( !layerFound ) {
            ok_method( "CheckValidationLayerSupport not found" );
            return false;
        }
    }

    ok_method( "CheckValidationLayerSupport found" );
    return true;
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
        if ( !indices.graphicsFamily.isSet && queueFamilyProperties[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
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
VkShaderModule CreateShaderModule( uint8_t *shaderCode, uint32_t shaderCodeSize ) {
    method( "CreateShaderModule" );

    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .pCode = ( uint32_t* )shaderCode,
        .codeSize = shaderCodeSize
    };

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule( app.vkDevice, &createInfo, NULL, &shaderModule );
    if ( result != VK_SUCCESS ) {
        fail_method( "CreateShaderModule", "failed to create shader module\nError Code: %d\n", result );
        return NULL;
    }

    ok_method( "CreateShaderModule" );
    return shaderModule;
}
uint8_t *LoadFile( const char *filename, uint32_t *size ) {
    method( "LoadFile" );

    FILE *file = fopen( filename, "rb" );
    if ( file == NULL ) {
        fail_method( "LoadFile", "failed to read \"%s\" file!\n", filename );
        return NULL;
    }

    uint32_t index = 0;
    uint32_t pChunk = 0;
    int c = 0;
    uint8_t *fileBuffer = NULL;
    while ( ( c = getc( file ) ) != EOF ) {
        if ( index >= pChunk * FILE_CHUNK_SIZE ) {
            pChunk++;
            fileBuffer = realloc( fileBuffer, pChunk * FILE_CHUNK_SIZE );
        }
        fileBuffer[ index ] = ( uint8_t )c;
        index++;
    }
    fclose(file);

    if ( size != NULL ) *size = index;
    fileBuffer = realloc( fileBuffer, index );

    ok_method( "LoadFile" );
    return fileBuffer;
}
