#pragma once

#include <string>
#include <array>
#include <vulkan/vulkan.h>

struct VulkanSkybox
{
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexMemory = VK_NULL_HANDLE;

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;

	uint32_t vertexCount = 0;


	// destroy later...
	void Destroy(VkDevice device)
	{
		// Pipeline
		if (pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
			pipeline = VK_NULL_HANDLE;
		}

		if (pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			pipelineLayout = VK_NULL_HANDLE;
		}

		// Descriptor set layout
		if (descriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
			descriptorSetLayout = VK_NULL_HANDLE;
		}

		// Image view
		if (imageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device, imageView, nullptr);
			imageView = VK_NULL_HANDLE;
		}

		// Sampler
		if (sampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(device, sampler, nullptr);
			sampler = VK_NULL_HANDLE;
		}

		// Image
		if (image != VK_NULL_HANDLE)
		{
			vkDestroyImage(device, image, nullptr);
			image = VK_NULL_HANDLE;
		}

		if (memory != VK_NULL_HANDLE)
		{
			vkFreeMemory(device, memory, nullptr);
			memory = VK_NULL_HANDLE;
		}

		// Vertex buffer
		if (vertexBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(device, vertexBuffer, nullptr);
			vertexBuffer = VK_NULL_HANDLE;
		}

		if (vertexMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(device, vertexMemory, nullptr);
			vertexMemory = VK_NULL_HANDLE;
		}

		// descriptorSet => DO NOTHING (handled by pool)
	}


};

struct SkyboxFaces
{
	std::string right;
	std::string left;
	std::string top;
	std::string down;
	std::string front;
	std::string back;
};


class Skybox
{
public:

	Skybox(const SkyboxFaces& f);

	SkyboxFaces faces;

};
