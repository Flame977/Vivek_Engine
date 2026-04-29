#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <iostream>
#include <stdexcept>
#include <array>
#include <memory>
#include "Window.h"
#include "VulkanUtils.h"
#include "DescriptorManager.h"
#include "FrameResources.h"
#include "Vertex.h"
#include "Texture.h"
#include "PipelineBuilder.h"
#include "Material.h"
#include "MeshData.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "ext/matrix_transform.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "Logger.h"
#include <functional>
#include "Skybox.h"




const std::vector<const char*> validationLayers =
{ "VK_LAYER_KHRONOS_validation" };

// for debugging !!!
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData)
{
	std::cerr << "Validation: " << callbackData->pMessage << std::endl << std::endl;
	return VK_FALSE;
}

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;


class VulkanContext
{
public:

	VulkanContext(Window& window);

	~VulkanContext();

	// this function was used previously, now everything is being handled by the renderer
	void DrawFrame();

	//draw frame will be broken down into multiple functions for better usage
	
	void BeginFrame(uint32_t& imageIndex, FrameResources*& outFrame);

	VkCommandBuffer GetCommandBuffer(uint32_t imageIndex);

	void EndFrame(uint32_t& imageIndex, FrameResources& frame);

	void BeginRenderPass(VkCommandBuffer cmd, uint32_t imageIndex);

	void EndRenderPass(VkCommandBuffer cmd);


	void WaitIdle() const;

	void RecreateSwapchain();

	void UpdateUniforms(CameraUBO& camera);

	VkDevice GetDevice() const;

	VkPhysicalDevice GetPhysicalDevice() const;

	VkCommandPool GetCommandPool() const;

	VkQueue GetGraphicsQueue() const;

	VkRenderPass GetRenderPass() const;

	VkExtent2D GetSwapchainExtent() const;

	float GetAspectRatio() const;

	void SetDisplayFps(float fps);

	float GetFps() const;

	VkPipeline GetGraphicsPipeline() const;

	VkPipelineLayout GetGraphicsPipelineLayout() const;

	FrameResources& GetFrame(uint32_t index);

	DescriptorManager& GetDescriptorManager();

	VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout layout);

private:

	void InitVulkan();

	void InitImGui();

	void ShutdownImGui();

	// New descriptors stuff more cleaner..
	void CreateDescriptorPool();

	void CreatePerFrameResources();

	void CreateInstance();

	void SetupDebugMessenger();

	void CreateSurface();

	void PickPhysicalDevice();

	bool IsDeviceSuitable(VkPhysicalDevice device) const;

	void CreateLogicalDevice();

	void CreateSwapchain();

	void CreateSwapchainImageViews();

	void CreateDepthResources();

	void CreateRenderPass();

	void CreateCommandPool();

	void CreatePipelines();

	void CleanupPipelines();

	void CreateGraphicsPipeline();

	void CreateFramebuffers();

	void CreateCommandBuffers();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const FrameResources& frame);

	void CreateSyncObjects();

	void UpdateUniformBuffer(uint32_t frameIndex, const CameraUBO& camera);

	void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

	void CleanupSwapchain();

	void CleaupRenderPass();

	void CleanupFrameResources();

	void DestroyDebugMessenger();

	void CreateDeviceBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& memory);

	// IMGUI stuff...

	void BeginImGuiFrame();


public:

	void DrawImGuiUI();

	void RenderImGuiUI(VkCommandBuffer commandBuffer);

	void SetImguiDrawCallback(std::function<void()> callback);


