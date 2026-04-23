#pragma once
#include "Mesh.h"
#include "Material.h"
#include "glm.hpp"
#include "ext/matrix_transform.hpp"


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


	static glm::mat4 BuildTransformMatrix(
		const glm::vec3& position,
		const glm::vec3& rotationDeg,
		const glm::vec3& scale)
	{
		// Translation
		glm::mat4 T = glm::translate(glm::mat4(1.0f), position);

		// Rotation (degrees -> radians)
		glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(rotationDeg.x), glm::vec3(1, 0, 0));
		glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), glm::radians(rotationDeg.y), glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), glm::radians(rotationDeg.z), glm::vec3(0, 0, 1));

		// Scale
		glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

		// Order matters: Scale -> Rotate -> Translate
		return T * Rz * Ry * Rx * S;
	}

	void UpdateTransform()
	{
		transform = BuildTransformMatrix(position, rotation, scale);
	}

	void RotateTransform(glm::vec3& rotation)
	{
		transform = BuildTransformMatrix(position, rotation, scale);
	}

	void Destroy()
	{
		mesh->Destroy();
		material->Destroy();
	}

};
