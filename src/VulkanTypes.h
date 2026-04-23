#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include "glm.hpp"

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


struct PushConstant
{
	glm::mat4 model;
};

#define MAX_LIGHTS 16

enum LightType
{
	Directional = 0,
	Point = 1,
	Spot = 2,
};

struct alignas(16) Light
{
	glm::vec4 position;
	glm::vec4 direction;
	glm::vec4 color;
	glm::vec4 params;
};
	
struct alignas(16) CameraUBO
{
	glm::mat4 view;
	glm::mat4 proj;

	glm::ivec4 lightInfo;

	Light lights[MAX_LIGHTS];
};


// implement this later...
//we need this to create any kind of pipeline we need
struct PipelineConfig
{
	std::vector<char> vertShaderCode;
	std::vector<char> fragShaderCode;

	//later, topology, culling, blending, etc...

};


// implement this later...
// This one is for the materials that will be setup later...
struct MaterialUBO
{
	glm::vec4 baseColor;   // tint
	float tiling;
	float roughness;       // future use
	float metallic;        // future use
	float padding;         // std140 alignment
};



