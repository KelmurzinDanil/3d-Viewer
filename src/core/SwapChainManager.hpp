#pragma once
#include <vector>
#include <algorithm>
#include <limits>
#include "DeviceManager.hpp"
#include "VulkanTypes.hpp"
#include "SurfaceManager.hpp"
#include "WindowManager.hpp"



class SwapChainManager{
    public:
        SwapChainManager(const SwapChainManager&) = delete;
        SwapChainManager& operator=(const SwapChainManager&) = delete;

        SwapChainManager(DeviceManager& deviceManager, SurfaceManager& surfaceManager, WindowManager& windowManager);

        std::vector<VkFramebufferPtr> swapChainFramebuffers;
        //const std::vector<VkFramebuffer>& getFramebuffers() const { return swapChainFramebuffers; }
        VkExtent2D getExtent() const { return swapChainExtent; }
        VkQueue getGraphicsQueue() const {
            if (!graphicsQueue) { //Потом удалить!!
                throw std::runtime_error("Graphics queue not initialized!");
            }
            return graphicsQueue;
        }
        
        VkQueue getPresentQueue() const {
            if (!presentQueue) { //Потом удалить!!
                throw std::runtime_error("Present queue not initialized!");
            }
            return presentQueue;
        }

        VkExtent2D getSwapChainExtent() const { return swapChainExtent;}

        VkSwapchainKHRPtr swapChain;    

        VkRenderPassPtr renderPass;

    private:
        DeviceManager& deviceManager;
        SurfaceManager& surfaceManager;
        WindowManager& windowManager;
        
        VkQueue graphicsQueue;                 
        VkQueue presentQueue;                  

        std::vector<VkImage> swapChainImages;    
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageViewPtr> swapChainImageViews;

        void createSwapChain();
        void recreateSwapChain();
        void createImageViews();
        void createFramebuffers();
        void createRenderPass();

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

};