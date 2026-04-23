#pragma once
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"

class Camera
{
public:
	Camera(float fov, float aspect, float nearPlane, float farPlane);

	
    //Input
    void ProcessKeyboard(int key, float deltaTime);
    void ProcessMouse(float xOffset, float yOffset);
    void SetMoveSpeed(float speed);

	const glm::mat4& GetView() const;
	const glm::mat4& GetProjection() const;

    void SetAspect(float aspect);

private:
	void RecalculateView();
	void RecalculateProjection();

private:
    glm::vec3 m_position{ 0.0f, 0.0f, 5.0f };
    glm::vec3 m_front{ 0.0f, 0.0f, -1.0f };
    glm::vec3 m_up{ 0.0f, 1.0f, 0.0f };
    glm::vec3 m_right{ 1.0f, 0.0f, 0.0f };

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;

    float m_moveSpeed = 5.0f;
    float m_mouseSensitivity = 0.1f;

    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;

    glm::mat4 m_view{ 1.0f };
    glm::mat4 m_projection{ 1.0f };
};