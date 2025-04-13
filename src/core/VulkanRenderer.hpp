#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#define NOMINMAX
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <iostream>
#include <stdexcept>
#include <set>
#include <algorithm>
#include <limits>      
#include <cstdint>
#include <fstream>
#include <array>
#include <cstring>


struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool isComplete() const { 
        return graphicsFamily.has_value() && presentFamily.has_value(); 
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanRenderer {
public:
    VulkanRenderer(GLFWwindow* win);
    ~VulkanRenderer();
    
    void initVulkan();
    void cleanup();
    void drawFrame();

private:
    // Константы и настройки
    const int MAX_FRAMES_IN_FLIGHT = 2;
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // Основные объекты Vulkan
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    
    // Очереди
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    // Цепочка подкачки
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;

    // Графический конвейер
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    // Команды
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    // Синхронизация
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    // Отладка
    VkDebugUtilsMessengerEXT debugMessenger;

    //===============================================
    // Инициализация и очистка
    //===============================================
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();

    //===============================================
    //  SwapChain и ImageViews
    //===============================================
    void createSwapChain();
    void recreateSwapChain();
    void cleanupSwapChain();
    void createImageViews();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    //===============================================
    // Графический конвейер
    //===============================================

    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    VkShaderModule createShaderModule(const std::vector<char>& code);
    static std::vector<char> readFile(const std::string& filename);

    //===============================================
    // Команды и синхронизация
    //===============================================

    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();
    void recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex);

    //===============================================
    // Вспомогательные утилиты
    //===============================================

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    //===============================================
    // Валидация и отладка
    //===============================================

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
        
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);
        
    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);
};

#endif 