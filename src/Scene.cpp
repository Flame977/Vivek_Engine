#include "Scene.h"


RenderObject& Scene::CreateObject(
	const std::string& name,
	Mesh* mesh,
	Material* material,
	glm::vec3 pos,
	glm::vec3 rot,
	glm::vec3 scale
)
{

	//just adding obj to list
	RenderObject obj = {
		name,
		mesh,
		material,
		pos,
		rot,
		scale };

	obj.SetTransform(pos, rot, scale);

	m_renderObjects.push_back(obj);
	return m_renderObjects.back();
}


void Scene::DestroyLastObject()
{
	//just removing the last object
	if (!m_renderObjects.empty())
		m_renderObjects.pop_back();
}

const std::vector<RenderObject>& Scene::GetObjects() const
{
	return m_renderObjects;
}

std::vector<RenderObject>& Scene::GetObjects()
{
	return m_renderObjects;
}

Entity Scene::CreateEntity()
{
	Entity e = m_nextEntity++;
	m_entities.push_back(e);
	return e;
}

void Scene::DestroyEntity(Entity e)
{
	// Remove from entity list
	m_entities.erase(
		std::remove(m_entities.begin(), m_entities.end(), e),
		m_entities.end()
	);

	RemoveAllEntities(e);
}

void Scene::RemoveAllEntities(Entity e)
{
	names.erase(e);
	transforms.erase(e);
	meshs.erase(e);
	materials.erase(e);
	lights.erase(e);
	
}



std::vector<Entity>& Scene::GetEntities()
{
	return m_entities;
}






