#include "Renderer.h"
#include "VulkanContext.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "RenderObject.h"
#include "Scene.h"
#include "ImGuizmo.h"

Renderer::Renderer(VulkanContext& vulkan) :m_vulkan(vulkan)
{
	InitRenderer();
}

Renderer::~Renderer()
{
	Cleanup();
}

void Renderer::InitRenderer()
{
	CreateDescriptorLayouts();

	CreateFrameDescriptors();

	CreateSkybox();

	CreateSkyboxDescriptors();

	CreatePipelines();

}


void Renderer::OnResize()
{
	m_vulkan.RecreateSwapchain();
	auto device = m_vulkan.GetDevice();

	CleanupPipelines(device);
	CreatePipelines();
}

void Renderer::WaitIdle()
{
	m_vulkan.WaitIdle();
}

void Renderer::DrawFrame(const Camera& camera, const Scene& scene)
{
	//scene building logic...


	// OG DRAW LOGIC...

	CameraUBO camData{};

	camData.view = camera.GetView();
	camData.proj = camera.GetProjection();

	camData.lightInfo.x = 1;

	// Directional light
	camData.lights[0].position.w = 0.0f; // type = directional
	camData.lights[0].direction = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
	camData.lights[0].color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	
	/*
	// Spot light
	camData.lights[1].position = glm::vec4(0.0f, 3.0f, -2.0f, 2.0f); // type = spot
	camData.lights[1].direction = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
	camData.lights[1].color = glm::vec4(10.0f, 0.0f, 0.0f, 1.0f);


	// cone angles (in radians -> convert to cos)
	float inner = glm::cos(glm::radians(15.0f));
	float outer = glm::cos(glm::radians(25.0f));

	camData.lights[1].params.x = 10.0f; // range
	camData.lights[1].params.y = inner;
	camData.lights[1].params.z = outer;
	*/


	m_vulkan.UpdateUniforms(camData);

	// OG LOGIC...
	//m_vulkan.DrawFrame();

	//NEW LOGIC...
	/*
	*/

	uint32_t imageIndex;
	FrameResources* frame;

	m_vulkan.BeginFrame(imageIndex, frame);

	m_vulkan.DrawImGuiUI();

	VkCommandBuffer cmd = m_vulkan.GetCommandBuffer(imageIndex);

	RecordCommands(cmd, imageIndex, *frame, scene);

	m_vulkan.EndFrame(imageIndex, *frame);

}


Mesh* Renderer::LoadMesh(const std::string& path)
{
	auto mesh = std::make_unique<Mesh>(CreateMesh(MeshLoader::Load(path)));

	Mesh* ptr = mesh.get();
	m_meshes.push_back(std::move(mesh));

	return ptr;
}

Material* Renderer::LoadMaterial(const std::string& path)
{
	auto mat = std::make_unique<Material>(CreateMaterial(path));

	mat->CreateDescriptorSet(
		m_vulkan.GetDevice(),
		m_vulkan.GetDescriptorManager(),
		m_materialDescriptorSetLayout
	);

	Material* ptr = mat.get();
	m_materials.push_back(std::move(mat));

	return ptr;
}


