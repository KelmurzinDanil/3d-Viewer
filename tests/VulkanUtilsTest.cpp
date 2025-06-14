#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "VulkanUtils.hpp"

namespace fs = std::filesystem;

TEST(VulkanUtilsTest, ReadFileSuccess) {
    // Создаем временный файл
    const std::string testContent = "Hello Vulkan!";
    const auto tmpPath = fs::temp_directory_path() / "test_file.bin";
    
    // Записываем данные в файл
    std::ofstream(tmpPath, std::ios::binary).write(testContent.data(), testContent.size());
    
    // Читаем и проверяем
    auto data = VulkanUtils::readFile(tmpPath.string());
    EXPECT_EQ(testContent.size(), data.size());
    EXPECT_TRUE(std::equal(testContent.begin(), testContent.end(), data.begin()));
    
    fs::remove(tmpPath); // Удаляем временный файл
}

TEST(VulkanUtilsTest, ReadFileNotFound) {
    EXPECT_THROW(
        VulkanUtils::readFile("non_existent_file.bin"),
        std::runtime_error
    );
}
TEST(VulkanUtilsTest, HasStencilComponent) {
    EXPECT_TRUE(VulkanUtils::hasStencilComponent(VK_FORMAT_D32_SFLOAT_S8_UINT));
    EXPECT_TRUE(VulkanUtils::hasStencilComponent(VK_FORMAT_D24_UNORM_S8_UINT));
    EXPECT_FALSE(VulkanUtils::hasStencilComponent(VK_FORMAT_D32_SFLOAT));
    EXPECT_FALSE(VulkanUtils::hasStencilComponent(VK_FORMAT_R8G8B8A8_UNORM));
}
