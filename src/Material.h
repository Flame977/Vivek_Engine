#pragma once
#include "Texture.h"
#include "DescriptorManager.h"


struct MaterialData
{
	float shininess = 32.0f;
	float specularStrength = 0.5f;
	float padding[2] = { 0.0f, 0.0f };
};


class Material
{
public:

	//Material(const Material&) = delete;
	//Material& operator=(const Material&) = delete;


	void CreateDescriptorSet(VkDevice device, VkPhysicalDevice physicalDevice, DescriptorManager& descriptorManager, VkDescriptorSetLayout layout);

	void UpdateMaterialData(MaterialData data);

	void Bind(VkCommandBuffer cmd, VkPipelineLayout layout) const;

	void Destroy();


	MaterialData materialData;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	std::unique_ptr<Texture> m_albedo = nullptr;

	VkBuffer        m_materialBuffer = VK_NULL_HANDLE;
	VkDeviceMemory  m_materialMemory = VK_NULL_HANDLE;
	VkDevice        m_device = VK_NULL_HANDLE;
};

