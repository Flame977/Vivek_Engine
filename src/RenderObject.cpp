#include "RenderObject.h"

void RenderObject::SetTransform(
	const glm::vec3& pos,
	const glm::vec3& rot,
	const glm::vec3& scl)
{
	position = pos;
	rotation = rot;
	scale = scl;

	glm::mat4 T = glm::translate(glm::mat4(1.0f), position);

	glm::quat q = glm::quat(glm::radians(rotation));
	glm::mat4 R = glm::toMat4(q);

	glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

	transform = T * R * S;
}


void RenderObject::Destroy()
{
	if (mesh != nullptr)
		mesh->Destroy();

	if (material != nullptr)
		material->Destroy();

}