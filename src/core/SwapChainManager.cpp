#include "SwapChainManager.hpp"

SwapChainManager::SwapChainManager(DeviceManager& deviceManager, SurfaceManager& surfaceManager, WindowManager& windowManager) 
: deviceManager(deviceManager),surfaceManager(surfaceManager),windowManager(windowManager),
  swapChain(nullptr, VulkanDeleter<VkSwapchainKHR_T, vkDestroySwapchainKHR, VkDevice>(deviceManager.device())),
  renderPass(nullptr, VulkanDeleter<VkRenderPass_T, vkDestroyRenderPass, VkDevice>(nullptr)),
  depthImageView(nullptr, VulkanDeleter<VkImageView_T, vkDestroyImageView, VkDevice>(nullptr)),
depthImage(nullptr, VulkanDeleter<VkImage_T, vkDestroyImage, VkDevice>(nullptr)),
depthImageMemory(nullptr, VulkanDeleter<VkDeviceMemory_T, vkFreeMemory, VkDevice>(nullptr))
  {
    createSwapChain();

    // Получаем очереди после создания свопчейна
    QueueFamilyIndices indices = deviceManager.findQueueFamilies(deviceManager.physicalDevice());
    vkGetDeviceQueue(deviceManager.device(), indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(deviceManager.device(), indices.presentFamily.value(), 0, &presentQueue);
    
    createRenderPass();
    createImageViews();
    createDepthResources();
    createFramebuffers();
}

void SwapChainManager::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = deviceManager.querySwapChainSupport(deviceManager.physicalDevice());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats); // Пространство
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes); // Мод отображения
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities); // Текущий размер области вывода

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surfaceManager.surface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // изображения - цель рендеринга (не для чтения или других операций).

    QueueFamilyIndices indices = deviceManager.findQueueFamilies(deviceManager.physicalDevice());
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // изображение может использоваться несколькими семействами очередей без синхронизации.
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // изображение принадлежит одному семейству очередей.
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // Поворот/отражение
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Без  Альфа канала
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // разрешает драйверу оптимизировать рендеринг (например, не рисовать перекрытые окном области).

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR rawSwapChain;
    if (vkCreateSwapchainKHR(deviceManager.device(), &createInfo, nullptr, &rawSwapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    swapChain = VkSwapchainKHRPtr(rawSwapChain,
         VulkanDeleter<VkSwapchainKHR_T, vkDestroySwapchainKHR,
                       VkDevice>(deviceManager.device()));
    vkGetSwapchainImagesKHR(deviceManager.device(), swapChain.get(), &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(deviceManager.device(), swapChain.get(), &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

VkImageViewPtr SwapChainManager::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1; // Не используем минимаппинг
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1; // Не используем текстуру-массив

        VkImageView rawView;
        if (vkCreateImageView(deviceManager.device(), &viewInfo, nullptr, &rawView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }

        return VkImageViewPtr(
        rawView,
        VulkanDeleter<VkImageView_T, vkDestroyImageView, VkDevice>(deviceManager.device()));
    }
void SwapChainManager::createImageViews() {
    swapChainImageViews.clear();
    swapChainImageViews.reserve(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews.emplace_back(
            createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT));
    }

}
VkSurfaceFormatKHR SwapChainManager::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChainManager::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { //< - лучший мод, но не поддерживается всеми видеокартами
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR; // < - Если ничего не подошло
}

VkExtent2D SwapChainManager::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
       
        VkExtent2D actualExtent = windowManager.getFramebufferSize();
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void SwapChainManager::createRenderPass() {

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VulkanUtils::findDepthFormat(deviceManager.physicalDevice());
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Без мультисэмплинга
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Очистить перед использованием
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Не сохранять после использования
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Трафарет не используется
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // Начальное состояние не важно
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Оптимально для теста глубины

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1; // Индекс в массиве attachments
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Состояние во время рендеринга
    
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Без Мультисемплинга
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//Очищаем буфер до рендера
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;// Сохраняем в память после рендера результат
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Трафарет, не используем 
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Неизвестно начальное состояние изображения 
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Используем для вывода на экран

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //Работаем с colorAttachment

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Работаем с графическим конвеером 
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pColorAttachments = &colorAttachmentRef;// Буфер цвета
    subpass.pDepthStencilAttachment = &depthAttachmentRef; // Буфер глубины

    
    VkSubpassDependency dependency{}; //Синхронизация 
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); 
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass; // Подзадачи 
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass rawRenderPass;
    if (vkCreateRenderPass(deviceManager.device(), &renderPassInfo, nullptr, &rawRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    renderPass = VkRenderPassPtr(rawRenderPass, VulkanDeleter<VkRenderPass_T,
         vkDestroyRenderPass, VkDevice>(deviceManager.device()));
}

void SwapChainManager::createImage(uint32_t width,
                                uint32_t height,
                                VkFormat format,
                                VkImageTiling tiling,
                                VkImageUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkImagePtr& image,
                                VkDeviceMemoryPtr& imageMemory) 
{

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1; // Без мипмаппинга
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Начальное состояние
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // Без мультисемплинга
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Для одной очереди

    VkImage rawImage;
    if (vkCreateImage(deviceManager.device(), &imageInfo, nullptr, &rawImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }
    image.reset(rawImage); // Передаем владение умному указателю


    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(deviceManager.device(), image.get(), &memRequirements);

    VkMemoryAllocateInfo allocInfo{}; //Выделям память
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanUtils::findMemoryType(deviceManager.physicalDevice(), memRequirements.memoryTypeBits, properties);

    VkDeviceMemory rawMemory;
    if (vkAllocateMemory(deviceManager.device(), &allocInfo, nullptr, &rawMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }
    imageMemory.reset(rawMemory);

    
    vkBindImageMemory(deviceManager.device(), image.get(), imageMemory.get(), 0); // привязываем память к изображению 
}

void SwapChainManager::createDepthResources() {
    VkFormat depthFormat = VulkanUtils::findDepthFormat(deviceManager.physicalDevice());

    createImage(swapChainExtent.width,
                swapChainExtent.height,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, //использование как буфера глубины
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // быстрая видеопамять GPU
                depthImage,
                depthImageMemory);
    depthImageView = createImageView(depthImage.get(), depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void SwapChainManager::createFramebuffers() {
    swapChainFramebuffers.clear();
    swapChainFramebuffers.reserve(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {

        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i].get(),
            depthImageView.get()
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass.get();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1; // для обычного 2D-рендеринга

        VkFramebuffer rawFramebuffer;
        if (vkCreateFramebuffer(deviceManager.device(), &framebufferInfo, nullptr, &rawFramebuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }

        swapChainFramebuffers.emplace_back(
            rawFramebuffer,
            VulkanDeleter<VkFramebuffer_T, vkDestroyFramebuffer, VkDevice>(deviceManager.device()));
        }
}
void SwapChainManager::recreateSwapChain() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(windowManager.window(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(deviceManager.device());

    //cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createDepthResources();
    createFramebuffers();
}
