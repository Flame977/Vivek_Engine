#include "DescriptorManager.h"


void DescriptorManager::CreateDescriptorPool(VkDevice device, uint32_t maxFrames, uint32_t maxObjects)
{
	m_device = device;

	//uint32_t maxObjects = 100; // or RenderObjects.size()
	uint32_t skyboxCount = 1;


	//need to do this for both ubo and textures...
	std::array<VkDescriptorPoolSize, 2> poolSizes{};

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = maxFrames;

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = maxObjects + skyboxCount;

	// OG code
	/*VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = maxSets;*/


	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = maxFrames + maxObjects + skyboxCount;



	if (vkCreateDescriptorPool(
		m_device,
		&poolInfo,
		nullptr,
		&m_descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error(
			"Failed to create descriptor pool");
	}
}

void DescriptorManager::ShutDown()
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
		m_descriptorPool = VK_NULL_HANDLE;
	}

	m_device = VK_NULL_HANDLE;
}

VkDescriptorSetLayout DescriptorManager::CreateDescriptorSetLayout(
	const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount =
		static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout layout;
	if (vkCreateDescriptorSetLayout(
		m_device,
		&layoutInfo,
		nullptr,
		&layout) != VK_SUCCESS)
	{
		throw std::runtime_error(
			"Failed to create descriptor set layout");
	}

	return layout;
}

VkDescriptorSet DescriptorManager::AllocateDescriptorSet(VkDescriptorSetLayout layout) const
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	VkDescriptorSet set;
	if (vkAllocateDescriptorSets(
		m_device,
		&allocInfo,
		&set) != VK_SUCCESS)
	{
		throw std::runtime_error(
			"Failed to allocate descriptor set");
	}

	return set;
}



void DescriptorManager::UpdateBuffer(
	VkDescriptorSet set,
	uint32_t binding,
	VkBuffer buffer,
	VkDeviceSize size) const
{

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = size;



	std::array<VkWriteDescriptorSet, 1> writes{};

	// UBO
	writes[0].sType =
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = set;
	writes[0].dstBinding = 0;
	writes[0].descriptorType =
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].descriptorCount = 1;
	writes[0].pBufferInfo = &bufferInfo;


	vkUpdateDescriptorSets(
		m_device,
		static_cast<uint32_t>(writes.size()),
		writes.data(),
		0,
		nullptr);

}



//this one will be divided into 2 later...
void DescriptorManager::UpdateBuffer(
	VkDescriptorSet set,
	uint32_t binding,
	VkBuffer buffer,
	VkDeviceSize size,
	VkImageView imageView,
	VkSampler sampler) const
{

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = size;



	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = imageView;
	imageInfo.sampler = sampler;

	std::array<VkWriteDescriptorSet, 2> writes{};

	// UBO
	writes[0].sType =
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = set;
	writes[0].dstBinding = 0;
	writes[0].descriptorType =
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].descriptorCount = 1;
	writes[0].pBufferInfo = &bufferInfo;

	// Texture
	writes[1].sType =
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].dstSet = set;
	writes[1].dstBinding = 1;
	writes[1].descriptorType =
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[1].descriptorCount = 1;
	writes[1].pImageInfo = &imageInfo;


	vkUpdateDescriptorSets(
		m_device,
		static_cast<uint32_t>(writes.size()),
		writes.data(),
		0,
		nullptr);

}

void DescriptorManager::UpdateImage(VkDescriptorSet set, uint32_t binding, VkImageView imageView, VkSampler sampler)
{
	 VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(
        m_device,   // make sure DescriptorManager stores device
        1,
        &write,
        0,
        nullptr
    );
}


void DescriptorManager::UpdateSkybox(
	VkDescriptorSet set,
	VkImageView imageView,
	VkSampler sampler) const
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = imageView;
	imageInfo.sampler = sampler;

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = set;
	write.dstBinding = 0; // matches shader
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.descriptorCount = 1;
	write.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(
		m_device,
		1,
		&write,
		0,
		nullptr
	);
}



/*



//only for Camera stuff
void DescriptorManager::UpdateFrameDescriptor(VkDescriptorSet set, VkBuffer buffer, VkDeviceSize size) const
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = size;

	VkWriteDescriptorSet write{};

	// UBO
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = set;
	write.dstBinding = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.descriptorCount = 1;
	write.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
}

//only for materials
void DescriptorManager::UpdateMaterialDescriptor(VkDescriptorSet set, VkBuffer materialUBO, VkDeviceSize uboSize, VkImageView imageView, VkSampler sampler) const
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = materialUBO;
	bufferInfo.offset = 0;
	bufferInfo.range = uboSize;

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = imageView;
	imageInfo.sampler = sampler;

	std::array<VkWriteDescriptorSet, 2> writes{};

	// Material UBO
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = set;
	writes[0].dstBinding = 0;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].descriptorCount = 1;
	writes[0].pBufferInfo = &bufferInfo;

	// Albedo texture
	writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].dstSet = set;
	writes[1].dstBinding = 1;
	writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[1].descriptorCount = 1;
	writes[1].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(
		m_device,
		static_cast<uint32_t>(writes.size()),
		writes.data(),
		0,
		nullptr);

}

*/






