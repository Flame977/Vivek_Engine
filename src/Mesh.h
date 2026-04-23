#pragma once
#include <vulkan/vulkan.h>
#include "assimp/Importer.hpp"

class Mesh
{
public:

	Mesh() = default;
	//~Mesh();

	void Destroy();   //this will be called in the vulkan destructor...

	// GPU resources should not be copied
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	// Allow move semantics (very important)
	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	void Bind(VkCommandBuffer cmd) const;
	void Draw(VkCommandBuffer cmd) const;

	uint32_t GetIndexCount() const { return m_indexCount; }



	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

	VkBuffer m_indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

	uint32_t m_indexCount = 0;

	VkDevice m_device = VK_NULL_HANDLE;
};