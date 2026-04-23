#pragma once
#include <vulkan/vulkan_core.h>
#include <array>
#include "glm.hpp"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	//glm::vec4 tangent;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription binding{};
		binding.binding = 0;
		binding.stride = sizeof(Vertex);
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributes{};

		// position
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[0].offset = offsetof(Vertex, position);

		// normal
		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[1].offset = offsetof(Vertex, normal);

		// for textures
		// uv
		attributes[2].binding = 0;
		attributes[2].location = 2;
		attributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[2].offset = offsetof(Vertex, uv);
		
		// this if for normals and stuff
		// tangents
		/*attributes[3].binding = 0;
		attributes[3].location = 3;
		attributes[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[3].offset = offsetof(Vertex, tangent);*/

		return attributes;
	}



};