Mesh Renderer::CreateMesh(const MeshData& data)
{
	//auto d = MeshLoader::Load("");

	auto device = m_vulkan.GetDevice();
	auto commandPool = m_vulkan.GetCommandPool();
	auto graphicsQueue = m_vulkan.GetGraphicsQueue();
	auto physicalDevice = m_vulkan.GetPhysicalDevice();

	Mesh mesh;
	mesh.m_device = m_vulkan.GetDevice();
	mesh.m_indexCount = static_cast<uint32_t>(data.indices.size());

	VulkanUtils::CreateDeviceBuffer(
		device,
		commandPool,
		graphicsQueue,
		physicalDevice,
		data.vertices.data(),
		sizeof(Vertex) * data.vertices.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		mesh.m_vertexBuffer,
		mesh.m_vertexBufferMemory
	);


	VulkanUtils::CreateDeviceBuffer(
		device,
		commandPool,
		graphicsQueue,
		physicalDevice,
		data.indices.data(),
		sizeof(uint32_t) * data.indices.size(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		mesh.m_indexBuffer,
		mesh.m_indexBufferMemory
	);

	return mesh;
}


Material Renderer::CreateMaterial(const std::string& texPath)
{
	auto device = m_vulkan.GetDevice();
	auto commandPool = m_vulkan.GetCommandPool();
	auto graphicsQueue = m_vulkan.GetGraphicsQueue();
	auto physicalDevice = m_vulkan.GetPhysicalDevice();

	Material mat;

	mat.m_albedo = std::make_unique<Texture>(
		device,
		physicalDevice,
		commandPool,
		graphicsQueue,
		texPath
	);

	return mat;
}


RenderObject* Renderer::CreateRenderObject(Scene& scene, const std::string& name, Mesh* mesh, Material* material, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
{
	auto obj = scene.CreateObject(
		name,
		mesh,
		material,
		pos,
		rot,
		scale);

	return &obj;
}

void Renderer::DestroyRenderObject(Scene& scene, RenderObject* obj)
{
	scene.DestroyLastObject();
}

std::vector<RenderObject>& Renderer::GetRenderObjects(Scene& scene)
{
	return scene.GetObjects();
}

void Renderer::SetFPS(float fps)
{
	m_vulkan.SetDisplayFps(fps);
}

float Renderer::GetFPS() const
{
	return m_vulkan.GetFps();
}

void Renderer::RecordCommands(
	VkCommandBuffer cmd,
	uint32_t imageIndex,
	const FrameResources& frame,
	const Scene& scene)
{

	// Begin render pass
	m_vulkan.BeginRenderPass(cmd, imageIndex);

	DrawSkybox(cmd, frame);

	DrawObjects(cmd, frame, scene);

	m_vulkan.RenderImGuiUI(cmd);

	// End render pass
	m_vulkan.EndRenderPass(cmd);
}


void Renderer::CreateDescriptorLayouts()
{
	// SET 0 (Frame)
	VkDescriptorSetLayoutBinding frameBinding{};
	frameBinding.binding = 0;
	frameBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	frameBinding.descriptorCount = 1;
	frameBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	m_frameDescriptorSetLayout =
		m_vulkan.CreateDescriptorSetLayout({ frameBinding });


	// SET 1 (Material)
	VkDescriptorSetLayoutBinding materialBinding{};
	materialBinding.binding = 0;
	materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialBinding.descriptorCount = 1;
	materialBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	m_materialDescriptorSetLayout =
		m_vulkan.CreateDescriptorSetLayout({ materialBinding });


	//SET 0 (skybox)
	std::vector<VkDescriptorSetLayoutBinding> skyboxBindings{};

	VkDescriptorSetLayoutBinding cubemapBinding{};
	cubemapBinding.binding = 0;
	cubemapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	cubemapBinding.descriptorCount = 1;
	cubemapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	cubemapBinding.pImmutableSamplers = nullptr;

	skyboxBindings.push_back(cubemapBinding);

	m_skybox.descriptorSetLayout =
		m_vulkan.CreateDescriptorSetLayout(skyboxBindings);

}


void Renderer::CreateGraphicsPipeline()
{
	VkDevice device = m_vulkan.GetDevice();
	VkRenderPass renderPass = m_vulkan.GetRenderPass();
	VkExtent2D extent = m_vulkan.GetSwapchainExtent();

	// Load shaders
	auto vertShaderCode = VulkanUtils::ReadFile("src/shaders/Shader.vert.spv");
	auto fragShaderCode = VulkanUtils::ReadFile("src/shaders/Shader.frag.spv");

	VkShaderModule vertShaderModule =
		VulkanUtils::CreateShaderModule(device, vertShaderCode);

	VkShaderModule fragShaderModule =
		VulkanUtils::CreateShaderModule(device, fragShaderCode);

	// Shader stages
	VkPipelineShaderStageCreateInfo vertStage{};
	vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertStage.module = vertShaderModule;
	vertStage.pName = "main";

	VkPipelineShaderStageCreateInfo fragStage{};
	fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStage.module = fragShaderModule;
	fragStage.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };

	// Vertex input
	auto binding = Vertex::GetBindingDescription();
	auto attributes = Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &binding;
	vertexInputInfo.vertexAttributeDescriptionCount =
		static_cast<uint32_t>(attributes.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

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
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Depth testing
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.stencilTestEnable = VK_FALSE;

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

	// Push constant (model matrix)
	VkPushConstantRange pushRange{};
	pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushRange.offset = 0;
	pushRange.size = sizeof(glm::mat4);

	// Pipeline layout (USES RENDERER LAYOUTS)
	std::array<VkDescriptorSetLayout, 2> layouts =
	{
		m_frameDescriptorSetLayout,     // set 0
		m_materialDescriptorSetLayout   // set 1
	};

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
	layoutInfo.pSetLayouts = layouts.data();
	layoutInfo.pushConstantRangeCount = 1;
	layoutInfo.pPushConstantRanges = &pushRange;

	if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &m_graphicsPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}

	// Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;

	// CRITICAL
	pipelineInfo.layout = m_graphicsPipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(
		device,
		VK_NULL_HANDLE,
		1,
		&pipelineInfo,
		nullptr,
		&m_graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}

	// Cleanup shader modules
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);

	std::cout << "Renderer :: graphics pipeline created successfully!" << std::endl;
}

