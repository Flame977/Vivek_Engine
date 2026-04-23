#include "Material.h"


void Material::CreateDescriptorSet(
    VkDevice device,
    DescriptorManager& descriptorManager,
    VkDescriptorSetLayout layout)
{
    descriptorSet = descriptorManager.AllocateDescriptorSet(layout);

    descriptorManager.UpdateImage(
        descriptorSet,
        0, // binding 0
        m_albedo->GetImageView(),
        m_albedo->GetSampler()
    );
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
	m_normal.reset();
}