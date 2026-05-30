#include "ShadowPass.h"
#include "Scene.h"


void ShadowPass::Initialize(VulkanContext& vulkan)
{
	m_vulkan = &vulkan;

	CreateShadowMap();

	CreateRenderPass();

	CreateFramebuffers();

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

const std::array<CascadeData, SHADOW_CASCADE_COUNT>& ShadowPass::GetCascades() const
{
	return m_cascades;
}

void ShadowPass::Render(VkCommandBuffer cmd, const Scene& scene)
{

	for (uint32_t c = 0; c < SHADOW_CASCADE_COUNT; c++)
	{
		// -----------------------------------
		// Upload shadow UBO
		// -----------------------------------

		/*ShadowUBO ubo{};
		ubo.lightSpace = m_lightSpace;*/

		void* mapped;
		vkMapMemory(
			m_vulkan->GetDevice(),
			m_uniformMemories[c],
			0,
			sizeof(glm::mat4),
			0,
			&mapped
		);

		memcpy(mapped, &m_cascades[c], sizeof(glm::mat4));
		vkUnmapMemory(m_vulkan->GetDevice(), m_uniformMemories[c]);

		// -----------------------------------
		// Begin render pass
		// -----------------------------------

		VkClearValue clear{};
		clear.depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};

		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass;
		renderPassInfo.framebuffer = m_cascadeFramebuffers[c];
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

		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// -----------------------------------
		// Viewport
		// -----------------------------------

		VkViewport viewport{};

		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)SHADOW_SIZE;
		viewport.height = (float)SHADOW_SIZE;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(cmd, 0, 1, &viewport);


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

		vkCmdSetScissor(cmd, 0, 1, &scissor);


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
			&m_descriptorSets[c],
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

			auto& transform = scene.transforms.at(entity);

			glm::mat4 model = transform.GetMatrix();

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
	//end of for loop

}

