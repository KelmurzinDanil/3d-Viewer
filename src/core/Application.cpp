#include "Application.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

Application::Application() : enableValidationLayers(true) {
    try {
        initializeGLFW();
        initializeManagers();
        initializeRenderer();
    } catch (...) {
        cleanup();
        throw;
    }
}
Application::~Application() {
    cleanup();
}
void Application::initializeGLFW() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
}

void Application::initializeManagers() {
    windowManager = std::make_unique<WindowManager>(800, 600, "3D Viewer");
    instanceManager = std::make_unique<InstanceManager>(enableValidationLayers);
    surfaceManager = std::make_unique<SurfaceManager>(*instanceManager, *windowManager);
    deviceManager = std::make_unique<DeviceManager>(*instanceManager, *surfaceManager);
    swapChainManager = std::make_unique<SwapChainManager>(*deviceManager, *surfaceManager, *windowManager);
    pipelineManager = std::make_unique<PipelineManager>(*deviceManager, *swapChainManager);
    commandManager = std::make_unique<CommandManager>(*deviceManager, *swapChainManager, *pipelineManager);
    bufferManager = std::make_unique<BufferManager>(
        *deviceManager, commandManager->getCommandPool(), *swapChainManager
    );
    textureManager = std::make_unique<TextureManager>(
            *bufferManager, 
            *deviceManager, 
            *swapChainManager
        );

    
}

void Application::initializeRenderer() {
    renderer = std::make_unique<VulkanRenderer>(
        *windowManager,
        *deviceManager,
        *swapChainManager,
        *pipelineManager,
        *instanceManager,
        *surfaceManager,
        *commandManager,
        *bufferManager,
        *textureManager, 
        enableValidationLayers
    );
}

void Application::cleanup() {
    if (deviceManager && deviceManager->device()) {
        vkDeviceWaitIdle(deviceManager->device()); // Ждём завершения всех операций GPU
    }

    renderer.reset();
    textureManager.reset();
    bufferManager.reset();
    commandManager.reset();
    swapChainManager.reset();
    pipelineManager.reset();
    deviceManager.reset();
    surfaceManager.reset();
    instanceManager.reset();
    windowManager.reset(); 
    glfwTerminate();
}
void Application::run() {
    mainLoop();
}

void Application::mainLoop() {
    while (!windowManager->shouldClose()) {
        windowManager->pollEvents();
        drawFrame();
    }
}

void Application::drawFrame() {
    renderer->drawFrame();
}