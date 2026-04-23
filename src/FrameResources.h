#pragma once
#include <vulkan/vulkan.h>

struct FrameResources
{
	VkBuffer uniformBuffer = VK_NULL_HANDLE;
	VkDeviceMemory uniformMemory = VK_NULL_HANDLE;

	VkDescriptorSet globalDescriptorSet = VK_NULL_HANDLE;

	VkFence inFlightFence = VK_NULL_HANDLE;
	VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
};
