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

ECS::Entity Scene::CreateEmptyEntity(const std::string& name)
{
	auto e = this->CreateEntity();
	auto unique = MakeUniqueName(name);
	this->transforms[e] = ECS::Transform{
	{0, 0, 0},
	{0, 0, 0},
	{1, 1, 1} };
	return e;
}

ECS::Entity Scene::CreatePrimitive(Renderer& renderer, const std::string& meshPath, const std::string& name)
{
	auto m = renderer.LoadMesh(meshPath);
	auto mat = renderer.LoadDefaultMaterial();

	auto e = this->CreateEntity();

	auto uniqueName = MakeUniqueName(name);

	this->names[e] = ECS::Name{ uniqueName };
	this->transforms[e] = ECS::Transform{
	{0, 0, 0},
	{0, 0, 0},
	{1, 1, 1} };

	this->meshs[e] = ECS::MeshRenderer{ m };
	this->materials[e] = ECS::MaterialRenderer{ mat };
	return e;
}

ECS::Entity Scene::CreateCube(Renderer& renderer, const std::string& name)
{
	return CreatePrimitive(renderer, "Assets/Models/Primitives/cube.fbx", name);
}

ECS::Entity Scene::CreateSphere(Renderer& renderer, const std::string& name)
{
	return CreatePrimitive(renderer, "Assets/Models/Primitives/sphere.fbx", name);
}

ECS::Entity Scene::CreateCylinder(Renderer& renderer, const std::string& name)
{
	return CreatePrimitive(renderer, "Assets/Models/Primitives/cylinder.fbx", name);
}

ECS::Entity Scene::CreateCone(Renderer& renderer, const std::string& name)
{
	return CreatePrimitive(renderer, "Assets/Models/Primitives/cone.fbx", name);
}

ECS::Entity Scene::CreatePlane(Renderer& renderer, const std::string& name)
{
	return CreatePrimitive(renderer, "Assets/Models/Primitives/plane.fbx", name);
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

ECS::Entity Scene::CreateDirectionalLight(Renderer& renderer)
{
	ECS::Entity sunlight = this->CreateEntity();

	this->names[sunlight] = { "Directional_Light" };
	this->transforms[sunlight] = ECS::Transform{
	{0, 0, 0},
	{0, 0, 0},
	{1, 1, 1} };


	this->lights[sunlight] = {
		ECS::LightType::Directional,
		{1.0f, 1.0f, 1.0f}, // color
		1.0f,               // intensity
		{0.0f, 0.0f, 0.0f},  // rotation
		{0.0f, 0.0f, 0.0f}  // position
	};

	return sunlight;
}

ECS::Entity Scene::CreatePointLight(Renderer& renderer)
{
	ECS::Entity point = this->CreateEntity();

	this->names[point] = { "Point_Light" };
	this->transforms[point] = ECS::Transform{
	{0, 0, 0},
	{0, 0, 0},
	{1, 1, 1} };

	//need to make it so that the transforms position matches that of the spot light pos

	this->lights[point] = {
		ECS::LightType::Point,
		{1.0f, 1.0f, 1.0f}, // color
		3.0f,               // intensity
		{0.0f, 0.0f, 0.0f}, // direction
		{0.0f, 0.0f, 0.0f}, // pos
		5

	};

	return point;
}


ECS::Entity Scene::CreateSpotLight(Renderer& renderer)
{
	ECS::Entity spot = this->CreateEntity();

	this->names[spot] = { "Spot_Light" };
	this->transforms[spot] = ECS::Transform{
	{0, 0, 0},
	{0, 0, 0},
	{1, 1, 1} };


	this->lights[spot] = {
		ECS::LightType::Spot,
		{1.0f, 0.0f, 0.0f}, // color
		10.0f,               // intensity
		{0.0f, -1.0f, 0.0f}, // direction

		//replace with transform pos
		{0.0f, 3.0f, -3.0f}, // position

		10.0f,				// range
		12.9f,				// inner
		17.8f				// outer
	};

	return spot;
}


void Scene::SetTransform(ECS::Entity& entity, ECS::Transform transform)
{
	auto& t = transforms.at(entity);
	t = transform;
}

void Scene::SetPosition(ECS::Entity& entity, glm::vec3 pos)
{
	auto& t = transforms.at(entity);
	t.position = pos;
}

void Scene::SetRotation(ECS::Entity& entity, glm::vec3 rot)
{
	auto& t = transforms.at(entity);
	t.rotation = rot;
}

void Scene::SetScale(ECS::Entity& entity, glm::vec3 scale)
{
	auto& t = transforms.at(entity);
	t.scale = scale;
}


std::vector<ECS::Entity>& Scene::GetEntities()
{
	return m_entities;
}

std::string Scene::MakeUniqueName(const std::string& base)
{
	std::string name = base;
	int counter = 1;

	while (true)
	{
		bool exists = false;

		for (auto& [e, n] : names)
		{
			if (n.name == name)
			{
				exists = true;
				break;
			}
		}

		if (!exists)
			return name;

		name = base + "_" + std::to_string(counter++);
	}
}






