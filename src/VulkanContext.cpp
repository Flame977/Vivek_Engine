#include "VulkanContext.h"


VulkanContext::VulkanContext(Window& window) :m_window(window)
{
	InitVulkan();
}

void VulkanContext::InitVulkan()
{
	// Core vulkan
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();

	PickPhysicalDevice();
	CreateLogicalDevice();

	// Swapchain stuff 
	CreateSwapchain();
	CreateSwapchainImageViews();
	CreateDepthResources();

	//Render Pass
	CreateRenderPass();

	//Descriptors stuff
	CreateDescriptorPool();

	//GPU resources creation
	CreateCommandPool();

	CreatePerFrameResources();  // UBOs...

	//Framebuffers and Rendering
	CreateFramebuffers();
	CreateCommandBuffers();
	CreateSyncObjects();


	// IMGUI...
	InitImGui();
}


void VulkanContext::InitImGui()
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	if (vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_imguiDescriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create ImGui descriptor pool");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags != ImGuiConfigFlags;

	ImGui_ImplGlfw_InitForVulkan(m_window.GetWindow(), true);

	ImGui_ImplVulkan_PipelineInfo pipeline_info{};
	pipeline_info.RenderPass = m_renderPass;


	ImGui_ImplVulkan_InitInfo init_info{};
	init_info.Instance = m_instance;
	init_info.PhysicalDevice = m_physicalDevice;
	init_info.Device = m_device;
	init_info.QueueFamily = m_graphicsQueueFamilyIndex;
	init_info.Queue = m_graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = m_imguiDescriptorPool;
	init_info.PipelineInfoMain = pipeline_info;

	init_info.MinImageCount = m_swapchainImages.size();
	init_info.ImageCount = m_swapchainImages.size();

	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;

	if (!ImGui_ImplVulkan_Init(&init_info))
	{
		throw std::runtime_error("Failed to initialize ImGui Vulkan backend");
	}
	else
		Log("Created IMGUI successfully !");
}

void VulkanContext::ShutdownImGui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	if (m_imguiDescriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(m_device, m_imguiDescriptorPool, nullptr);
		m_imguiDescriptorPool = VK_NULL_HANDLE;
	}
}


VulkanContext::~VulkanContext()
{
	ShutdownImGui();

	vkDeviceWaitIdle(m_device);


	CleanupFrameResources();

	m_descriptorManager.ShutDown();
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);


	CleanupPipelines();
	CleaupRenderPass();
	CleanupSwapchain();

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroyDevice(m_device, nullptr);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	DestroyDebugMessenger();
	vkDestroyInstance(m_instance, nullptr);
}


//NOT USING ANYMORE...
void VulkanContext::DrawFrame()
{
	uint32_t imageIndex;
	FrameResources* frame;

	// NEW FLOW
	BeginFrame(imageIndex, frame);

	// IMGUI (keep same for now)
	DrawImGuiUI();

	// COMMAND BUFFER
	VkCommandBuffer cmd = GetCommandBuffer(imageIndex);

	// RECORD (still using old function)
	RecordCommandBuffer(cmd, imageIndex, *frame);

	// SUBMIT + PRESENT
	EndFrame(imageIndex, *frame);
}



void VulkanContext::BeginFrame(uint32_t& imageIndex, FrameResources*& outFrame)
{
	FrameResources& frame = m_frames[m_currentFrame];

	vkWaitForFences(m_device, 1, &frame.inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(m_device, 1, &frame.inFlightFence);

	VkResult result = vkAcquireNextImageKHR(
		m_device,
		m_swapchain,
		UINT64_MAX,
		frame.imageAvailableSemaphore,
		VK_NULL_HANDLE,
		&imageIndex);

	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to acquire swapchain image");

	outFrame = &frame;

	vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);
}

VkCommandBuffer VulkanContext::GetCommandBuffer(uint32_t imageIndex)
{
	return m_commandBuffers[imageIndex];
}

