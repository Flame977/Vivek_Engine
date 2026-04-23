#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <array>
#include "VulkanTypes.h"
#include "VulkanUtils.h"


class DescriptorManager
{
public:

	void CreateDescriptorPool(VkDevice device, uint32_t maxFrames, uint32_t maxObjects);

	void ShutDown();

	VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout layout) const;

	void UpdateBuffer(VkDescriptorSet set, uint32_t binding, VkBuffer buffer, VkDeviceSize size) const;

	void UpdateBuffer(VkDescriptorSet set, uint32_t binding, VkBuffer buffer, VkDeviceSize size, VkImageView imageView, VkSampler sampler) const;

	void UpdateImage(VkDescriptorSet set, uint32_t binding, VkImageView imageView, VkSampler sampler);

	void UpdateSkybox(VkDescriptorSet set, VkImageView imageView, VkSampler sampler) const;

	/*
	void UpdateFrameDescriptor(VkDescriptorSet set, VkBuffer buffer, VkDeviceSize size) const;

	void UpdateMaterialDescriptor(VkDescriptorSet set, VkBuffer materialUBO, VkDeviceSize uboSize,VkImageView imageView, VkSampler sampler) const;
	*/

private:

	VkDevice m_device = VK_NULL_HANDLE;

	VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;


};