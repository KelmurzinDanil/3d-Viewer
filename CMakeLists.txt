
cmake_minimum_required(VERSION 3.20)
project(VulkanApp)

# Копировать шейдеры в бинарную директорию
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)


if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:MSVCRT")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/core/VulkanRenderer.cpp
    src/core/Application.cpp
    src/core/PipelineBuilder.cpp
    src/core/CommandManager.cpp
    src/core/DeviceManager.cpp
    src/core/InstanceManager.cpp
    src/core/SwapChainManager.cpp
    src/core/SurfaceManager.cpp
    src/core/WindowManager.cpp
    src/core/PipelineManager.cpp
    src/core/BufferManager.cpp
    src/core/Vertex.cpp
    src/core/Constants.cpp
    src/core/TextureManager.cpp
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/textures
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/textures
)
# Определение макроса SHADER_DIR как строки
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/External/GLFW/glfw-3.4.bin.WIN32/include
    ${PROJECT_SOURCE_DIR}/External/GLEW/glew-2.1.0/include
    "C:/VulkanSDK/1.4.309.0/Include"
    ${PROJECT_SOURCE_DIR}/External/glm
    ${PROJECT_SOURCE_DIR}/External/stb_image
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/External/GLFW/glfw-3.4.bin.WIN32/lib-vc2022/glfw3.lib
    ${PROJECT_SOURCE_DIR}/External/GLEW/glew-2.1.0/lib/Release/Win32/glew32.lib
    "C:/VulkanSDK/1.4.309.0/Lib/vulkan-1.lib"
    opengl32  
    gdi32     
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
endif()