#define GLFW_INCLUDE_VULKAN
#pragma once
#include "InstanceManager.hpp"
#include "WindowManager.hpp"
#include <GLFW/glfw3.h>

class SurfaceManager {
public:
    SurfaceManager(const SurfaceManager&) = delete;
    SurfaceManager& operator=(const SurfaceManager&) = delete;

    SurfaceManager(InstanceManager& instanceManager, WindowManager& windowManager);

    VkSurfaceKHR surface() const { return surface_.get(); }

private:
    WindowManager& windowManager_;
    InstanceManager& instanceManager_;
    VkSurfaceKHRPtr surface_;
    
    void createSurface(GLFWwindow* window);
};