void Renderer::CreateFrameDescriptors()
{
	auto& descriptorManager = m_vulkan.GetDescriptorManager();

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		FrameResources& frame = m_vulkan.GetFrame(i);

		// Allocate descriptor set (same as before)
		frame.globalDescriptorSet =
			descriptorManager.AllocateDescriptorSet(m_frameDescriptorSetLayout);

		// Update UBO 
		descriptorManager.UpdateBuffer(
			frame.globalDescriptorSet,
			0, // binding
			frame.uniformBuffer,
			sizeof(CameraUBO)
		);
	}

	std::cout << "Renderer :: Frame descriptors created and descriptors Update called!" << std::endl;
}

void Renderer::CreateSkyboxDescriptors()
{
	auto& descriptorManager = m_vulkan.GetDescriptorManager();

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		FrameResources& frame = m_vulkan.GetFrame(i);

		// Allocate descriptor set (same as before)
		m_skybox.descriptorSet =
			descriptorManager.AllocateDescriptorSet(m_skybox.descriptorSetLayout);

		// Update Skybox
		descriptorManager.UpdateSkybox(
			m_skybox.descriptorSet,
			m_skybox.imageView,
			m_skybox.sampler
		);
	}

	std::cout << "Renderer :: Skybox descriptors created and descriptors Update called!" << std::endl;
}

void Renderer::Cleanup()
{
	VkDevice device = m_vulkan.GetDevice();

	for (auto& mesh : m_meshes)
		mesh->Destroy();

	for (auto& mat : m_materials)
		mat->Destroy();

	m_skybox.Destroy(device);


	if (m_graphicsPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
		m_graphicsPipeline = VK_NULL_HANDLE;
	}

	if (m_graphicsPipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(device, m_graphicsPipelineLayout, nullptr);
		m_graphicsPipelineLayout = VK_NULL_HANDLE;
	}

	if (m_frameDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device, m_frameDescriptorSetLayout, nullptr);
		m_frameDescriptorSetLayout = VK_NULL_HANDLE;
	}

	if (m_materialDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device, m_materialDescriptorSetLayout, nullptr);
		m_materialDescriptorSetLayout = VK_NULL_HANDLE;
	}
}

void Renderer::CreatePipelines()
{
	CreateSkyboxPipeline();
	CreateGraphicsPipeline();
}

void Renderer::CleanupPipelines(VkDevice device)
{
	vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, m_graphicsPipelineLayout, nullptr);

	vkDestroyPipeline(device, m_skybox.pipeline, nullptr);
	vkDestroyPipelineLayout(device, m_skybox.pipelineLayout, nullptr);
}


