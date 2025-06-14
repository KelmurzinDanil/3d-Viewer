#include "TextureManager.hpp"
#include <stb_image.h>
TextureManager::TextureManager(BufferManager& bufferManager, DeviceManager& deviceManager, SwapChainManager& swapChainManager):
bufferManager_(bufferManager), deviceManager_(deviceManager),swapChainManager_(swapChainManager),
textureImageView(nullptr, VulkanDeleter<VkImageView_T, vkDestroyImageView, VkDevice>(nullptr)),
textureSampler(nullptr, VulkanDeleter<VkSampler_T, vkDestroySampler, VkDevice>(nullptr)),
textureImageMemory(nullptr, VulkanDeleter<VkDeviceMemory_T, vkFreeMemory, VkDevice>(nullptr)),
textureImage(nullptr, VulkanDeleter<VkImage_T, vkDestroyImage, VkDevice>(nullptr))

{
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
}


void TextureManager::createTextureImage(){

    namespace fs = std::filesystem;
    

    int texWidth, texHeight, texChannels;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    stbi_set_flip_vertically_on_load(false);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!"); 
    }  
    

    bufferManager_.createBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // TRANSFER_SRC означает, что буфер будет источником данных для операций копирования
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT // HOST_VISIBLE — память доступна для записи/чтения с CPU (хост-устройства).
        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // HOST_COHERENT — гарантирует автоматическую синхронизацию кэшей CPU и GPU.                                                                              
        stagingBuffer,
        stagingBufferMemory
    );

   
    void* data;
    //Заполняет data, используя staging-буфер
    vkMapMemory(deviceManager_.device(), stagingBufferMemory, 0, imageSize, 0, &data);
    //Копирует данные pixels в data
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    //После этой штуки data недействителен
    vkUnmapMemory(deviceManager_.device(), stagingBufferMemory);

    stbi_image_free(pixels);
    swapChainManager_.createImage(texWidth,
                texHeight,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                textureImage, textureImageMemory);

    bufferManager_.transitionImageLayout(textureImage.get(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    bufferManager_.copyBufferToImage(stagingBuffer, textureImage.get(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    bufferManager_.transitionImageLayout(textureImage.get(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(deviceManager_.device(), stagingBuffer, nullptr);
    vkFreeMemory(deviceManager_.device(), stagingBufferMemory, nullptr);
}

void TextureManager::createTextureImageView() {
    textureImageView = swapChainManager_.createImageView(textureImage.get(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}
void TextureManager::createTextureSampler() {


    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(deviceManager_.physicalDevice(), &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler rawTextureSampler;
    if (vkCreateSampler(deviceManager_.device(), &samplerInfo, nullptr, &rawTextureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    textureSampler = VkSamplerPtr(rawTextureSampler, VulkanDeleter<VkSampler_T, vkDestroySampler, VkDevice>(deviceManager_.device()));
}