private:

	VkInstance m_instance = VK_NULL_HANDLE;

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;

	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	size_t m_objectAlignedSize;

	VkDevice m_device = VK_NULL_HANDLE;

	VkQueue m_graphicsQueue = VK_NULL_HANDLE;

	uint32_t m_graphicsQueueFamilyIndex = 0;

	VkQueue m_presentQueue = VK_NULL_HANDLE;

	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

	VkFormat m_swapchainImageFormat;
	std::vector<VkImage> m_swapchainImages;
	VkExtent2D m_swapchainExtent;
	std::vector<VkImageView> m_swapchainImageViews;


	VkFormat m_depthFormat;
	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;

	VkRenderPass m_renderPass = VK_NULL_HANDLE;

	VkPipelineLayout m_graphicsPipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;


	std::vector<VkFramebuffer> m_swapchainFramebuffers;

	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> m_commandBuffers;


	std::array<FrameResources, MAX_FRAMES_IN_FLIGHT> m_frames;
	uint32_t m_currentFrame = 0;

	DescriptorManager m_descriptorManager;

	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

	VkDescriptorPool m_imguiDescriptorPool;


	// Triangle
	//const std::vector<Vertex> vertices = {
	//{{ 0.0f, 0.5f, 0.0f }, {1.0f, 0.0f, 0.0f}},  //red up
	//{{ -0.5f,  -0.5f, 0.0f }, {0.0f, 1.0f, 0.0f}},  //green right 
	//{{0.5f,  -0.5f, 0.0f }, {0.0f, 0.0f, 1.0f}},  //blue left
	//};


	//not using anymore, as we will be using proper meshes
	/*

	//with UVs
	const std::vector<Vertex> vertices = {
		// FRONT (+Z)
		{{-0.5f,-0.5f, 0.5f},{1,0,0},{0.0f,0.0f}},
		{{-0.5f, 0.5f, 0.5f},{1,0,0},{0.0f,1.0f}},
		{{ 0.5f, 0.5f, 0.5f},{1,0,0},{1.0f,1.0f}},
		{{ 0.5f,-0.5f, 0.5f},{1,0,0},{1.0f,0.0f}},

		// BACK (-Z)
		{{ 0.5f,-0.5f,-0.5f},{0,1,0},{0.0f,0.0f}},
		{{ 0.5f, 0.5f,-0.5f},{0,1,0},{0.0f,1.0f}},
		{{-0.5f, 0.5f,-0.5f},{0,1,0},{1.0f,1.0f}},
		{{-0.5f,-0.5f,-0.5f},{0,1,0},{1.0f,0.0f}},

		// LEFT (-X)
		{{-0.5f,-0.5f,-0.5f},{0,0,1},{0.0f,0.0f}},
		{{-0.5f, 0.5f,-0.5f},{0,0,1},{0.0f,1.0f}},
		{{-0.5f, 0.5f, 0.5f},{0,0,1},{1.0f,1.0f}},
		{{-0.5f,-0.5f, 0.5f},{0,0,1},{1.0f,0.0f}},

		// RIGHT (+X)
		{{ 0.5f,-0.5f, 0.5f},{1,1,0},{0.0f,0.0f}},
		{{ 0.5f, 0.5f, 0.5f},{1,1,0},{0.0f,1.0f}},
		{{ 0.5f, 0.5f,-0.5f},{1,1,0},{1.0f,1.0f}},
		{{ 0.5f,-0.5f,-0.5f},{1,1,0},{1.0f,0.0f}},

		// TOP (+Y)
		{{-0.5f, 0.5f, 0.5f},{1,0,1},{0.0f,0.0f}},
		{{-0.5f, 0.5f,-0.5f},{1,0,1},{0.0f,1.0f}},
		{{ 0.5f, 0.5f,-0.5f},{1,0,1},{1.0f,1.0f}},
		{{ 0.5f, 0.5f, 0.5f},{1,0,1},{1.0f,0.0f}},

		// BOTTOM (-Y)
		{{-0.5f,-0.5f,-0.5f},{0,1,1},{1.0f,0.0f}},
		{{-0.5f,-0.5f, 0.5f},{0,1,1},{1.0f,1.0f}},
		{{ 0.5f,-0.5f, 0.5f},{0,1,1},{0.0f,1.0f}},
		{{ 0.5f,-0.5f,-0.5f},{0,1,1},{0.0f,0.0f}},
	};

	const std::vector<uint32_t> indices = {
		 0,  1,  2,  2,  3,  0,   // front
		 4,  5,  6,  6,  7,  4,   // back
		 8,  9, 10, 10, 11,  8,   // left
		12, 13, 14, 14, 15, 12,   // right
		16, 17, 18, 18, 19, 16,   // top
		20, 21, 22, 22, 23, 20    // bottom
	};

	*/

	//all of this needs to be go in the mesh class
/*
	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

	VkBuffer m_indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;
*/


	Window& m_window;

	float m_displayFPS;

	std::function<void()> m_drawImguiCallback;

};