void Renderer::CreateSkyboxPipeline()
{
	auto device = m_vulkan.GetDevice();
	auto swapchainExtent = m_vulkan.GetSwapchainExtent();
	auto renderPass = m_vulkan.GetRenderPass();

	auto vertShaderCode = VulkanUtils::ReadFile("src/shaders/skybox.vert.spv");
	auto fragShaderCode = VulkanUtils::ReadFile("src/shaders/skybox.frag.spv");

	VkShaderModule vertShaderModule =
		VulkanUtils::CreateShaderModule(device, vertShaderCode);

	VkShaderModule fragShaderModule =
		VulkanUtils::CreateShaderModule(device, fragShaderCode);

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

	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	// POSITION ONLY
	VkVertexInputBindingDescription binding{};
	binding.binding = 0;
	binding.stride = sizeof(glm::vec3);
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attribute{};
	attribute.binding = 0;
	attribute.location = 0;
	attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute.offset = 0;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &binding;
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = &attribute;

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Viewport & scissor (same as yours)
	VkViewport viewport{};
	viewport.width = (float)swapchainExtent.width;
	viewport.height = (float)swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.extent = swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// RASTERIZER (FRONT CULL)
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Color blend
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	// DEPTH SETTINGS
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_FALSE; // KEY
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;


	// PIPELINE LAYOUT
	std::array<VkDescriptorSetLayout, 2> layouts =
	{
		m_frameDescriptorSetLayout,           // set 0 => camera
		m_skybox.descriptorSetLayout     // set 1 => cubemap
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
	pipelineLayoutInfo.pSetLayouts = layouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;


	if (vkCreatePipelineLayout(
		device,
		&pipelineLayoutInfo,
		nullptr,
		&m_skybox.pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}


	// Pipeline layout (already created)
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
	pipelineInfo.layout = m_skybox.pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(
		device,
		VK_NULL_HANDLE,
		1,
		&pipelineInfo,
		nullptr,
		&m_skybox.pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create skybox pipeline");
	}

	// Cleanup
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);

	std::cout << "Renderer :: Skybox pipeline created successfully!" << std::endl;

}



// VERY PARTICULAR PROCESS of creating a skybox...
void Renderer::CreateSkybox()
{
	auto device = m_vulkan.GetDevice();
	auto physicalDevice = m_vulkan.GetPhysicalDevice();
	auto commandPool = m_vulkan.GetCommandPool();
	auto graphicsQueue = m_vulkan.GetGraphicsQueue();


	// THIS NEEDS TO BE A PARAMETER ... will do that later...
	std::vector<std::string> faces =
	{
		"Assets/Textures/Skybox/right.png",
		"Assets/Textures/Skybox/left.png",
		"Assets/Textures/Skybox/up.png",
		"Assets/Textures/Skybox/down.png",
		"Assets/Textures/Skybox/front.png",
		"Assets/Textures/Skybox/back.png"
	};

	int width, height, channels;
	std::vector<unsigned char*> textures(6);

	// load faces
	for (int i = 0; i < 6; i++)
	{
		textures[i] = stbi_load(
			faces[i].c_str(),
			&width,
			&height,
			&channels,
			STBI_rgb_alpha);

		if (!textures[i])
		{
			std::cout << "Failed to load: " << faces[i] << std::endl;
			std::cout << "Reason: " << stbi_failure_reason() << std::endl;
			throw std::runtime_error("Failed to load skybox face!");
		}
	}

	VkDeviceSize imageSize = width * height * 4;
	VkDeviceSize totalSize = imageSize * 6;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VulkanUtils::CreateBuffer(
		device,
		physicalDevice,
		totalSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingMemory);

	// copy all faces to staging buffer
	void* data;
	vkMapMemory(device, stagingMemory, 0, totalSize, 0, &data);

	for (int i = 0; i < 6; i++)
	{
		memcpy(
			(char*)data + (imageSize * i),
			textures[i],
			imageSize
		);

		stbi_image_free(textures[i]); // free after copy
	}

	vkUnmapMemory(device, stagingMemory);

	// create cube map image
	VulkanUtils::CreateImage(
		device,
		physicalDevice,
		width,
		height,
		6,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_skybox.image,
		m_skybox.memory,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
	);

	// prepare copy regions
	std::vector<VkBufferImageCopy> regions(6);

	for (uint32_t i = 0; i < 6; i++)
	{
		regions[i].bufferOffset = imageSize * i;

		regions[i].bufferRowLength = 0;
		regions[i].bufferImageHeight = 0;

		regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		regions[i].imageSubresource.mipLevel = 0;
		regions[i].imageSubresource.baseArrayLayer = i; // each face
		regions[i].imageSubresource.layerCount = 1;

		regions[i].imageOffset = { 0, 0, 0 };

		regions[i].imageExtent.width = width;
		regions[i].imageExtent.height = height;
		regions[i].imageExtent.depth = 1;
	}

	// transition to TRANSFER_DST
	VulkanUtils::TransitionImageLayout(
		m_vulkan.GetDevice(),
		m_vulkan.GetCommandPool(),
		m_vulkan.GetGraphicsQueue(),
		m_skybox.image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		6
	);

	VulkanUtils::CopyBufferToImage(
		device,
		commandPool,
		graphicsQueue,
		stagingBuffer,
		m_skybox.image,
		regions
	);

	// transition to SHADER READ
	VulkanUtils::TransitionImageLayout(
		device,
		commandPool,
		graphicsQueue,
		m_skybox.image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		6
	);


	//create Image view 
	m_skybox.imageView = VulkanUtils::CreateImageView(
		device,
		m_skybox.image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT,
		6,
		VK_IMAGE_VIEW_TYPE_CUBE
	);


	// creating the sampler
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	// IMPORTANT for skybox
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(
		device,
		&samplerInfo,
		nullptr,
		&m_skybox.sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create skybox sampler!");
	}

	//cleanup staging buffer
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingMemory, nullptr);

	CreateSkyboxGeometry();
}