void VulkanContext::EndFrame(uint32_t& imageIndex, FrameResources& frame)
{
	VkSemaphore waitSemaphores[] = { frame.imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { frame.renderFinishedSemaphore };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, frame.inFlightFence) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit draw command buffer");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.pImageIndices = &imageIndex;

	if (vkQueuePresentKHR(m_presentQueue, &presentInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to present swapchain image");
}

void VulkanContext::BeginRenderPass(VkCommandBuffer cmd, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(cmd, &beginInfo);

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchainExtent;

	VkClearValue clearValues[2];
	clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanContext::EndRenderPass(VkCommandBuffer cmd)
{
	vkCmdEndRenderPass(cmd);

	if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer");
}

void VulkanContext::WaitIdle() const
{
	if (m_device != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle(m_device);
	}
}

void VulkanContext::RecreateSwapchain()
{
	WaitIdle();

	CleaupRenderPass();
	CleanupSwapchain();

	CreateSwapchain();
	CreateSwapchainImageViews();
	CreateDepthResources();
	CreateRenderPass();
	//CreateGraphicsPipeline();
	//CreatePipelines();
	CreateFramebuffers();
	CreateCommandBuffers();

	std::cout << "Recreated Swapchain...!" << std::endl;
}

void VulkanContext::UpdateUniforms(CameraUBO& camera)
{
	UpdateUniformBuffer(m_currentFrame, camera);
}

VkDevice VulkanContext::GetDevice() const
{
	return m_device;
}

VkPhysicalDevice VulkanContext::GetPhysicalDevice() const
{
	return m_physicalDevice;
}

VkCommandPool VulkanContext::GetCommandPool() const
{
	return m_commandPool;
}

VkQueue VulkanContext::GetGraphicsQueue() const
{
	return m_graphicsQueue;
}

VkRenderPass VulkanContext::GetRenderPass() const
{
	return m_renderPass;
}

VkExtent2D VulkanContext::GetSwapchainExtent() const
{
	return m_swapchainExtent;
}

float VulkanContext::GetAspectRatio() const
{
	return static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.height);
}

void VulkanContext::SetDisplayFps(float fps)
{
	m_displayFPS = fps;
}

float VulkanContext::GetFps() const
{
	return m_displayFPS;
}

VkPipeline VulkanContext::GetGraphicsPipeline() const
{
	return m_graphicsPipeline;
}

VkPipelineLayout VulkanContext::GetGraphicsPipelineLayout() const
{
	return m_graphicsPipelineLayout;
}

FrameResources& VulkanContext::GetFrame(uint32_t index)
{
	return m_frames[index];
}

DescriptorManager& VulkanContext::GetDescriptorManager()
{
	return m_descriptorManager;
}

VkDescriptorSetLayout VulkanContext::CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	return m_descriptorManager.CreateDescriptorSetLayout(bindings);
}

VkDescriptorSet VulkanContext::AllocateDescriptorSet(VkDescriptorSetLayout layout)
{
	return m_descriptorManager.AllocateDescriptorSet(layout);
}



void VulkanContext::CreateDescriptorPool()
{
	m_descriptorManager.CreateDescriptorPool(m_device, MAX_FRAMES_IN_FLIGHT, 100);
}


void VulkanContext::CreatePerFrameResources()
{
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VulkanUtils::CreateBuffer(
			m_device,
			m_physicalDevice,
			sizeof(CameraUBO),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_frames[i].uniformBuffer,
			m_frames[i].uniformMemory
		);
	}
}




void VulkanContext::CreateInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Custom Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	//getting the glfw extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(
		glfwExtensions,
		glfwExtensions + glfwExtensionCount
	);

	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);


	//creating the createInfo
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	//glfw extensions enabling needed
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Enable validation layers
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance");

	std::cout << "Vulkan instance created successfully!" << std::endl;
}

