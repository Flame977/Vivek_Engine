#include "ShadowPass.h"
#include "Scene.h"


void ShadowPass::Initialize(VulkanContext& vulkan)
{
	m_vulkan = &vulkan;

	CreateShadowMap();

	CreateRenderPass();

	CreateFramebuffer();

	CreateDescriptors();

	CreatePipeline();
}

VkImageView ShadowPass::GetShadowImageView() const
{
	return m_shadowImageView;
}

VkSampler ShadowPass::GetShadowSampler() const
{
	return m_shadowSampler;
}

void ShadowPass::Render(
	VkCommandBuffer cmd,
	const Scene& scene)
{


	// -----------------------------------
	// Upload shadow UBO
	// -----------------------------------

	ShadowUBO ubo{};
	ubo.lightSpace = m_lightSpace;

	void* mapped;

	vkMapMemory(
		m_vulkan->GetDevice(),
		m_uniformMemory,
		0,
		sizeof(ShadowUBO),
		0,
		&mapped
	);

	memcpy(mapped, &ubo, sizeof(ShadowUBO));

	vkUnmapMemory(
		m_vulkan->GetDevice(),
		m_uniformMemory
	);

	// -----------------------------------
	// Begin render pass
	// -----------------------------------

	VkClearValue clear{};
	clear.depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType =
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

	renderPassInfo.renderPass =
		m_renderPass;

	renderPassInfo.framebuffer =
		m_framebuffer;

	renderPassInfo.renderArea.offset =
	{
		0,0
	};

	renderPassInfo.renderArea.extent =
	{
		SHADOW_SIZE,
		SHADOW_SIZE
	};

	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clear;

	vkCmdBeginRenderPass(
		cmd,
		&renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE
	);

	// -----------------------------------
	// Viewport
	// -----------------------------------

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	viewport.width =
		(float)SHADOW_SIZE;

	viewport.height =
		(float)SHADOW_SIZE;

	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(
		cmd,
		0,
		1,
		&viewport
	);

	// -----------------------------------
	// Scissor
	// -----------------------------------

	VkRect2D scissor{};
	scissor.offset = { 0,0 };

	scissor.extent =
	{
		SHADOW_SIZE,
		SHADOW_SIZE
	};

	vkCmdSetScissor(
		cmd,
		0,
		1,
		&scissor
	);

	// -----------------------------------
	// Bind pipeline
	// -----------------------------------

	vkCmdBindPipeline(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipeline
	);

	// -----------------------------------
	// Bind descriptors
	// -----------------------------------

	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		0,
		1,
		&m_descriptorSet,
		0,
		nullptr
	);


	// -----------------------------------
	// Draw meshes
	// -----------------------------------

	for (auto& [entity, meshComp] : scene.meshs)
	{
		if (!scene.transforms.count(entity))
			continue;

		auto& transform =
			scene.transforms.at(entity);

		glm::mat4 model =
			transform.GetMatrix();

		vkCmdPushConstants(
			cmd,
			m_pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(glm::mat4),
			&model
		);

		meshComp.mesh->Bind(cmd);

		meshComp.mesh->Draw(cmd);
	}

	// -----------------------------------
	// End pass
	// -----------------------------------

	vkCmdEndRenderPass(cmd);


}

void ShadowPass::Cleanup(VkDevice device)
{
	if (m_pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(
			device,
			m_pipeline,
			nullptr
		);
	}

	if (m_pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(
			device,
			m_pipelineLayout,
			nullptr
		);
	}

	if (m_renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(
			device,
			m_renderPass,
			nullptr
		);
	}

	if (m_framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(
			device,
			m_framebuffer,
			nullptr
		);
	}

	if (m_shadowImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(
			device,
			m_shadowImageView,
			nullptr
		);
	}

	if (m_shadowSampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(
			device,
			m_shadowSampler,
			nullptr
		);
	}

	if (m_shadowImage != VK_NULL_HANDLE)
	{
		vkDestroyImage(
			device,
			m_shadowImage,
			nullptr
		);
	}

	if (m_shadowMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(
			device,
			m_shadowMemory,
			nullptr
		);
	}

	if (m_uniformBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(
			device,
			m_uniformBuffer,
			nullptr
		);
	}

	if (m_uniformMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(
			device,
			m_uniformMemory,
			nullptr
		);
	}

	if (m_descriptorLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(
			device,
			m_descriptorLayout,
			nullptr
		);
	}
}

glm::mat4 ShadowPass::GetLightSpace() const
{
	return m_lightSpace;
}

void ShadowPass::SetLightSpace(
	const glm::mat4& matrix)
{
	m_lightSpace = matrix;
}

glm::mat4 ShadowPass::GetLightView() const
{
	return m_lightView;
}

glm::mat4 ShadowPass::GetLightProjection() const
{
	return m_lightProj;
}

