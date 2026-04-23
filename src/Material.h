#pragma once
#include "Texture.h"
#include "DescriptorManager.h"

//need to add more logic to this later, making different pipelines and shit for different types of materials...
class Material
{
public:

	//the material needs to be a container for a shader, like having a specific shader for a material instance
	// we will do that later...

	//Material(const Material&) = delete;
	//Material& operator=(const Material&) = delete;


	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;


	std::unique_ptr<Texture> m_albedo = nullptr;

	std::unique_ptr<Texture> m_normal = nullptr;


	void CreateDescriptorSet(
		VkDevice device,
		DescriptorManager& descriptorManager,
		VkDescriptorSetLayout layout);

	void Bind(VkCommandBuffer cmd, VkPipelineLayout layout) const;

	void Destroy();
};