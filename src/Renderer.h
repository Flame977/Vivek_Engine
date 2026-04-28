#pragma once
#include <string>
#include <vector>
#include <memory>
#include "glm.hpp"

#include <vulkan/vulkan.h>
#include "FrameResources.h"
#include "Skybox.h"
#include "Texture.h"
#include "MeshData.h"
#include "MeshLoader.h"

class VulkanContext;
class Camera;
class Mesh;
class Material;
class RenderObject;
class Scene;

class Renderer
{
public:

	Renderer(VulkanContext& vulkan);

	~Renderer();

	void InitRenderer();

	void OnResize();

	void WaitIdle();

	void DrawFrame(const Camera& camera, const Scene& scene);

	Mesh* LoadMesh(const std::string& path);

	Material* LoadMaterial(const std::string& path);

	Material* LoadDefaultMaterial();

	Mesh CreateMesh(const MeshData& data);

	Material CreateMaterial(const std::string& texPath);

	RenderObject* CreateRenderObject(
		Scene& scene,
		const std::string& name,
		Mesh* mesh,
		Material* material,
		glm::vec3 pos,
		glm::vec3 rot,
		glm::vec3 scale
	);

	void DestroyRenderObject(Scene& scene, RenderObject* obj);

	std::vector<RenderObject>& GetRenderObjects(Scene& scene);

	void SetFPS(float fps);

	float GetFPS() const;

	void RecordCommands(
		VkCommandBuffer cmd,
		uint32_t imageIndex,
		const FrameResources& frame,
		const Scene& scene);

	// Expose layouts to other systems
	VkDescriptorSetLayout GetFrameLayout() const { return m_frameDescriptorSetLayout; }
	VkDescriptorSetLayout GetMaterialLayout() const { return m_materialDescriptorSetLayout; }

private:

	void CreateDescriptorLayouts();

	void CreateGraphicsPipeline();

	void CreateFrameDescriptors();

	void CreateSkyboxDescriptors();

	void Cleanup();

	void CreatePipelines();

	void CleanupPipelines(VkDevice device);

	void CreateSkyboxPipeline();

	void CreateSkybox();

	void CreateSkyboxGeometry();

	void DrawObjects(VkCommandBuffer cmd, const FrameResources& frame,const Scene& scene);

	void DrawSkybox(VkCommandBuffer cmd, const FrameResources& frame);

private:

	VulkanContext& m_vulkan;

	VkDescriptorSetLayout m_frameDescriptorSetLayout;    // set 0
	VkDescriptorSetLayout m_materialDescriptorSetLayout; // set 1

	//renderer owns pipelines from now on
	VkPipelineLayout m_graphicsPipelineLayout;
	VkPipeline m_graphicsPipeline;

	//vulkan specific stuff for making the skybox...
	VulkanSkybox m_skybox;

	//raw resources...
	std::vector<std::unique_ptr<Mesh>> m_meshes;
	std::vector<std::unique_ptr<Material>> m_materials;

};