void ShadowPass::CreateShadowMap()
{
	auto device =
		m_vulkan->GetDevice();

	auto physicalDevice =
		m_vulkan->GetPhysicalDevice();

	VulkanUtils::CreateImage(
		device,
		physicalDevice,
		SHADOW_SIZE,
		SHADOW_SIZE,
		1,
		VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_shadowImage,
		m_shadowMemory,
		0
	);

	m_shadowImageView =
		VulkanUtils::CreateImageView(
			device,
			m_shadowImage,
			VK_FORMAT_D32_SFLOAT,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			1,
			VK_IMAGE_VIEW_TYPE_2D
		);

	m_shadowSampler =
		VulkanUtils::CreateSampler(
			device,
			VK_FILTER_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			VK_FALSE,
			VK_COMPARE_OP_ALWAYS
		);
}


void ShadowPass::CreateRenderPass()
{
	VkAttachmentDescription depthAttachment{};

	depthAttachment.format =
		VK_FORMAT_D32_SFLOAT;

	depthAttachment.samples =
		VK_SAMPLE_COUNT_1_BIT;

	depthAttachment.loadOp =
		VK_ATTACHMENT_LOAD_OP_CLEAR;

	depthAttachment.storeOp =
		VK_ATTACHMENT_STORE_OP_STORE;

	depthAttachment.stencilLoadOp =
		VK_ATTACHMENT_LOAD_OP_DONT_CARE;

	depthAttachment.stencilStoreOp =
		VK_ATTACHMENT_STORE_OP_DONT_CARE;

	depthAttachment.initialLayout =
		VK_IMAGE_LAYOUT_UNDEFINED;

	depthAttachment.finalLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthRef{};
	depthRef.attachment = 0;

	depthRef.layout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint =
		VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 0;
	subpass.pColorAttachments = nullptr;

	subpass.pDepthStencilAttachment =
		&depthRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType =
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments =
		&depthAttachment;

	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses =
		&subpass;

	if (vkCreateRenderPass(
		m_vulkan->GetDevice(),
		&renderPassInfo,
		nullptr,
		&m_renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error(
			"Failed to create shadow render pass!"
		);
	}
}

void ShadowPass::CreateFramebuffer()
{
	VkImageView attachments[] =
	{
		m_shadowImageView
	};

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType =
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

	framebufferInfo.renderPass =
		m_renderPass;

	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments =
		attachments;

	framebufferInfo.width =
		SHADOW_SIZE;

	framebufferInfo.height =
		SHADOW_SIZE;

	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(
		m_vulkan->GetDevice(),
		&framebufferInfo,
		nullptr,
		&m_framebuffer) != VK_SUCCESS)
	{
		throw std::runtime_error(
			"Failed to create shadow framebuffer!"
		);
	}
}

void ShadowPass::CreateDescriptors()
{
	auto& descriptorManager =
		m_vulkan->GetDescriptorManager();

	VkDescriptorSetLayoutBinding binding{};
	binding.binding = 0;

	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	binding.descriptorCount = 1;

	binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	m_descriptorLayout =
		m_vulkan->CreateDescriptorSetLayout(
			{ binding }
		);

	VulkanUtils::CreateBuffer(
		m_vulkan->GetDevice(),
		m_vulkan->GetPhysicalDevice(),
		sizeof(ShadowUBO),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_uniformBuffer,
		m_uniformMemory
	);

	m_descriptorSet =
		descriptorManager.AllocateDescriptorSet(
			m_descriptorLayout
		);

	descriptorManager.UpdateBuffer(
		m_descriptorSet,
		0,
		m_uniformBuffer,
		sizeof(ShadowUBO)
	);
}

