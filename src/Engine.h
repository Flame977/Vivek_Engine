#pragma once
#include <memory>
#include "Camera.h"
#include "Timer.h"
#include "Window.h"
#include "Shader.h"
#include "VulkanContext.h"
#include "Texture.h"
#include "Logger.h"
#include "Renderer.h"
#include "Scene.h"

#include "ImGuizmo.h"

//#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
		 
#include "gtx/quaternion.hpp"
#include "gtx/matrix_decompose.hpp"


class Engine
{

public:

	Engine();

	~Engine();

	void Run();

private:

	void OnUpdate();

	void OnResize(int width, int height);

	void HandleCameraInput(float deltaTime);

	void DrawImgui();



	Timer m_timer;

	std::unique_ptr<Window> m_window;

	std::unique_ptr<VulkanContext> m_vulkan;

	std::unique_ptr<Renderer> m_renderer;

	std::unique_ptr<Camera> m_camera;

	std::unique_ptr<Scene> m_scene;


	bool m_running = false;


};