void ShadowPass::Cleanup(VkDevice device)
{

	// Destroy per-cascade image views
	for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
	{
		if (m_cascadeImageViews[i] != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device, m_cascadeImageViews[i], nullptr);
			m_cascadeImageViews[i] = VK_NULL_HANDLE;
		}

		if (m_cascadeFramebuffers[i] != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(device, m_cascadeFramebuffers[i], nullptr);
			m_cascadeFramebuffers[i] = VK_NULL_HANDLE;
		}

		if (m_uniformBuffers[i] != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(device, m_uniformBuffers[i], nullptr);
			vkFreeMemory(device, m_uniformMemories[i], nullptr);
			m_uniformBuffers[i] = VK_NULL_HANDLE;
			m_uniformMemories[i] = VK_NULL_HANDLE;
		}
	}


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

	for (auto frameBuffer : m_cascadeFramebuffers)
	{
		if (frameBuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(
				device,
				frameBuffer,
				nullptr
			);
		}
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

	for (auto buffer : m_uniformBuffers)
	{
		if (buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(
				device,
				buffer,
				nullptr
			);
		}
	}

	for (auto mem : m_uniformMemories)
	{
		if (mem != VK_NULL_HANDLE)
		{
			vkFreeMemory(
				device,
				mem,
				nullptr
			);
		}
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



// Simple version of update cascades...
void ShadowPass::UpdateCascades(
	const glm::mat4& cameraView,
	const glm::mat4& cameraProj,
	const glm::vec3& lightDir,
	float cameraNear,
	float cameraFar)
{
	// Simple fixed sizes per cascade — tune these to your scene
	float sizes[SHADOW_CASCADE_COUNT] = { 15.0f, 30.0f, 60.0f, 120.0f };
	float splits[SHADOW_CASCADE_COUNT] = { 10.0f, 25.0f, 60.0f, 150.0f };

	glm::vec3 target = glm::vec3(glm::inverse(cameraView)[3]);

	for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
	{
		float s = sizes[i];

		glm::vec3 lightPos = target - glm::normalize(lightDir) * 50.0f;

		glm::mat4 lightView = glm::lookAt(
			lightPos,
			target,
			glm::vec3(0.0f, 1.0f, 0.0f)
		);


		// Shimmering fix apparently...

		// Snap target to texel grid in light space
		// This prevents sub-texel movement between frames
		float worldUnitsPerTexel = (s * 2.0f) / (float)SHADOW_SIZE;

		// Transform origin into light space
		glm::vec4 originLS = lightView * glm::vec4(target, 1.0f);

		// Snap to nearest texel
		originLS.x = floor(originLS.x / worldUnitsPerTexel) * worldUnitsPerTexel;
		originLS.y = floor(originLS.y / worldUnitsPerTexel) * worldUnitsPerTexel;

		// Reconstruct snapped target in world space
		glm::vec4 snappedLS = glm::inverse(lightView) * originLS;
		glm::vec3 snappedTarget = glm::vec3(snappedLS);

		// Rebuild light view with snapped target
		lightView = glm::lookAt(
			snappedTarget - glm::normalize(lightDir) * 50.0f,
			snappedTarget,
			glm::vec3(0.0f, 1.0f, 0.0f)
		);



		glm::mat4 lightProj = glm::orthoRH_ZO(
			-s, s,
			-s, s,
			0.01f, 1000.0f
		);
		lightProj[1][1] *= -1.0f;

		m_cascades[i].lightSpace = lightProj * lightView;
		m_cascades[i].splitDepth = splits[i];
	}
}




void ShadowPass::CreateShadowMap()
{
	auto device = m_vulkan->GetDevice();

	auto physicalDevice = m_vulkan->GetPhysicalDevice();

	VulkanUtils::CreateImage(
		device,
		physicalDevice,
		SHADOW_SIZE,
		SHADOW_SIZE,
		SHADOW_CASCADE_COUNT,
		VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_shadowImage,
		m_shadowMemory,
		0
	);

	m_shadowImageView = VulkanUtils::CreateImageView(
		device,
		m_shadowImage,
		VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		SHADOW_CASCADE_COUNT,
		VK_IMAGE_VIEW_TYPE_2D_ARRAY
	);


	// Per-cascade views — used by framebuffers to render into each layer
	for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
	{
		m_cascadeImageViews[i] = VulkanUtils::CreateImageView(
			device,
			m_shadowImage,
			VK_FORMAT_D32_SFLOAT,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			1,
			VK_IMAGE_VIEW_TYPE_2D,
			i
		);

	}

	m_shadowSampler = VulkanUtils::CreateSampler(
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

void ShadowPass::CreateFramebuffers()
{
	for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
	{
		VkImageView attachments[] = { m_cascadeImageViews[i], };

		VkFramebufferCreateInfo framebufferInfo{};

		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = SHADOW_SIZE;
		framebufferInfo.height = SHADOW_SIZE;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(
			m_vulkan->GetDevice(),
			&framebufferInfo,
			nullptr,
			&m_cascadeFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shadow framebuffer!");
		}

	}
}

void ShadowPass::CreateDescriptors()
{


	auto& descriptorManager = m_vulkan->GetDescriptorManager();

	VkDescriptorSetLayoutBinding binding{};

	binding.binding = 0;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	binding.descriptorCount = 1;
	binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	m_descriptorLayout = m_vulkan->CreateDescriptorSetLayout({ binding });

	for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
	{

		VulkanUtils::CreateBuffer(
			m_vulkan->GetDevice(),
			m_vulkan->GetPhysicalDevice(),
			sizeof(glm::mat4),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_uniformBuffers[i],
			m_uniformMemories[i]
		);

		m_descriptorSets[i] = descriptorManager.AllocateDescriptorSet(m_descriptorLayout);

		descriptorManager.UpdateBuffer(
			m_descriptorSets[i],
			0,
			m_uniformBuffers[i],
			sizeof(glm::mat4)
		);


	}

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
