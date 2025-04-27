#pragma once
#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>
#include "InstanceManager.hpp"
#include "VulkanTypes.hpp"
#include "VulkanUtils.hpp"
#include "SurfaceManager.hpp"



struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool isComplete() const { 
        return graphicsFamily.has_value() && presentFamily.has_value(); 
    }
};

class DeviceManager {
public:
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;

    DeviceManager(InstanceManager& instanceManager, SurfaceManager& surfaceManager);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    VkDevice device() const { return device_.get(); }
    VkPhysicalDevice physicalDevice() const { return physicalDevice_; }



private:
    InstanceManager& instanceManager_;
    SurfaceManager& surfaceManager_;

    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevicePtr device_;

    const std::vector<const char*> deviceExtensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createRenderPass();
    
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
};