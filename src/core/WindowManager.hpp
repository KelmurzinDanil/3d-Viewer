#define GLFW_INCLUDE_VULKAN
#pragma once
#include <GLFW/glfw3.h>

class WindowManager {
public:
    WindowManager(int width, int height, const char* title);
    ~WindowManager();
    
    bool shouldClose() const;
    void pollEvents() const;

    GLFWwindow* window() const { return window_; }

    VkExtent2D getFramebufferSize() const;
    
private:
    GLFWwindow* window_;
};


