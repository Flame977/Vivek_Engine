#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include "VulkanUtils.h"
#include "stb_image.h"

class Texture
{
public:
    Texture(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        const std::string& path);

    ~Texture();

    VkImageView GetImageView() const { return m_imageView; }
    VkSampler  GetSampler()   const { return m_sampler; }


private:
	void CreateTextureImage(const std::string& path);
	void CreateImageView();
	void CreateSampler();


    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;

    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;

    uint32_t m_width = 0;
    uint32_t m_height = 0;

};