void VulkanContext::SetupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType =
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = DebugCallback;

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(
			m_instance,
			"vkCreateDebugUtilsMessengerEXT");

	if (!func || func(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
		throw std::runtime_error("Failed to set up debug messenger");

	std::cout << "Debug messenger created successfully!" << std::endl;

}

void VulkanContext::CreateSurface()
{
	//glfw creates the window surface...
	if (glfwCreateWindowSurface(m_instance, m_window.GetWindow(), nullptr, &m_surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan surface");
	}

	std::cout << "Vulkan surface created successfully!" << std::endl;
}


void VulkanContext::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(
		m_instance,
		&deviceCount,
		nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("No Vulkan-compatible GPUs found");

	std::cout << "Device count is::" << deviceCount << std::endl;

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(
		m_instance,
		&deviceCount,
		devices.data());

	VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
	int bestScore = -1;

	for (const auto& device : devices)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);
		std::cout << "Found Device ::" << props.deviceName << std::endl;

		if (!IsDeviceSuitable(device))
			continue;

		int score = VulkanUtils::RatePhysicalDevice(device);

		if (score > bestScore)
		{
			bestScore = score;
			bestDevice = device;
		}
	}

	if (bestDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a suitable GPU");

	m_physicalDevice = bestDevice;

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(m_physicalDevice, &props);

	std::cout << "Best GPU Selected ::" << props.deviceName << std::endl;
}

bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device) const
{
	auto indices = VulkanUtils::FindQueueFamilies(device, m_surface);

	bool extensionsSupported = VulkanUtils::CheckDeviceExtensionSupport(device);

	return indices.IsComplete() && extensionsSupported;
}

