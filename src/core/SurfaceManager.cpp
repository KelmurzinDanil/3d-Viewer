#include "SurfaceManager.hpp"

SurfaceManager::SurfaceManager(InstanceManager& instanceManager, WindowManager& windowManager)
    : instanceManager_(instanceManager),windowManager_(windowManager),
    surface_(nullptr, VulkanDeleter<VkSurfaceKHR_T, vkDestroySurfaceKHR, VkInstance>(instanceManager_.instance()))
{
    createSurface(windowManager_.window());
}
void SurfaceManager::createSurface(GLFWwindow* window_) {
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(instanceManager_.instance(), window_, nullptr, &rawSurface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    surface_ = VkSurfaceKHRPtr(
        rawSurface,
        VulkanDeleter<VkSurfaceKHR_T, vkDestroySurfaceKHR, VkInstance>(
            instanceManager_.instance()
        )
    );
}