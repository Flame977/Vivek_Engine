#pragma once
#include "vulkan/vulkan.h"
#include "VulkanContext.h"
#include "Constants.h"

using namespace Constants;


struct CascadeData
{
	glm::mat4 lightSpace;
	float splitDepth;
	float padding[3];
};


class Scene;

class ShadowPass
{
public:

	void Initialize(VulkanContext& vulkan);
	void Render(VkCommandBuffer cmd, const Scene& scene);
	void Cleanup(VkDevice device);

	// Called from your light update code every frame
	void UpdateCascades(
		const glm::mat4& cameraView,
		const glm::mat4& cameraProj,
		const glm::vec3& lightDir,
		float cameraNear,
		float cameraFar
	);

	VkImageView GetShadowImageView() const;
	VkSampler GetShadowSampler() const;

	// Expose cascade data for main pass UBO
	const std::array<CascadeData, SHADOW_CASCADE_COUNT>& GetCascades() const;


	static constexpr uint32_t SHADOW_SIZE = 4096;

private:

	void CreateShadowMap();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreatePipeline();
	void CreateDescriptors();

private:

	VulkanContext* m_vulkan = nullptr;


	// shadow texture
	VkImage m_shadowImage = VK_NULL_HANDLE;
	VkDeviceMemory m_shadowMemory = VK_NULL_HANDLE;
	VkImageView m_shadowImageView = VK_NULL_HANDLE;

	// debug / future sampling
	VkSampler m_shadowSampler = VK_NULL_HANDLE;

	// One image view and framebuffer per cascade
	std::array<VkImageView, SHADOW_CASCADE_COUNT> m_cascadeImageViews{};
	std::array<VkFramebuffer, SHADOW_CASCADE_COUNT> m_cascadeFramebuffers{};

	// render pass
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;


	// One UBO per cascade
	std::array<VkBuffer, SHADOW_CASCADE_COUNT> m_uniformBuffers{};
	std::array<VkDeviceMemory, SHADOW_CASCADE_COUNT> m_uniformMemories{};
	std::array<VkDescriptorSet, SHADOW_CASCADE_COUNT> m_descriptorSets{};

	VkDescriptorSetLayout m_descriptorLayout = VK_NULL_HANDLE;

	// Cascade data computed every frame
	std::array<CascadeData, SHADOW_CASCADE_COUNT> m_cascades{};



};


