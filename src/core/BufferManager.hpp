#pragma once 
#include "DeviceManager.hpp"
#include "SwapChainManager.hpp"
#include "Vertex.hpp"
#include <vector>




class BufferManager {
    public:
        BufferManager(DeviceManager& deviceManager, VkCommandPool commandPool, SwapChainManager& swapChainManager);
        static const std::vector<uint16_t> indices;

        VkBuffer getIndexBuffer() const {return indexBuffer.get();}
        VkBuffer getVertexBuffer() const {return vertexBuffer.get();}
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
        
        /**
        * @brief Создает вершинный буфер и выделяет память
        */
        void createVertexBuffer();
        void createIndexBuffer();


        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    };