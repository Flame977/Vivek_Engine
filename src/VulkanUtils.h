#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "VulkanTypes.h"
#include "GLFW/glfw3.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <iostream>


namespace VulkanUtils
{

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	int RatePhysicalDevice(VkPhysicalDevice device);

	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

	VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes);

	VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

	std::vector<char> ReadFile(const std::string& filename);

	VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t layerCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags flags);

	VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount, VkImageViewType viewType);

	void CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

	void TransitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount);

	std::vector<VkBufferImageCopy> Create2DRegion(uint32_t width, uint32_t height);

	void CopyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, const std::vector<VkBufferImageCopy>& regions);

	VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

	void EndSingleTimeCommands(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);

	void UploadData(VkDevice device, VkDeviceMemory memory, const void* srcData, VkDeviceSize size, VkDeviceSize offset = 0);


	void CreateDeviceBuffer(
		VkDevice device,
		VkCommandPool commandPool,
		VkQueue graphicsQueue,
		VkPhysicalDevice physicalDevice,
		const void* srcData,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkBuffer& buffer,
		VkDeviceMemory& memory);

	void CopyBuffer(
		VkDevice device,
		VkCommandPool commandPool,
		VkQueue graphicsQueue,
		VkBuffer src,
		VkBuffer dst,
		VkDeviceSize size);

};