void ShadowPass::CreatePipeline()
{
	auto device =
		m_vulkan->GetDevice();

	// -----------------------------------
	// Load shaders
	// -----------------------------------

	auto vertCode =
		VulkanUtils::ReadFile(
			"src/shaders/Shadows.vert.spv"
		);

	auto fragCode =
		VulkanUtils::ReadFile(
			"src/shaders/Shadows.frag.spv"
		);

	VkShaderModule vertModule =
		VulkanUtils::CreateShaderModule(
			device,
			vertCode
		);

	VkShaderModule fragModule =
		VulkanUtils::CreateShaderModule(
			device,
			fragCode
		);

	// -----------------------------------
	// Shader stages
	// -----------------------------------

	VkPipelineShaderStageCreateInfo vertStage{};
	vertStage.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

	vertStage.stage =
		VK_SHADER_STAGE_VERTEX_BIT;

	vertStage.module =
		vertModule;

	vertStage.pName = "main";

	VkPipelineShaderStageCreateInfo fragStage{};
	fragStage.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

	fragStage.stage =
		VK_SHADER_STAGE_FRAGMENT_BIT;

	fragStage.module =
		fragModule;

	fragStage.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		vertStage,
		fragStage
	};

	// -----------------------------------
	// Vertex Input
	// -----------------------------------

	auto bindingDescription =
		Vertex::GetBindingDescription();

	auto attributeDescriptions =
		Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	vertexInput.vertexBindingDescriptionCount = 1;

	vertexInput.pVertexBindingDescriptions =
		&bindingDescription;

	vertexInput.vertexAttributeDescriptionCount =
		static_cast<uint32_t>(
			attributeDescriptions.size());

	vertexInput.pVertexAttributeDescriptions =
		attributeDescriptions.data();

	// -----------------------------------
	// Input Assembly
	// -----------------------------------

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

	inputAssembly.topology =
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// -----------------------------------
	// Viewport
	// -----------------------------------

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	viewport.width =
		(float)SHADOW_SIZE;

	viewport.height =
		(float)SHADOW_SIZE;

	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// -----------------------------------
	// Scissor
	// -----------------------------------

	VkRect2D scissor{};
	scissor.offset = { 0,0 };

	scissor.extent =
	{
		SHADOW_SIZE,
		SHADOW_SIZE
	};

	// -----------------------------------
	// Viewport State
	// -----------------------------------

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;

	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// -----------------------------------
	// Rasterizer
	// -----------------------------------

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

	rasterizer.depthClampEnable =
		VK_FALSE;

	rasterizer.rasterizerDiscardEnable =
		VK_FALSE;

	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

	rasterizer.lineWidth = 1.0f;

	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;

	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	// IMPORTANT:
	// shadow acne prevention
	//disabling for now...
	rasterizer.depthBiasEnable = VK_TRUE;

	rasterizer.depthBiasConstantFactor = 0.25f;
	rasterizer.depthBiasSlopeFactor = 0.25f;
	rasterizer.depthBiasClamp = 0.0f;



	// -----------------------------------
	// Multisampling
	// -----------------------------------

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

	multisampling.rasterizationSamples =
		VK_SAMPLE_COUNT_1_BIT;

	// -----------------------------------
	// Depth
	// -----------------------------------

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	depthStencil.depthTestEnable =
		VK_TRUE;

	depthStencil.depthWriteEnable =
		VK_TRUE;

	depthStencil.depthCompareOp =
		VK_COMPARE_OP_LESS;

	depthStencil.depthBoundsTestEnable =
		VK_FALSE;

	depthStencil.stencilTestEnable =
		VK_FALSE;

	// -----------------------------------
	// NO COLOR ATTACHMENTS
	// -----------------------------------

	VkPipelineColorBlendStateCreateInfo colorBlend{};
	colorBlend.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

	colorBlend.attachmentCount = 0;
	colorBlend.pAttachments = nullptr;

	// -----------------------------------
	// Push Constants
	// -----------------------------------

	VkPushConstantRange pushRange{};
	pushRange.stageFlags =
		VK_SHADER_STAGE_VERTEX_BIT;

	pushRange.offset = 0;

	pushRange.size =
		sizeof(glm::mat4);

	// -----------------------------------
	// Pipeline Layout
	// -----------------------------------

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	layoutInfo.setLayoutCount = 1;

	layoutInfo.pSetLayouts =
		&m_descriptorLayout;

	layoutInfo.pushConstantRangeCount = 1;

	layoutInfo.pPushConstantRanges =
		&pushRange;

	if (vkCreatePipelineLayout(
		device,
		&layoutInfo,
		nullptr,
		&m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error(
			"Failed to create shadow pipeline layout!"
		);
	}

	// -----------------------------------
	// Graphics Pipeline
	// -----------------------------------

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType =
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages =
		shaderStages;

	pipelineInfo.pVertexInputState =
		&vertexInput;

	pipelineInfo.pInputAssemblyState =
		&inputAssembly;

	pipelineInfo.pViewportState =
		&viewportState;

	pipelineInfo.pRasterizationState =
		&rasterizer;

	pipelineInfo.pMultisampleState =
		&multisampling;

	pipelineInfo.pDepthStencilState =
		&depthStencil;

	pipelineInfo.pColorBlendState =
		&colorBlend;

	pipelineInfo.layout =
		m_pipelineLayout;

	pipelineInfo.renderPass =
		m_renderPass;

	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(
		device,
		VK_NULL_HANDLE,
		1,
		&pipelineInfo,
		nullptr,
		&m_pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error(
			"Failed to create shadow pipeline!"
		);
		return;
	}

	// -----------------------------------
	// Cleanup shader modules
	// -----------------------------------

	vkDestroyShaderModule(
		device,
		vertModule,
		nullptr
	);

	vkDestroyShaderModule(
		device,
		fragModule,
		nullptr
	);

	Log("ShadowPass :: Created Shadow Pipeline!");
}
