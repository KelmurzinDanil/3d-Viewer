#include "VulkanRenderer.hpp"
#include "BasicTriangleStrategy.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult VulkanRenderer::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanRenderer::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VulkanRenderer::VulkanRenderer(GLFWwindow* win)
    : window(win),
      instance(nullptr, VulkanDeleter<VkInstance_T, vkDestroyInstance>()),
      debugMessenger(nullptr, DebugUtilsMessengerDeleter()),
      surface(nullptr, VulkanDeleter<VkSurfaceKHR_T, vkDestroySurfaceKHR, VkInstance>(nullptr)),
      physicalDevice(VK_NULL_HANDLE),
      device(nullptr, VulkanDeleter<VkDevice_T, vkDestroyDevice>()),
      swapChain(nullptr, VulkanDeleter<VkSwapchainKHR_T, vkDestroySwapchainKHR, VkDevice>(nullptr)),
      renderPass(nullptr, VulkanDeleter<VkRenderPass_T, vkDestroyRenderPass, VkDevice>(nullptr)),
      pipelineLayout(nullptr, VulkanDeleter<VkPipelineLayout_T, vkDestroyPipelineLayout, VkDevice>(nullptr)),
      graphicsPipeline(nullptr, VulkanDeleter<VkPipeline_T, vkDestroyPipeline, VkDevice>(nullptr)),
      commandPool(nullptr, VulkanDeleter<VkCommandPool_T, vkDestroyCommandPool, VkDevice>(nullptr)),
      imageAvailableSemaphore(nullptr, VulkanDeleter<VkSemaphore_T, vkDestroySemaphore, VkDevice>(nullptr)),
      renderFinishedSemaphore(nullptr, VulkanDeleter<VkSemaphore_T, vkDestroySemaphore, VkDevice>(nullptr)),
      inFlightFence(nullptr, VulkanDeleter<VkFence_T, vkDestroyFence, VkDevice>(nullptr))
{
    pipelineStrategy = std::make_unique<BasicTriangleStrategy>();
    initVulkan();
}


