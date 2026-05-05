#pragma once

#include <unordered_map>
#include <algorithm>
#include "Components.h"
#include "Renderer.h"
#include <functional>


class Scene
{
public:


	ECS::Entity CreateEntity();
	void DestroyEntity(ECS::Entity e);
	void RemoveAllEntities(ECS::Entity e);

	ECS::Entity CreateEmptyEntity(const std::string& name);

	ECS::Entity CreatePrimitive(Renderer& renderer, const std::string& meshPath, const std::string& name);
	ECS::Entity CreateCube(Renderer& renderer, const std::string& name = "Cube");
	ECS::Entity CreateSphere(Renderer& renderer, const std::string& name = "Sphere");
	ECS::Entity CreateCylinder(Renderer& renderer, const std::string& name = "Cylinder");
	ECS::Entity CreateCone(Renderer& renderer, const std::string& name = "Cone");
	ECS::Entity CreatePlane(Renderer& renderer, const std::string& name = "Plane");

	void LoadSkybox(Renderer& renderer);
	void SetSkyboxDrawState(bool state);


	ECS::Entity CreateDirectionalLight(Renderer& renderer);
	ECS::Entity CreatePointLight(Renderer& renderer);
	ECS::Entity CreateSpotLight(Renderer& renderer);



	// ECS components...

	std::unordered_map<ECS::Entity, ECS::Name> names;
	std::unordered_map<ECS::Entity, ECS::Transform> transforms;
	std::unordered_map<ECS::Entity, ECS::MeshRenderer> meshs;
	std::unordered_map<ECS::Entity, ECS::MaterialRenderer> materials;
	std::unordered_map<ECS::Entity, ECS::LightComponent> lights;


	void SetTransform(ECS::Entity& entity, ECS::Transform transform);

	void SetPosition(ECS::Entity& entity, glm::vec3 pos);

	void SetRotation(ECS::Entity& entity, glm::vec3 pos);

	void SetScale(ECS::Entity& entity, glm::vec3 pos);

	std::vector<ECS::Entity>& GetEntities();

	std::string MakeUniqueName(const std::string& base);



	bool DrawSkybox = false;


private:

	ECS::Entity m_nextEntity = 0;
	std::vector<ECS::Entity> m_entities;


};