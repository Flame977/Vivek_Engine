#pragma once

#include <string>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/quaternion.hpp"


namespace ECS
{
	struct Name
	{
		std::string name;
	};

	struct Transform
	{
		glm::vec3 position{ 0.0f };
		glm::vec3 rotation{ 0.0f };
		glm::vec3 scale{ 1.0f };

		glm::mat4 GetMatrix() const
		{
			glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 R = glm::toMat4(glm::quat(glm::radians(rotation)));
			glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

			return T * R * S;
		}
	};

	struct MeshRenderer
	{
		Mesh* mesh = nullptr;
	};

	struct MaterialRenderer
	{
		Material* material = nullptr;
	};

	enum class LightType
	{
		Directional = 0,
		Point = 1,
		Spot = 2,

	};

	struct LightComponent
	{
		ECS::LightType type = ECS::LightType::Directional;

		glm::vec3 color{ 1.0f };
		float intensity = 1.0f;

		glm::vec3 direction{ 0.0f, -1.0f, 0.0f }; // for directional
		glm::vec3 position{ 0.0f };               // for point/spot

		// optional (for later)
		float range = 10.0f;
		float innerCone = 0.9f;
		float outerCone = 0.8f;
	};



}




