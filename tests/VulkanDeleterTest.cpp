#include <gtest/gtest.h>
#include "VulkanTypes.hpp"

TEST(VulkanDeleterTest, VkInstanceDeletion) {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkInstance rawInstance = VK_NULL_HANDLE;
    ASSERT_EQ(vkCreateInstance(&createInfo, nullptr, &rawInstance), VK_SUCCESS);
    
    {
        // Оборачиваем в умный указатель
        VkInstancePtr instancePtr(rawInstance);
        EXPECT_NE(instancePtr.get(), VK_NULL_HANDLE);
        
        // При выходе из области видимости должен вызваться деструктор
    }
    
}