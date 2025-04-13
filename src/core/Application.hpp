#ifndef APPLICATION_H
#define APPLICATION_H

#include "VulkanRenderer.hpp"

class Application {
public:
    Application();
    ~Application();
    void run();
    void mainLoop();
    void drawFrame();

private:
    GLFWwindow* window;
    VulkanRenderer* renderer;
};

#endif