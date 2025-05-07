#pragma once 
#include "DeviceManager.hpp"
#include "SwapChainManager.hpp"
#include "Vertex.hpp"
#include "Constants.hpp"
#include <vector>
#include <vulkan/vulkan.h>




class BufferManager {
    public:
        BufferManager(DeviceManager& deviceManager, VkCommandPool commandPool, SwapChainManager& swapChainManager);

        VkBuffer getIndexBuffer() const {return indexBuffer.get();}
        VkBuffer getVertexBuffer() const {return vertexBuffer.get();}

        const std::vector<void*>& getUniformBuffersMapped() const;
        const std::vector<VkBufferPtr>& getUniformBuffers() const;
    private:
        DeviceManager& deviceManager_;
        VkCommandPool commandPool_;
        SwapChainManager& swapChainManager_;

        VkBuffer rawVertexBuffer;
        VkDeviceMemory rawVertexBufferMemory;
        
        VkBufferPtr indexBuffer;
        VkDeviceMemoryPtr indexBufferMemory;

        VkDeviceMemoryPtr vertexBufferMemory;
        VkBufferPtr vertexBuffer;  ///< Вершинный буфер
        
        
        std::vector<VkBufferPtr> uniformBuffers;
        std::vector<VkDeviceMemoryPtr> uniformBuffersMemory;
        std::vector<void*> uniformBuffersMapped;

        /**
        * @brief Создает вершинный буфер и выделяет память
        */
        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffers();

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    };