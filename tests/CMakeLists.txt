# Подключаем GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Включаем модуль GoogleTest для использования gtest_discover_tests
include(GoogleTest)  

# Создаем тестовый исполняемый файл
add_executable(VulkanTests
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanUtilsTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanDeleterTest.cpp
)
add_custom_command(TARGET VulkanTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/model
    $<TARGET_FILE_DIR:VulkanTests>/model
)

add_custom_command(TARGET VulkanTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/textures
    $<TARGET_FILE_DIR:VulkanTests>/textures
)

# Копирование шейдеров
add_custom_command(TARGET VulkanTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:VulkanTests>/shaders
)

target_include_directories(VulkanTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include             # ← Путь к общим заголовкам
    ${PROJECT_SOURCE_DIR}/src/core  
    ${PROJECT_SOURCE_DIR}/External/GLFW/glfw-3.4.bin.WIN32/include
    ${PROJECT_SOURCE_DIR}/External/GLEW/glew-2.1.0/include
    "C:/VulkanSDK/1.4.309.0/Include"
    ${PROJECT_SOURCE_DIR}/External/glm
    ${PROJECT_SOURCE_DIR}/External/stb_image
    ${PROJECT_SOURCE_DIR}/External/tiny_obj_loader
)

target_compile_definitions(VulkanTests PRIVATE GLM_ENABLE_EXPERIMENTAL MODEL_PATH="model/viking.obj")
target_compile_definitions(VulkanTests PRIVATE 
    SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders"
)

target_link_libraries(VulkanTests PRIVATE
    GTest::gtest_main
    ${PROJECT_SOURCE_DIR}/External/GLFW/glfw-3.4.bin.WIN32/lib-vc2022/glfw3.lib
    ${PROJECT_SOURCE_DIR}/External/GLEW/glew-2.1.0/lib/Release/Win32/glew32.lib
    "C:/VulkanSDK/1.4.309.0/Lib/vulkan-1.lib"
    opengl32  
    gdi32     
)




# Настройки компилятора
target_compile_options(VulkanTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Werror -pedantic>
)

# Регистрация тестов
gtest_discover_tests(VulkanTests)