void VulkanContext::CreateLogicalDevice()
{
	QueueFamilyIndices indices =
		VulkanUtils::FindQueueFamilies(
			m_physicalDevice,
			m_surface);

	//this is needed for IMGUI
	m_graphicsQueueFamilyIndex = indices.graphicsFamily.value();

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType =
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE; // optional but common

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount =
		static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount =
		static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (vkCreateDevice(
		m_physicalDevice,
		&createInfo,
		nullptr,
		&m_device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device");
	}

	// Retrieve queues
	vkGetDeviceQueue(
		m_device,
		indices.graphicsFamily.value(),
		0,
		&m_graphicsQueue);

	vkGetDeviceQueue(
		m_device,
		indices.presentFamily.value(),
		0,
		&m_presentQueue);

	std::cout << "Logical device created successfully!" << std::endl;

}

void VulkanContext::CreateSwapchain()
{
	auto support = VulkanUtils::QuerySwapchainSupport(m_physicalDevice, m_surface);

	VkSurfaceFormatKHR surfaceFormat = VulkanUtils::ChooseSurfaceFormat(support.formats);

	VkPresentModeKHR presentMode = VulkanUtils::ChoosePresentMode(support.presentModes);

	VkExtent2D extent = VulkanUtils::ChooseExtent(support.capabilities, m_window.GetWindow());

	uint32_t imageCount = support.capabilities.minImageCount + 1;

	if (support.capabilities.maxImageCount > 0 &&
		imageCount > support.capabilities.maxImageCount)
	{
		imageCount = support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType =
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//createInfo.pNext = nullptr;

	auto indices =
		VulkanUtils::FindQueueFamilies(
			m_physicalDevice,
			m_surface);

	uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode =
			VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode =
			VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform =
		support.capabilities.currentTransform;
	createInfo.compositeAlpha =
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(
		m_device,
		&createInfo,
		nullptr,
		&m_swapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swapchain");
	}

	// Retrieve swapchain images
	vkGetSwapchainImagesKHR(
		m_device,
		m_swapchain,
		&imageCount,
		nullptr);

	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(
		m_device,
		m_swapchain,
		&imageCount,
		m_swapchainImages.data());

	m_swapchainImageFormat = surfaceFormat.format;
	m_swapchainExtent = extent;

	std::cout << "Swapchain created successfully!" << std::endl;

}

void VulkanContext::CreateSwapchainImageViews()
{
	m_swapchainImageViews.resize(m_swapchainImages.size());

	for (size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapchainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(
			m_device,
			&createInfo,
			nullptr,
			&m_swapchainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swapchain image view");
		}
	}

	std::cout << "Swapchain image views created successfully!" << std::endl;

}

void VulkanContext::CreateRenderPass()
{
	// Color attachment
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	// Depth attachment
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthRef{};
	depthRef.attachment = 1;
	depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	// Subpasses
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthRef;

	std::array<VkAttachmentDescription, 2> attachments = {
		colorAttachment,
		depthAttachment
	};


	// Dependency
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;



	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(
		m_device,
		&renderPassInfo,
		nullptr,
		&m_renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass");
	}

	std::cout << "Render pass created successfully!" << std::endl;
}

void VulkanContext::CreatePipelines()
{
	// need to do this just before creating...
	CleanupPipelines();

	//CreateSkyboxPipeline();
	CreateGraphicsPipeline();
}


void VulkanContext::CleanupPipelines()
{
	//graphics pipeline
	if (m_graphicsPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
		m_graphicsPipeline = VK_NULL_HANDLE;
	}

	if (m_graphicsPipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(m_device, m_graphicsPipelineLayout, nullptr);
		m_graphicsPipelineLayout = VK_NULL_HANDLE;
	}

	//skybox pipeline
	/*
	if (m_skybox.pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device, m_skybox.pipeline, nullptr);
		m_skybox.pipeline = VK_NULL_HANDLE;
	}

	if (m_skybox.pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(m_device, m_skybox.pipelineLayout, nullptr);
		m_skybox.pipelineLayout = VK_NULL_HANDLE;
	}
	*/

}


void VulkanContext::CreateGraphicsPipeline()
{

	// Load SPIR-V shader code
	auto vertShaderCode = VulkanUtils::ReadFile("src/shaders/Shader.vert.spv");
	auto fragShaderCode = VulkanUtils::ReadFile("src/shaders/Shader.frag.spv");

	VkShaderModule vertShaderModule =
		VulkanUtils::CreateShaderModule(m_device, vertShaderCode);

	VkShaderModule fragShaderModule =
		VulkanUtils::CreateShaderModule(m_device, fragShaderCode);

	// Shader stages
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	auto binding = Vertex::GetBindingDescription();
	auto attributes = Vertex::GetAttributeDescriptions();

	// Vertex input with vertex buffers
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &binding;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology =
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport & scissor
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width =
		static_cast<float>(m_swapchainExtent.width);
	viewport.height =
		static_cast<float>(m_swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT /*VK_CULL_MODE_NONE*/;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Multisampling (disabled)
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples =
		VK_SAMPLE_COUNT_1_BIT;

	// Color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;


	// pipeline depth stencil stuff
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.stencilTestEnable = VK_FALSE;

	// push constant stuff
	VkPushConstantRange pushRange{};
	pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushRange.offset = 0;
	pushRange.size = sizeof(glm::mat4);

	// Pipeline layout with descriptors
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushRange;

	if (vkCreatePipelineLayout(
		m_device,
		&pipelineLayoutInfo,
		nullptr,
		&m_graphicsPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}

	// Graphics pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.layout = m_graphicsPipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(
		m_device,
		VK_NULL_HANDLE,
		1,
		&pipelineInfo,
		nullptr,
		&m_graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}

	// Shader modules no longer needed
	vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(m_device, vertShaderModule, nullptr);

	std::cout << "Graphics pipeline created successfully!" << std::endl;
}



void VulkanContext::CreateFramebuffers()
{
	m_swapchainFramebuffers.resize(m_swapchainImageViews.size());

	for (size_t i = 0; i < m_swapchainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
			m_swapchainImageViews[i], // color (attachment 0)
			m_depthImageView           // depth (attachment 1)
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); //
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_swapchainExtent.width;
		framebufferInfo.height = m_swapchainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(
			m_device,
			&framebufferInfo,
			nullptr,
			&m_swapchainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer");
		}
	}

	std::cout << "Framebuffers created successfully!" << std::endl;
}


void VulkanContext::CreateCommandPool()
{
	auto indices =
		VulkanUtils::FindQueueFamilies(
			m_physicalDevice,
			m_surface);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType =
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex =
		indices.graphicsFamily.value();
	poolInfo.flags =
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(
		m_device,
		&poolInfo,
		nullptr,
		&m_commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}

	std::cout << "Command pool created successfully!" << std::endl;
}

void VulkanContext::CreateCommandBuffers()
{
	m_commandBuffers.resize(m_swapchainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

	if (vkAllocateCommandBuffers(
		m_device,
		&allocInfo,
		m_commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers");
	}

	std::cout << "Command buffers allocated successfully!" << std::endl;
}


// not being used...
void VulkanContext::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const FrameResources& frame)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(
		commandBuffer,
		&beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin command buffer");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchainExtent;


	VkClearValue clearValues[2];
	clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(
		commandBuffer,
		&renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE);



	//draw call stuff for skybox...
	//DrawSkybox(commandBuffer, frame);



	/*

	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_graphicsPipeline);

	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_graphicsPipelineLayout,
		0,
		1,
		&frame.globalDescriptorSet,
		0,
		nullptr);
	*/




	//loop for binding and drawing objects

	//for (size_t j = 0; j < RenderObjects.size(); j++)
	//{
	//	RenderObject& obj = RenderObjects[j];

	//	/*vkCmdBindDescriptorSets(
	//		commandBuffer,
	//		VK_PIPELINE_BIND_POINT_GRAPHICS,
	//		m_pipelineLayout,
	//		0,
	//		1,
	//		&m_frames[m_currentFrame].descriptorSets[j],
	//		0,
	//		nullptr
	//	);*/


	//	obj.UpdateTransform();

	//	vkCmdPushConstants(commandBuffer, m_graphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &obj.transform);

	//	obj.mesh->Bind(commandBuffer);
	//	obj.mesh->Draw(commandBuffer);

	//}


	// REQUIRED
	RenderImGuiUI(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer");
	}
}



void VulkanContext::CreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType =
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType =
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	// Start signaled so first frame doesn't block
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(
			m_device,
			&semaphoreInfo,
			nullptr,
			&m_frames[i].imageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(
				m_device,
				&semaphoreInfo,
				nullptr,
				&m_frames[i].renderFinishedSemaphore) != VK_SUCCESS ||
			vkCreateFence(
				m_device,
				&fenceInfo,
				nullptr,
				&m_frames[i].inFlightFence) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create sync objects");
		}
	}


	std::cout << "Synchronization objects created successfully!" << std::endl;
}



//this function is being used to update the uniform buffer for camera movement to work

void VulkanContext::UpdateUniformBuffer(uint32_t frameIndex, const CameraUBO& cameraData)
{
	/*void* mapped;
	vkMapMemory(
		m_device,
		m_frames[frameIndex].uniformMemory,
		0,
		sizeof(UniformBufferObject),
		0,
		&mapped);

	memcpy(mapped, &ubo, sizeof(UniformBufferObject));
	vkUnmapMemory(m_device, m_frames[frameIndex].uniformMemory);
	*/

	void* mapped;
	vkMapMemory(
		m_device,
		m_frames[frameIndex].uniformMemory,
		0,
		sizeof(CameraUBO),
		0,
		&mapped);

	memcpy(mapped, &cameraData, sizeof(CameraUBO));
	vkUnmapMemory(m_device, m_frames[frameIndex].uniformMemory);
}


void VulkanContext::CopyBuffer(
	VkBuffer src,
	VkBuffer dst,
	VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cmd;
	vkAllocateCommandBuffers(
		m_device,
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

	vkQueueSubmit(m_graphicsQueue, 1, &submit, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_device, m_commandPool, 1, &cmd);
}


void VulkanContext::CreateDepthResources()
{
	// 1. Choose a supported depth format
	m_depthFormat =
		VulkanUtils::FindDepthFormat(m_physicalDevice);

	// 2. Create the depth image
	VulkanUtils::CreateImage(
		m_device,
		m_physicalDevice,
		m_swapchainExtent.width,
		m_swapchainExtent.height,
		1,
		m_depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_depthImage,
		m_depthImageMemory,
		0);

	// 3. Create the depth image view
	m_depthImageView =
		VulkanUtils::CreateImageView(
			m_device,
			m_depthImage,
			m_depthFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			1,
			VK_IMAGE_VIEW_TYPE_2D
		);
}


// Cleanup stuff

void VulkanContext::CleanupSwapchain()
{
	for (auto framebuffer : m_swapchainFramebuffers)
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	m_swapchainFramebuffers.clear();

	vkDestroyImageView(m_device, m_depthImageView, nullptr);
	vkDestroyImage(m_device, m_depthImage, nullptr);
	vkFreeMemory(m_device, m_depthImageMemory, nullptr);

	for (auto imageView : m_swapchainImageViews)
		vkDestroyImageView(m_device, imageView, nullptr);
	m_swapchainImageViews.clear();

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}


void VulkanContext::CleaupRenderPass()
{
	if (m_renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(m_device, m_renderPass, nullptr);
		m_renderPass = VK_NULL_HANDLE;
	}
}


void VulkanContext::CleanupFrameResources()
{
	for (auto& frame : m_frames)
	{
		vkDestroySemaphore(m_device, frame.imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(m_device, frame.renderFinishedSemaphore, nullptr);
		vkDestroyFence(m_device, frame.inFlightFence, nullptr);

		vkDestroyBuffer(m_device, frame.uniformBuffer, nullptr);
		vkFreeMemory(m_device, frame.uniformMemory, nullptr);
	}
}



void VulkanContext::DestroyDebugMessenger()
{
	if (m_debugMessenger != VK_NULL_HANDLE)
	{
		auto destroyFunc =
			(PFN_vkDestroyDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");

		if (destroyFunc)
			destroyFunc(m_instance, m_debugMessenger, nullptr);
	}

}


void VulkanContext::CreateDeviceBuffer(
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
		m_device,
		m_physicalDevice,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingMemory
	);

	void* mapped;
	vkMapMemory(m_device, stagingMemory, 0, size, 0, &mapped);
	memcpy(mapped, srcData, static_cast<size_t>(size));
	vkUnmapMemory(m_device, stagingMemory);

	//GPU buffer
	VulkanUtils::CreateBuffer(
		m_device,
		m_physicalDevice,
		size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		buffer,
		memory
	);

	CopyBuffer(stagingBuffer, buffer, size);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingMemory, nullptr);
}


void VulkanContext::BeginImGuiFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}


void VulkanContext::DrawImGuiUI()
{
	BeginImGuiFrame();

	if (m_drawImguiCallback)
	{
		m_drawImguiCallback();
	}
	else
	{
		std::cout << "IMGUI callback not assigned !" << std::endl;
	}

	ImGui::Render();
}

void VulkanContext::RenderImGuiUI(VkCommandBuffer commandBuffer)
{
	ImGui_ImplVulkan_RenderDrawData(
		ImGui::GetDrawData(),
		commandBuffer);
}

//will call this in engine class...
void VulkanContext::SetImguiDrawCallback(std::function<void()> callback)
{
	m_drawImguiCallback = callback;
}




