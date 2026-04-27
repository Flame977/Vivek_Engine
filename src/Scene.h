#pragma once

#include "RenderObject.h"
#include <unordered_map>
#include <algorithm>
#include "Components.h"

using Entity = uint32_t;

class Scene
{
public:

	RenderObject& CreateObject(
		const std::string& name,
		Mesh* mesh,
		Material* material,
		glm::vec3 pos,
		glm::vec3 rot,
		glm::vec3 scale
	);

	void DestroyLastObject();

	const std::vector<RenderObject>& GetObjects() const;

	std::vector<RenderObject>& GetObjects();



	Entity CreateEntity();
	void DestroyEntity(Entity e);
	void RemoveAllEntities(Entity e);


	// ECS components...
	
	std::unordered_map<Entity, ECS::Name> names;
	std::unordered_map<Entity, ECS::Transform> transforms;
	std::unordered_map<Entity, ECS::MeshRenderer> meshs;
	std::unordered_map<Entity, ECS::MaterialRenderer> materials;
	std::unordered_map<Entity, ECS::LightComponent> lights;

	std::vector<Entity>& GetEntities();

private:

	std::vector<RenderObject> m_renderObjects;

	Entity m_nextEntity = 0;
	std::vector<Entity> m_entities;

};