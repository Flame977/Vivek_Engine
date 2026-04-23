#include "VulkanUtils.h"


QueueFamilyIndices VulkanUtils::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(
		device,
		&queueFamilyCount,
		nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
		device,
		&queueFamilyCount,
		queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			!indices.graphicsFamily.has_value())
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			device,
			i,
			surface,
			&presentSupport);

		if (presentSupport && !indices.presentFamily.has_value())
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
			break;
	}

	return indices;
}


bool VulkanUtils::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(
		device,
		nullptr,
		&extensionCount,
		nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
		device,
		nullptr,
		&extensionCount,
		extensions.data());

	const std::vector<const char*> requiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	for (const char* required : requiredExtensions)
	{
		bool found = false;
		for (const auto& ext : extensions)
		{
			if (strcmp(required, ext.extensionName) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
			return false;
	}

	return true;
}


int VulkanUtils::RatePhysicalDevice(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties props;
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceProperties(device, &props);
	vkGetPhysicalDeviceFeatures(device, &features);

	int score = 0;

	// Prefer discrete GPUs
	if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;

	// Larger max texture size = better
	score += props.limits.maxImageDimension2D;

	// Optional features
	if (features.samplerAnisotropy)
		score += 100;

	return score;
}

SwapchainSupportDetails VulkanUtils::QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapchainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		device,
		surface,
		&details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		device,
		surface,
		&formatCount,
		nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device,
			surface,
			&formatCount,
			details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device,
		surface,
		&presentModeCount,
		nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device,
			surface,
			&presentModeCount,
			details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR VulkanUtils::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	for (const auto& format : formats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}
	return formats[0];
}

VkPresentModeKHR VulkanUtils::ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes)
{
	for (const auto& mode : modes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode; // best (triple buffering)
	}
	return VK_PRESENT_MODE_FIFO_KHR; // guaranteed
}

VkExtent2D VulkanUtils::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	VkExtent2D actualExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};

	actualExtent.width = std::clamp(
		actualExtent.width,
		capabilities.minImageExtent.width,
		capabilities.maxImageExtent.width);

	actualExtent.height = std::clamp(
		actualExtent.height,
		capabilities.minImageExtent.height,
		capabilities.maxImageExtent.height);

	return actualExtent;
}

std::vector<char> VulkanUtils::ReadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + filename);
	}

	// Get file size
	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	// Read file
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

VkShaderModule VulkanUtils::CreateShaderModule(VkDevice device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module");
	}
	//std::cout << "Successfully created Shader Module::" << std::endl;

	return shaderModule;
}

