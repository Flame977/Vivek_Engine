#include "Texture.h"

Texture::Texture(VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkCommandPool commandPool,
	VkQueue graphicsQueue,
	const std::string& path) :

	m_device(device),
	m_physicalDevice(physicalDevice),
	m_commandPool(commandPool),
	m_graphicsQueue(graphicsQueue)
{
	CreateTextureImage(path);
	CreateImageView();
	CreateSampler();
}

Texture::~Texture()
{
	if (m_sampler != VK_NULL_HANDLE)
		vkDestroySampler(m_device, m_sampler, nullptr);

	if (m_imageView != VK_NULL_HANDLE)
		vkDestroyImageView(m_device, m_imageView, nullptr);

	if (m_image != VK_NULL_HANDLE)
		vkDestroyImage(m_device, m_image, nullptr);

	if (m_imageMemory != VK_NULL_HANDLE)
		vkFreeMemory(m_device, m_imageMemory, nullptr);
}

void Texture::CreateTextureImage(const std::string& path)
{
	int texWidth, texHeight, texChannels;

	//to fix the flipped image...
	stbi_set_flip_vertically_on_load(true);

	stbi_uc* pixels = stbi_load(
		path.c_str(),
		&texWidth,
		&texHeight,
		&texChannels,
		STBI_rgb_alpha);

	if (!pixels)
		throw std::runtime_error("Failed to load texture image");

	VkDeviceSize imageSize =
		static_cast<VkDeviceSize>(texWidth) *
		static_cast<VkDeviceSize>(texHeight) * 4;

	m_width = texWidth;
	m_height = texHeight;

	// --- 1. Create staging buffer ---
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VulkanUtils::CreateBuffer(
		m_device,
		m_physicalDevice,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	// --- 2. Copy pixel data into staging buffer ---
	void* data;
	vkMapMemory(
		m_device,
		stagingBufferMemory,
		0,
		imageSize,
		0,
		&data);

	memcpy(data, pixels, static_cast<size_t>(imageSize));

	vkUnmapMemory(m_device, stagingBufferMemory);

	stbi_image_free(pixels);

	// --- 3. Create GPU image ---
	VulkanUtils::CreateImage(
		m_device,
		m_physicalDevice,
		m_width,
		m_height,
		1,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_image,
		m_imageMemory,
		0
	);

	// --- 4. Transition image to transfer destination ---
	VulkanUtils::TransitionImageLayout(
		m_device,
		m_commandPool,
		m_graphicsQueue,
		m_image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1);

	// --- 5. Copy buffer to image ---
	auto region2d = VulkanUtils::Create2DRegion(m_width, m_height);
	VulkanUtils::CopyBufferToImage(
		m_device,
		m_commandPool,
		m_graphicsQueue,
		stagingBuffer,
		m_image,
		region2d
	);

	// --- 6. Transition image to shader readable ---
	VulkanUtils::TransitionImageLayout(
		m_device,
		m_commandPool,
		m_graphicsQueue,
		m_image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		1);

	// --- 7. Cleanup staging buffer ---
	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);

}

void Texture::CreateImageView()
{
	m_imageView = VulkanUtils::CreateImageView(
		m_device,
		m_image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT,
		1,
		VK_IMAGE_VIEW_TYPE_2D
	);
}

void Texture::CreateSampler()
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(
		m_physicalDevice,
		&properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType =
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	samplerInfo.magFilter =
		VK_FILTER_LINEAR;
	samplerInfo.minFilter =
		VK_FILTER_LINEAR;

	samplerInfo.addressModeU =
		VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV =
		VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW =
		VK_SAMPLER_ADDRESS_MODE_REPEAT;

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy =
		properties.limits.maxSamplerAnisotropy;

	samplerInfo.borderColor =
		VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	samplerInfo.unnormalizedCoordinates =
		VK_FALSE;

	samplerInfo.compareEnable =
		VK_FALSE;
	samplerInfo.compareOp =
		VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode =
		VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f; // no mipmaps

	if (vkCreateSampler(
		m_device,
		&samplerInfo,
		nullptr,
		&m_sampler) != VK_SUCCESS)
	{
		throw std::runtime_error(
			"Failed to create texture sampler");
	}
}
