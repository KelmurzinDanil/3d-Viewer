#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "core/Application.hpp"
#include <iostream>

int main() {
    std::cout << "Starting application..." << std::endl;
    Application app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}