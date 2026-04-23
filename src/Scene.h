#pragma once

#include"RenderObject.h"

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

private:

	std::vector<RenderObject> m_renderObjects;

};