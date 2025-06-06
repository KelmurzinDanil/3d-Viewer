#pragma once
#include <array>
#include <vector>
#include <algorithm>
#include <limits>
#include "DeviceManager.hpp"
#include "VulkanTypes.hpp"
#include "SurfaceManager.hpp"
#include "WindowManager.hpp"



class SwapChainManager{
    friend class TextureManager;
    public:
        SwapChainManager(const SwapChainManager&) = delete;
        SwapChainManager& operator=(const SwapChainManager&) = delete;

        SwapChainManager(DeviceManager& deviceManager, SurfaceManager& surfaceManager, WindowManager& windowManager);

        std::vector<VkFramebufferPtr> swapChainFramebuffers;
        //const std::vector<VkFramebuffer>& getFramebuffers() const { return swapChainFramebuffers; }
        VkExtent2D getExtent() const { return swapChainExtent; }
        VkQueue getGraphicsQueue() const {
            return graphicsQueue;
        }
        
        VkQueue getPresentQueue() const {
            return presentQueue;
        }
        void setDepthImageView(VkImageViewPtr depthImgView) { depthImageView = std::move(depthImgView); }
        VkImageView getDepthImageView() const {return depthImageView.get(); }

        VkExtent2D getSwapChainExtent() const { return swapChainExtent;}

        VkSwapchainKHR getSwapChain() const { return swapChain.get();}
        VkRenderPass getRenderPass() const { return renderPass.get();}
        VkImageViewPtr createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

        void createImage(uint32_t width,
                    uint32_t height,
                    VkFormat format,
                    VkImageTiling tiling,
                    VkImageUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkImagePtr& image,
                    VkDeviceMemoryPtr& imageMemory);
                    
    private:
        DeviceManager& deviceManager;
        SurfaceManager& surfaceManager;
        WindowManager& windowManager;
        
        VkSwapchainKHRPtr swapChain;    
        VkRenderPassPtr renderPass;

        VkQueue graphicsQueue;                 
        VkQueue presentQueue;                  

        std::vector<VkImage> swapChainImages;    
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageViewPtr> swapChainImageViews;

        VkImageViewPtr depthImageView;
        VkImagePtr depthImage;
        VkDeviceMemoryPtr depthImageMemory;

        void createDepthResources();
        void createSwapChain();
        void recreateSwapChain();
        void createImageViews();
        void createFramebuffers();
        void createRenderPass();
        

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

};