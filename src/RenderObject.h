#pragma once
#include "Mesh.h"
#include "Material.h"
#include "glm.hpp"
#include "ext/matrix_transform.hpp"
#include "Camera.h"

#include "gtc/matrix_transform.hpp"   // translate, scale
#include "gtc/type_ptr.hpp"           // value_ptr
#include "gtc/quaternion.hpp"         // glm::quat
#include "gtx/quaternion.hpp"         // toMat4, eulerAngles

using Entity = uint32_t;


class RenderObject
{

public:
	std::string name;

	Mesh* mesh;
	Material* material;

	glm::vec3 position{ 0.0f };
	glm::vec3 rotation{ 0.0f };
	glm::vec3 scale{ 1.0f };

	glm::mat4 transform{ 1.0f };

	void SetTransform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl);

	void Destroy();

};
