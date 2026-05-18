#include "Material.h"


void Material::CreateDescriptorSet(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	DescriptorManager& descriptorManager,
	VkDescriptorSetLayout layout)
{
	m_device = device;

	VulkanUtils::CreateBuffer(
		device,
		physicalDevice,
		sizeof(MaterialData),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_materialBuffer,
		m_materialMemory
		);

	descriptorSet = descriptorManager.AllocateDescriptorSet(layout);

	descriptorManager.UpdateImage(
		descriptorSet,
		0, // binding 0
		m_albedo->GetImageView(),
		m_albedo->GetSampler()
	);


	// Binding 1 — material UBO
	descriptorManager.UpdateBuffer(
		descriptorSet,
		1,
		m_materialBuffer,
		sizeof(MaterialData)
	);

}


void Material::UpdateMaterialData(MaterialData data)
{
	materialData = data;
	void* mapped;
	vkMapMemory(m_device, m_materialMemory, 0, sizeof(MaterialData), 0, &mapped);
	memcpy(mapped, &materialData, sizeof(MaterialData));
	vkUnmapMemory(m_device, m_materialMemory);
}

void Material::Bind(VkCommandBuffer cmd, VkPipelineLayout layout) const
{
	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		layout,
		1, // SET 1 (material set)
		1,
		&descriptorSet,
		0,
		nullptr
	);
}

void Material::Destroy()
{
	m_albedo.reset();

	vkDestroyBuffer(m_device, m_materialBuffer, nullptr);
	vkFreeMemory(m_device, m_materialMemory, nullptr);
}