VulkanRenderer::~VulkanRenderer() {
    cleanup(); 
}

    void VulkanRenderer::initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createCommandBuffer();
        createSyncObjects();
    }

    void VulkanRenderer::cleanup() {
    // Очистите все объекты, связанные с устройством
    vkDeviceWaitIdle(device.get()); // Убедитесь, что устройство простаивает

    // Уничтожение объектов в правильном порядке
    swapChainFramebuffers.clear();
    swapChainImageViews.clear();
    swapChainImages.clear();  // Очистка изображений swapchain

    commandPool.reset();      // Уничтожение пула команд
    graphicsPipeline.reset(); // Уничтожение графического конвейера
    pipelineLayout.reset();   // Уничтожение макета конвейера
    renderPass.reset();       // Уничтожение прохода рендеринга
    swapChain.reset();        // Уничтожение цепочки подкачки

    // Уничтожение семафоров и заборов
    imageAvailableSemaphore.reset();
    renderFinishedSemaphore.reset();
    inFlightFence.reset();

    device.reset();           // Уничтожение логического устройства
    surface.reset();          // Уничтожение поверхности
    debugMessenger.reset();   // Уничтожение отладчика
    instance.reset();         // Уничтожение экземпляра Vulkan
}

    void VulkanRenderer::createInstance() {
        // Проверка поддержки валидационных слоев
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // Информация о приложении
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // Информация о создании экземпляра
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Получение необходимых расширений (включая GLFW)
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Настройка валидационных слоев и отладчика
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        // Создание экземпляра Vulkan
        VkInstance rawInstance;
        if (vkCreateInstance(&createInfo, nullptr, &rawInstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
        instance = VkInstancePtr(rawInstance,
            VulkanDeleter<VkInstance_T,
            vkDestroyInstance>());
        
    }

    void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void VulkanRenderer::setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        VkDebugUtilsMessengerEXT rawDebugMessenger;
        if (CreateDebugUtilsMessengerEXT(instance.get(), &createInfo, nullptr, &rawDebugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }

        debugMessenger = VkDebugUtilsMessengerEXTPtr(
            rawDebugMessenger,
            DebugUtilsMessengerDeleter(instance.get()) 
        );
    }

    void VulkanRenderer::createSurface() {
        VkSurfaceKHR rawSurface;
        if (glfwCreateWindowSurface(instance.get(), window, nullptr, &rawSurface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        surface = VkSurfaceKHRPtr(
            rawSurface,
            VulkanDeleter<VkSurfaceKHR_T, vkDestroySurfaceKHR, VkInstance>(
                instance.get()
            )
        );
    }

    void VulkanRenderer::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance.get(), &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance.get(), &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void VulkanRenderer::createLogicalDevice() {
        // Находим индексы семейств очередей
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        // Создаем информацию о создании очередей
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Определяем функциональность устройства
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Информация о создании логического устройства
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // Если graphics и present очереди принадлежат разным семействам,
        // используется режим совместного использования (concurrent sharing mode)
        // Если они совпадают, используется эксклюзивный режим (exclusive sharing mode)
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

         // Создаем логическое устройство
        VkDevice rawDevice;
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &rawDevice) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        device = VkDevicePtr(
            rawDevice,
            VulkanDeleter<VkDevice_T, vkDestroyDevice>()
        );
        
        // Получаем дескрипторы очередей
        vkGetDeviceQueue(device.get(), indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device.get(), indices.presentFamily.value(), 0, &presentQueue);
    }

    void VulkanRenderer::createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface.get();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkSwapchainKHR rawSwapChain;
        if (vkCreateSwapchainKHR(device.get(), &createInfo, nullptr, &rawSwapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        swapChain = VkSwapchainKHRPtr(rawSwapChain,
             VulkanDeleter<VkSwapchainKHR_T, vkDestroySwapchainKHR,
                           VkDevice>(device.get()));
        vkGetSwapchainImagesKHR(device.get(), swapChain.get(), &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device.get(), swapChain.get(), &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void VulkanRenderer::createImageViews() {
        swapChainImageViews.clear();
        swapChainImageViews.reserve(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkImageView rawView;
            if (vkCreateImageView(device.get(), &createInfo, nullptr, &rawView) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
            swapChainImageViews.emplace_back(
                rawView,
                VulkanDeleter<VkImageView_T, vkDestroyImageView, VkDevice>(device.get())
            );
        }
    }

    void VulkanRenderer::createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkRenderPass rawRenderPass;
        if (vkCreateRenderPass(device.get(), &renderPassInfo, nullptr, &rawRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }

        renderPass = VkRenderPassPtr(rawRenderPass, VulkanDeleter<VkRenderPass_T,
             vkDestroyRenderPass, VkDevice>(device.get()));
    }


    void VulkanRenderer::createFramebuffers() {
        swapChainFramebuffers.clear();
        swapChainFramebuffers.reserve(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                swapChainImageViews[i].get()
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass.get();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            VkFramebuffer rawFramebuffer;
            if (vkCreateFramebuffer(device.get(), &framebufferInfo, nullptr, &rawFramebuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }

            swapChainFramebuffers.emplace_back(
                rawFramebuffer,
                VulkanDeleter<VkFramebuffer_T, vkDestroyFramebuffer, VkDevice>(device.get()));
            }
    }

    void VulkanRenderer::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        VkCommandPool rawCommandPool;
        if (vkCreateCommandPool(device.get(), &poolInfo, nullptr, &rawCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
        commandPool = VkCommandPoolPtr(rawCommandPool, 
            VulkanDeleter<VkCommandPool_T, vkDestroyCommandPool, VkDevice>(device.get()));
    }

    void VulkanRenderer::createCommandBuffer() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool.get();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device.get(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass.get();
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex].get();
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.get());

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) swapChainExtent.width;
            viewport.height = (float) swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void VulkanRenderer::createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphore rawImageAvailableSemaphore;
        VkSemaphore rawRenderFinishedSemaphore;
        VkFence rawInFlightFence;

        if (vkCreateSemaphore(device.get(), &semaphoreInfo, nullptr, &rawImageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device.get(), &semaphoreInfo, nullptr, &rawRenderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device.get(), &fenceInfo, nullptr, &rawInFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects!");
        }

        imageAvailableSemaphore = VkSemaphorePtr(
            rawImageAvailableSemaphore,
            VulkanDeleter<VkSemaphore_T, vkDestroySemaphore, VkDevice>(
                device.get()
            )
        );

        renderFinishedSemaphore = VkSemaphorePtr(
            rawRenderFinishedSemaphore,
            VulkanDeleter<VkSemaphore_T, vkDestroySemaphore, VkDevice>(
                device.get()
            )
        );

        inFlightFence = VkFencePtr(
            rawInFlightFence,
            VulkanDeleter<VkFence_T, vkDestroyFence, VkDevice>(
                device.get()
            )
        );
    }

    void VulkanRenderer::drawFrame() {
        // Получаем сырые указатели из умных
        VkFence rawInFlightFence = inFlightFence.get();
        VkSemaphore rawImageAvailableSemaphore = imageAvailableSemaphore.get();
        VkSemaphore rawRenderFinishedSemaphore = renderFinishedSemaphore.get();
    
        // Ожидаем завершение предыдущего кадра
        vkWaitForFences(device.get(), 1, &rawInFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device.get(), 1, &rawInFlightFence);
    
        // Получаем индекс изображения из цепочки подкачки
        uint32_t imageIndex;
        vkAcquireNextImageKHR(
            device.get(), 
            swapChain.get(), 
            UINT64_MAX, 
            rawImageAvailableSemaphore, 
            VK_NULL_HANDLE, 
            &imageIndex
        );
    
        // Подготавливаем командный буфер
        vkResetCommandBuffer(commandBuffer, 0);
        recordCommandBuffer(commandBuffer, imageIndex);
    
        // Настраиваем информацию для отправки команд
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
        VkSemaphore waitSemaphores[] = {rawImageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
    
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
    
        VkSemaphore signalSemaphores[] = {rawRenderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
    
        // Отправляем командный буфер в очередь
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, rawInFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
    
        // Настраиваем информацию для отображения кадра
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
    
        VkSwapchainKHR swapChains[] = {swapChain.get()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
    
        // Отображаем кадр
        vkQueuePresentKHR(presentQueue, &presentInfo);
    }

    
    VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChainSupportDetails VulkanRenderer::querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.get(), &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.get(), &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.get(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.get(), &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.get(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) {
        // Находим семейства очередей
        QueueFamilyIndices indices = findQueueFamilies(device);

        // Проверяем поддержку необходимых расширений
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        // Проверяем адекватность swapchain
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        // Устройство считается подходящим, если:
        // 1. Найдены все необходимые семейства очередей
        // 2. Поддерживаются необходимые расширения
        // 3. Swapchain имеет хотя бы один поддерживаемый формат и режим представления
        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface.get(), &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    std::vector<const char*> VulkanRenderer::getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool VulkanRenderer::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }   
    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
    void VulkanRenderer::createGraphicsPipeline() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        
        VkPipelineLayout rawLayout;
        if (vkCreatePipelineLayout(device.get(), &pipelineLayoutInfo, nullptr, &rawLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        pipelineLayout = VkPipelineLayoutPtr(
            rawLayout, 
            VulkanDeleter<VkPipelineLayout_T, vkDestroyPipelineLayout, VkDevice>(device.get())
        );
    
        // Исправление: передаем сырые указатели, а не перемещаем умные
        graphicsPipeline = pipelineStrategy->createGraphicsPipeline(
            device.get(),          
            renderPass.get(),      
            pipelineLayout.get() 
        );
    }

