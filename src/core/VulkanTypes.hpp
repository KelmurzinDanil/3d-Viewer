#pragma once
#include <vulkan/vulkan.h>
#include <memory>
/**
 * @brief Шаблонный удалитель для Vulkan объектов
 * Реализует RAII-управление ресурсами Vulkan через std::unique_ptr
 * 
 * @tparam T Тип Vulkan объекта (например, VkInstance_T)
 * @tparam DeleteFunc Функция удаления Vulkan (например, vkDestroyInstance)
 * @tparam DeleteArgs Типы дополнительных аргументов функции удаления
 */
template <typename T, auto DeleteFunc, typename... DeleteArgs>
struct VulkanDeleter {
    VulkanDeleter(DeleteArgs... args) : args(std::forward<DeleteArgs>(args)...) {}

    void operator()(T* obj) const {
        if (obj) {
             // Распаковываем сохраненные аргументы и вызываем функцию удаления
            std::apply([&](auto&&... params) {
                DeleteFunc(params..., obj, nullptr); 
            }, args);
        }
    }

private:
    std::tuple<DeleteArgs...> args; ///< Сохраненные аргументы для функции удаления
};

/**
 * @brief Специализированный удалитель для VkDebugUtilsMessengerEXT
 * Требует ручного получения функции-деструктора через vkGetInstanceProcAddr
 */
struct DebugUtilsMessengerDeleter {
    VkInstance instance; ///< Экземпляр Vulkan, необходимый для получения функции

    DebugUtilsMessengerDeleter(VkInstance inst = VK_NULL_HANDLE) : instance(inst) {}

    void operator()(VkDebugUtilsMessengerEXT messenger) const {
        if (messenger != VK_NULL_HANDLE && instance != VK_NULL_HANDLE) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func) func(instance, messenger, nullptr);
        }
    }
};


using VkDebugUtilsMessengerEXTPtr = std::unique_ptr<VkDebugUtilsMessengerEXT_T, DebugUtilsMessengerDeleter>;

using VkDescriptorSetLayoutPtr = std::unique_ptr<VkDescriptorSetLayout_T,
    VulkanDeleter<VkDescriptorSetLayout_T, vkDestroyDescriptorSetLayout, VkDevice>>;

using VkDescriptorPoolPtr = std::unique_ptr<VkDescriptorPool_T,
    VulkanDeleter<VkDescriptorPool_T, vkDestroyDescriptorPool, VkDevice>>;

using VkDeviceMemoryPtr = std::unique_ptr<VkDeviceMemory_T,
    VulkanDeleter<VkDeviceMemory_T, vkFreeMemory, VkDevice>>;

using VkBufferPtr = std::unique_ptr<VkBuffer_T,
    VulkanDeleter<VkBuffer_T, vkDestroyBuffer, VkDevice>>;

using VkInstancePtr = std::unique_ptr<VkInstance_T, 
    VulkanDeleter<VkInstance_T, vkDestroyInstance>>;

using VkDevicePtr = std::unique_ptr<VkDevice_T, 
    VulkanDeleter<VkDevice_T, vkDestroyDevice>>;

using VkImagePtr = std::unique_ptr<VkImage_T, 
    VulkanDeleter<VkImage_T, vkDestroyImage, VkDevice>>;

using VkSurfaceKHRPtr = std::unique_ptr<VkSurfaceKHR_T, 
    VulkanDeleter<VkSurfaceKHR_T, vkDestroySurfaceKHR, VkInstance>>;

using VkSwapchainKHRPtr = std::unique_ptr<VkSwapchainKHR_T, 
    VulkanDeleter<VkSwapchainKHR_T, vkDestroySwapchainKHR, VkDevice>>;

using VkRenderPassPtr = std::unique_ptr<VkRenderPass_T, 
    VulkanDeleter<VkRenderPass_T, vkDestroyRenderPass, VkDevice>>;

using VkPipelineLayoutPtr = std::unique_ptr<VkPipelineLayout_T, 
    VulkanDeleter<VkPipelineLayout_T, vkDestroyPipelineLayout, VkDevice>>;

using VkPipelinePtr = std::unique_ptr<VkPipeline_T, 
    VulkanDeleter<VkPipeline_T, vkDestroyPipeline, VkDevice>>;

using VkCommandPoolPtr = std::unique_ptr<VkCommandPool_T, 
    VulkanDeleter<VkCommandPool_T, vkDestroyCommandPool, VkDevice>>;

using VkSemaphorePtr = std::unique_ptr<VkSemaphore_T, 
    VulkanDeleter<VkSemaphore_T, vkDestroySemaphore, VkDevice>>;

using VkFencePtr = std::unique_ptr<VkFence_T, 
    VulkanDeleter<VkFence_T, vkDestroyFence, VkDevice>>;

using VkImageViewPtr = std::unique_ptr<VkImageView_T, 
    VulkanDeleter<VkImageView_T, vkDestroyImageView, VkDevice>>;

using VkSamplerPtr =  std::unique_ptr<VkSampler_T,
    VulkanDeleter<VkSampler_T, vkDestroySampler, VkDevice>>;
    
using VkFramebufferPtr = std::unique_ptr<VkFramebuffer_T, 
    VulkanDeleter<VkFramebuffer_T, vkDestroyFramebuffer, VkDevice>>;

using VkShaderModulePtr = std::unique_ptr<VkShaderModule_T,
 VulkanDeleter<VkShaderModule_T, vkDestroyShaderModule, VkDevice>>;


/**
 * Allocator (VkAllocationCallbacks) в Vulkan — это механизм управления памятью,
 *  позволяющий приложению контролировать выделение/освобождение памяти,
 *  используемой Vulkan. В вашем коде nullptr( DeleteFunc(params..., obj, nullptr); ) в качестве allocator
 *  означает использование стандартного аллокатора Vulkan.
*/ 