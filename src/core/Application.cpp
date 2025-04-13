#include "Application.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

Application::Application()
    : window(nullptr), renderer(nullptr) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(800, 600, "3D Viewer", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    try {
        renderer = new VulkanRenderer(window);
    } catch (...) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
}

Application::~Application() {
    delete renderer;
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void Application::run() {
    mainLoop();
    renderer->cleanup();
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }
}

void Application::drawFrame() {
    renderer->drawFrame();
}