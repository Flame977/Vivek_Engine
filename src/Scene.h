#pragma once

#include <unordered_map>
#include <algorithm>
#include "Components.h"
#include "Renderer.h"


class Scene
{
public:


	ECS::Entity CreateEntity();
	void DestroyEntity(ECS::Entity e);
	void RemoveAllEntities(ECS::Entity e);

	void CreatePrimitive(Renderer& renderer, const std::string& meshPath, const std::string& name);
	void CreateCube(Renderer& renderer);
	void CreateSphere(Renderer& renderer);
	void CreateCylinder(Renderer& renderer);
	void CreateCone(Renderer& renderer);
	void CreatePlane(Renderer& renderer);

	void LoadSkybox(Renderer& renderer);
	void SetSkyboxDrawState(bool state);
	//bool& GetSkyboxDrawState();

	// ECS components...

	std::unordered_map<ECS::Entity, ECS::Name> names;
	std::unordered_map<ECS::Entity, ECS::Transform> transforms;
	std::unordered_map<ECS::Entity, ECS::MeshRenderer> meshs;
	std::unordered_map<ECS::Entity, ECS::MaterialRenderer> materials;
	std::unordered_map<ECS::Entity, ECS::LightComponent> lights;

	std::vector<ECS::Entity>& GetEntities();

	bool DrawSkybox = false;


private:

	ECS::Entity m_nextEntity = 0;
	std::vector<ECS::Entity> m_entities;


};