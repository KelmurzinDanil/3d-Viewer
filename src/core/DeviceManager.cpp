#include "DeviceManager.hpp"

DeviceManager::DeviceManager(InstanceManager& instanceManager,SurfaceManager& surfaceManager) 
    : instanceManager_(instanceManager), surfaceManager_(surfaceManager){
    //if (!window_) {
    //    throw std::runtime_error("Window is null!");
    //}
    pickPhysicalDevice();
    createLogicalDevice();
}

void DeviceManager::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instanceManager_.instance(), &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instanceManager_.instance(), &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice_ = device;
            break;
        }
    }

    if (physicalDevice_ == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void DeviceManager::createLogicalDevice() {
    // Находим индексы семейств очередей
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);

    
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

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions_.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions_.data();

    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

     // Создаем логическое устройство
    VkDevice rawDevice;
    if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &rawDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    device_ = VkDevicePtr(
        rawDevice,
        VulkanDeleter<VkDevice_T, vkDestroyDevice>()
    );

}

QueueFamilyIndices DeviceManager::findQueueFamilies(VkPhysicalDevice device) {
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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surfaceManager_.surface(), &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    if (!indices.isComplete()) {
        throw std::runtime_error("Failed to find required queue families!");
    }

    return indices;
}
bool DeviceManager::isDeviceSuitable(VkPhysicalDevice device) {
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
SwapChainSupportDetails DeviceManager::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;
    
    // 1. Получение базовых возможностей поверхности
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surfaceManager_.surface(), &details.capabilities);
    
    // 2. Получение поддерживаемых форматов
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surfaceManager_.surface(), &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surfaceManager_.surface(), &formatCount, details.formats.data());
    }
    
    // 3. Получение поддерживаемых режимов представления
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surfaceManager_.surface(), &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surfaceManager_.surface(), &presentModeCount, details.presentModes.data());
    }
    
    return details;
}
bool DeviceManager::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions_.begin(), deviceExtensions_.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

