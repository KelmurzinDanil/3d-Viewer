#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include "VulkanTypes.hpp"



class InstanceManager {
public:
    InstanceManager(const InstanceManager&) = delete;
    InstanceManager& operator=(const InstanceManager&) = delete;

    InstanceManager(bool enableValidation);

    VkInstance instance() const { return instance_.get(); }
    bool enableValidationLayers() const {return enableValidationLayers_;}
    const std::vector<const char*> validationLayers() const {return validationLayers_;}
private:
    void createInstance();
    void setupDebugMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions() const;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    VkInstancePtr instance_;
    VkDebugUtilsMessengerEXTPtr debugMessenger_;
    bool enableValidationLayers_;
    const std::vector<const char*> validationLayers_ = {"VK_LAYER_KHRONOS_validation"};
};