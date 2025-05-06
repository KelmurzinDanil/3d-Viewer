#ifndef APPLICATION_H
#define APPLICATION_H

#include "VulkanRenderer.hpp"

class Application {
public:
    Application();
    ~Application();


    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;


    Application(Application&&) = default;
    Application& operator=(Application&&) = default;


    void run();
    
    void initializeGLFW();
    void initializeManagers();
    void initializeRenderer();
    void cleanup();
    void mainLoop();
    void drawFrame();

private:

    bool enableValidationLayers = true;
    std::unique_ptr<WindowManager> windowManager;
    std::unique_ptr<InstanceManager> instanceManager;
    std::unique_ptr<SurfaceManager> surfaceManager;
    std::unique_ptr<DeviceManager> deviceManager;
    std::unique_ptr<SwapChainManager> swapChainManager;
    std::unique_ptr<PipelineManager> pipelineManager;
    std::unique_ptr<CommandManager> commandManager;
    std::unique_ptr<BufferManager> bufferManager;
    std::unique_ptr<VulkanRenderer> renderer;
};

#endif