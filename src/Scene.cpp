#include "Scene.h"

ECS::Entity Scene::CreateEntity()
{
	ECS::Entity e = m_nextEntity++;
	m_entities.push_back(e);
	return e;
}

void Scene::DestroyEntity(ECS::Entity e)
{
	// Remove from entity list
	m_entities.erase(
		std::remove(m_entities.begin(), m_entities.end(), e),
		m_entities.end()
	);

	RemoveAllEntities(e);
}

void Scene::RemoveAllEntities(ECS::Entity e)
{
	names.erase(e);
	transforms.erase(e);
	meshs.erase(e);
	materials.erase(e);
	lights.erase(e);

}

void Scene::CreatePrimitive(Renderer& renderer, const std::string& meshPath, const std::string& name)
{
	auto m = renderer.LoadMesh(meshPath);
	auto mat = renderer.LoadDefaultMaterial();

	auto p = this->CreateEntity();
	this->names[p] = ECS::Name{ name };
	this->transforms[p] = ECS::Transform{
	{0, 0, 0},
	{0, 0, 0},
	{1, 1, 1} };

	this->meshs[p] = ECS::MeshRenderer{ m };
	this->materials[p] = ECS::MaterialRenderer{ mat };
}

void Scene::CreateCube(Renderer& renderer)
{
	CreatePrimitive(renderer, "Assets/Models/Primitives/cube.fbx", "Cube");
}

void Scene::CreateSphere(Renderer& renderer)
{
	CreatePrimitive(renderer, "Assets/Models/Primitives/sphere.fbx", "Sphere");
}

void Scene::CreateCylinder(Renderer& renderer)
{
	CreatePrimitive(renderer, "Assets/Models/Primitives/cylinder.fbx", "Cylinder");
}

void Scene::CreateCone(Renderer& renderer)
{
	CreatePrimitive(renderer, "Assets/Models/Primitives/cone.fbx", "Cone");
}

void Scene::CreatePlane(Renderer& renderer)
{
	CreatePrimitive(renderer, "Assets/Models/Primitives/plane.fbx", "Plane");
}

void Scene::LoadSkybox(Renderer& renderer)
{
	renderer.CreateSkybox();
	renderer.CreateSkyboxDescriptors();
	SetSkyboxDrawState(true);
}

void Scene::SetSkyboxDrawState(bool state)
{
	DrawSkybox = state;
}


std::vector<ECS::Entity>& Scene::GetEntities()
{
	return m_entities;
}