uint32_t VulkanUtils::FindMemoryType(
	VkPhysicalDevice physicalDevice,
	uint32_t typeFilter,
	VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(
		physicalDevice,
		&memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type");
}


void VulkanUtils::CreateBuffer(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory)
{
	// Create buffer handle
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(
		device,
		&bufferInfo,
		nullptr,
		&buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer");
	}

	// Query memory requirements
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(
		device,
		buffer,
		&memRequirements);

	// Allocate memory
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex =
		FindMemoryType(
			physicalDevice,
			memRequirements.memoryTypeBits,
			properties);

	if (vkAllocateMemory(
		device,
		&allocInfo,
		nullptr,
		&bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory");
	}

	// Bind buffer to memory
	vkBindBufferMemory(
		device,
		buffer,
		bufferMemory,
		0);
}



void VulkanUtils::CreateImage(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	uint32_t width,
	uint32_t height,
	uint32_t layerCount,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkImage& image,
	VkDeviceMemory& imageMemory,
	VkImageCreateFlags flags)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = layerCount;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = flags;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("Failed to create image");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex =
		VulkanUtils::FindMemoryType(
			physicalDevice,
			memRequirements.memoryTypeBits,
			properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate image memory");

	vkBindImageMemory(device, image, imageMemory, 0);
}


VkImageView VulkanUtils::CreateImageView(
	VkDevice device,
	VkImage image,
	VkFormat format,
	VkImageAspectFlags aspectFlags,
	uint32_t layerCount,
	VkImageViewType viewType

)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = viewType;
	viewInfo.format = format;

	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = layerCount;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		throw std::runtime_error("Failed to create image view");

	return imageView;
}






VkFormat VulkanUtils::FindSupportedFormat(
	VkPhysicalDevice physicalDevice,
	const std::vector<VkFormat>& candidates,
	VkImageTiling tiling,
	VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(
			physicalDevice,
			format,
			&props);

		if (tiling == VK_IMAGE_TILING_LINEAR &&
			(props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
			(props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported format");
}


VkFormat VulkanUtils::FindDepthFormat(VkPhysicalDevice physicalDevice)
{
	return FindSupportedFormat(
		physicalDevice,
		{   //candidates
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}


//void VulkanContext::CreateDepthResources()
//{
//	m_depthFormat = VulkanUtils::FindDepthFormat();
//
//	VulkanUtils::CreateImage(
//		m_device,
//		m_physicalDevice,
//		m_swapchainExtent.width,
//		m_swapchainExtent.height,
//		m_depthFormat,
//		VK_IMAGE_TILING_OPTIMAL,
//		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//		m_depthImage,
//		m_depthImageMemory);
//
//	m_depthImageView = VulkanUtils::CreateImageView(
//		m_device,
//		m_depthImage,
//		m_depthFormat,
//		VK_IMAGE_ASPECT_DEPTH_BIT);
//}


void VulkanUtils::TransitionImageLayout(
	VkDevice device,
	VkCommandPool commandPool,
	VkQueue graphicsQueue,
	VkImage image,
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t layerCount)
{
	VkCommandBuffer commandBuffer =
		BeginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType =
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex =
		VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex =
		VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	barrier.subresourceRange.aspectMask =
		VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask =
			VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage =
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage =
			VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask =
			VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask =
			VK_ACCESS_SHADER_READ_BIT;

		sourceStage =
			VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage =
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::invalid_argument(
			"Unsupported image layout transition");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage,
		destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	EndSingleTimeCommands(
		device,
		graphicsQueue,
		commandPool,
		commandBuffer);
}


//only for 2d textures
std::vector<VkBufferImageCopy> VulkanUtils::Create2DRegion(
	uint32_t width,
	uint32_t height)
{
	VkBufferImageCopy region{};

	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	return { region };
}

void VulkanUtils::CopyBufferToImage(
	VkDevice device,
	VkCommandPool commandPool,
	VkQueue graphicsQueue,
	VkBuffer buffer,
	VkImage image,
	const std::vector<VkBufferImageCopy>& regions)
{
	VkCommandBuffer commandBuffer =
		BeginSingleTimeCommands(device, commandPool);

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(regions.size()),
		regions.data());

	EndSingleTimeCommands(
		device,
		graphicsQueue,
		commandPool,
		commandBuffer);
}



//VkCommandBuffer VulkanUtils::BeginSingleTimeCommands(
//	VkDevice device,
//	VkCommandPool commandPool)
//{
//	VkCommandBufferAllocateInfo allocInfo{};
//	allocInfo.sType =
//		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	allocInfo.level =
//		VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//	allocInfo.commandPool = commandPool;
//	allocInfo.commandBufferCount = 1;
//
//	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
//
//	if (vkAllocateCommandBuffers(
//		device,
//		&allocInfo,
//		&commandBuffer) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to allocate command buffer");
//	}
//
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType =
//		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags =
//		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//	vkBeginCommandBuffer(
//		commandBuffer,
//		&beginInfo);
//
//	return commandBuffer;
//}

//new version
VkCommandBuffer VulkanUtils::BeginSingleTimeCommands(
	VkDevice device,
	VkCommandPool commandPool)
{
	assert(device != VK_NULL_HANDLE);
	assert(commandPool != VK_NULL_HANDLE);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VkResult res = vkAllocateCommandBuffers(
		device,
		&allocInfo,
		&commandBuffer);

	if (res != VK_SUCCESS)
	{
		throw std::runtime_error(
			"vkAllocateCommandBuffers failed in BeginSingleTimeCommands");
	}

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error(
			"vkBeginCommandBuffer failed in BeginSingleTimeCommands");
	}

	return commandBuffer;
}



void VulkanUtils::EndSingleTimeCommands(
	VkDevice device,
	VkQueue graphicsQueue,
	VkCommandPool commandPool,
	VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType =
		VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(
		graphicsQueue,
		1,
		&submitInfo,
		VK_NULL_HANDLE);

	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(
		device,
		commandPool,
		1,
		&commandBuffer);
}

// Uploads the data to the vk device ig...
void VulkanUtils::UploadData(
	VkDevice device,
	VkDeviceMemory memory,
	const void* srcData,
	VkDeviceSize size,
	VkDeviceSize offset)
{
	void* dstData = nullptr;

	if (vkMapMemory(device, memory, offset, size, 0, &dstData) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map memory!");
	}

	memcpy(dstData, srcData, static_cast<size_t>(size));

	vkUnmapMemory(device, memory);
}



void VulkanUtils::CreateDeviceBuffer(
	VkDevice device,
	VkCommandPool commandPool,
	VkQueue graphicsQueue,
	VkPhysicalDevice physicalDevice,
	const void* srcData,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkBuffer& buffer,
	VkDeviceMemory& memory)
{
	//Staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VulkanUtils::CreateBuffer(
		device,
		physicalDevice,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingMemory
	);

	void* mapped;
	vkMapMemory(device, stagingMemory, 0, size, 0, &mapped);
	memcpy(mapped, srcData, static_cast<size_t>(size));
	vkUnmapMemory(device, stagingMemory);

	//GPU buffer
	VulkanUtils::CreateBuffer(
		device,
		physicalDevice,
		size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		buffer,
		memory
	);

	VulkanUtils::CopyBuffer(device,commandPool,graphicsQueue,stagingBuffer, buffer, size);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingMemory, nullptr);
}



void VulkanUtils::CopyBuffer(
	VkDevice device,
	VkCommandPool commandPool,
	VkQueue graphicsQueue,
	VkBuffer src,
	VkBuffer dst,
	VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cmd;
	vkAllocateCommandBuffers(
		device,
		&allocInfo,
		&cmd);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags =
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd, &beginInfo);

	VkBufferCopy copy{};
	copy.size = size;
	vkCmdCopyBuffer(cmd, src, dst, 1, &copy);

	vkEndCommandBuffer(cmd);

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;

	vkQueueSubmit(graphicsQueue, 1, &submit, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &cmd);
}



















