#pragma once
#include "vulkan/vulkan.h"
#include "VulkanContext.h"

class Scene;

class ShadowPass
{
public:

	void Initialize(VulkanContext& vulkan);

	VkImageView GetShadowImageView() const;

	VkSampler GetShadowSampler() const;

	void Render(
		VkCommandBuffer cmd,
		const Scene& scene);

	void Cleanup(VkDevice device);

	glm::mat4 GetLightSpace() const;
	void SetLightSpace(const glm::mat4& matrix);
	glm::mat4 GetLightView() const;
	glm::mat4 GetLightProjection() const;

private:

	void CreateShadowMap();
	void CreateRenderPass();
	void CreateFramebuffer();
	void CreatePipeline();
	void CreateDescriptors();

private:

	VulkanContext* m_vulkan = nullptr;

	static constexpr uint32_t SHADOW_SIZE = 4096;

	// shadow texture
	VkImage m_shadowImage = VK_NULL_HANDLE;
	VkDeviceMemory m_shadowMemory = VK_NULL_HANDLE;
	VkImageView m_shadowImageView = VK_NULL_HANDLE;

	// debug / future sampling
	VkSampler m_shadowSampler = VK_NULL_HANDLE;

	// render pass
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

	// pipeline
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;

	// uniforms
	VkBuffer m_uniformBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_uniformMemory = VK_NULL_HANDLE;

	VkDescriptorSetLayout m_descriptorLayout = VK_NULL_HANDLE;

	VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

	glm::mat4 m_lightSpace = glm::mat4(1.0f);
	glm::mat4 m_lightView = glm::mat4(1.0f);
	glm::mat4 m_lightProj = glm::mat4(1.0f);


};


