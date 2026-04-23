#include "Camera.h"


Camera::Camera(float fov, float aspect, float nearPlane, float farPlane)
	: m_fov(fov), m_aspect(aspect), m_near(nearPlane), m_far(farPlane)
{
	RecalculateProjection();
	RecalculateView();
}

void Camera::RecalculateView()
{
	glm::vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	m_front = glm::normalize(front);

	m_right = glm::normalize(
		glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f)));

	m_up = glm::normalize(
		glm::cross(m_right, m_front));

	m_view = glm::lookAt(
		m_position,
		m_position + m_front,
		m_up);
}


void Camera::RecalculateProjection()
{
	m_projection = glm::perspective(
		glm::radians(m_fov),
		m_aspect,
		m_near,
		m_far);

	// Vulkan clip space fix
	m_projection[1][1] *= -1;
}

const glm::mat4& Camera::GetView() const { return m_view; }
const glm::mat4& Camera::GetProjection() const { return m_projection; }

void Camera::SetAspect(float aspect)
{
	m_aspect = aspect;
	RecalculateProjection();
}

void Camera::SetMoveSpeed(float speed)
{
	m_moveSpeed = speed;
}

void Camera::ProcessKeyboard(int key, float deltaTime)
{
	float velocity = m_moveSpeed * deltaTime;

	if (key == GLFW_KEY_W)
		m_position += m_front * velocity;

	if (key == GLFW_KEY_S)
		m_position -= m_front * velocity;

	if (key == GLFW_KEY_A)
		m_position -= m_right * velocity;

	if (key == GLFW_KEY_D)
		m_position += m_right * velocity;

	if (key == GLFW_KEY_E)
		m_position += m_up * velocity;

	if (key == GLFW_KEY_Q)
		m_position -= m_up * velocity;


	RecalculateView();
}

void Camera::ProcessMouse(float xOffset, float yOffset)
{
	xOffset *= m_mouseSensitivity;
	yOffset *= m_mouseSensitivity;

	m_yaw += xOffset;
	m_pitch += yOffset;

	// Prevent gimbal lock (Unity-style clamp)
	m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

	RecalculateView();
}