void Renderer::CreateSkyboxGeometry()
{
	std::vector<glm::vec3> vertices =
	{
		// positions (36 vertices)
	{-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1},
	{ 1, 1, 1}, {-1, 1, 1}, {-1,-1, 1},

	{-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1},
	{ 1, 1,-1}, { 1,-1,-1}, {-1,-1,-1},

	{-1, 1,-1}, {-1, 1, 1}, { 1, 1, 1},
	{ 1, 1, 1}, { 1, 1,-1}, {-1, 1,-1},

	{-1,-1,-1}, { 1,-1,-1}, { 1,-1, 1},
	{ 1,-1, 1}, {-1,-1, 1}, {-1,-1,-1},

	{ 1,-1,-1}, { 1, 1,-1}, { 1, 1, 1},
	{ 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1},

	{-1,-1,-1}, {-1,-1, 1}, {-1, 1, 1},
	{-1, 1, 1}, {-1, 1,-1}, {-1,-1,-1}

	};

	VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

	VulkanUtils::CreateBuffer(
		m_vulkan.GetDevice(),
		m_vulkan.GetPhysicalDevice(),
		size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_skybox.vertexBuffer,
		m_skybox.vertexMemory
	);

	VulkanUtils::UploadData(m_vulkan.GetDevice(), m_skybox.vertexMemory, vertices.data(), size);

	m_skybox.vertexCount = static_cast<uint32_t>(vertices.size());
}

void Renderer::DrawObjects(VkCommandBuffer cmd, const FrameResources& frame, const Scene& scene)
{
	// Bind graphics pipeline
	vkCmdBindPipeline(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_graphicsPipeline
	);

	// Bind global descriptor set (camera UBO)
	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_graphicsPipelineLayout,
		0, // set 0
		1,
		&frame.globalDescriptorSet,
		0,
		nullptr
	);

	// Bind global (camera) descriptor
	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_graphicsPipelineLayout,
		0,
		1,
		&frame.globalDescriptorSet,
		0,
		nullptr
	);

	// Object rendering starts here...

	for (const RenderObject& obj : scene.GetObjects())
	{
		// Update transform (temporary approach)
		const_cast<RenderObject&>(obj).UpdateTransform();

		// Push model matrix
		vkCmdPushConstants(
			cmd,
			m_graphicsPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(glm::mat4),
			&obj.transform
		);

		obj.material->Bind(cmd, m_graphicsPipelineLayout);

		// Bind mesh
		obj.mesh->Bind(cmd);

		// Draw mesh
		obj.mesh->Draw(cmd);
	}


}


void Renderer::DrawSkybox(VkCommandBuffer cmd, const FrameResources& frame)
{
	// Bind skybox pipeline
	vkCmdBindPipeline(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_skybox.pipeline
	);

	// Bind descriptor sets (IMPORTANT: 2 sets)
	VkDescriptorSet sets[] =
	{
		frame.globalDescriptorSet,      // set 0 => camera
		m_skybox.descriptorSet    // set 1 => cubemap
	};

	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_skybox.pipelineLayout,
		0,
		2,
		sets,
		0,
		nullptr
	);

	// Bind vertex buffer (cube)
	VkBuffer vertexBuffers[] = { m_skybox.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

	// Draw cube (36 vertices)
	vkCmdDraw(cmd, m_skybox.vertexCount, 1, 0, 0);
}


