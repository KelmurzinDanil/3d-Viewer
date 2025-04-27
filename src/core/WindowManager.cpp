#include "WindowManager.hpp"
WindowManager::WindowManager(int width, int height, const char* title) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
}
WindowManager::~WindowManager(){
    if (window_) {
        glfwDestroyWindow(window_);
    }
}
VkExtent2D WindowManager::getFramebufferSize() const { 
    int width, height;
    
    // Получаем реальные размеры фреймбуфера
    // Важно для корректного расчета в Vulkan
    glfwGetFramebufferSize(window_, &width, &height);
    return {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
}
bool WindowManager::shouldClose() const {
    return glfwWindowShouldClose(window_);
}

void WindowManager::pollEvents() const {
    glfwPollEvents();
}