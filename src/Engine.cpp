#include <iostream>
#include "Engine.h"

Engine::Engine()
{
	// Automatic shader compilation!
	Shader::CompileShaders();

	m_window = std::make_unique<Window>(1280, 768, "Vivek Engine");
	m_vulkan = std::make_unique<VulkanContext>(*m_window);

	// Renderer
	m_renderer = std::make_unique<Renderer>(*m_vulkan);
	m_camera = std::make_unique<Camera>(60, m_window->GetWidth() / m_window->GetHeight(), 0.1f, 100);

	// scene instance...
	m_scene = std::make_unique<Scene>();

	Mesh* mesh = m_renderer->LoadMesh("Assets/Models/ak47.fbx");
	Material* mat = m_renderer->LoadMaterial("Assets/Textures/ak47_albedo.png");

	m_scene->CreateObject("ak1", mesh, mat, { 0,0,-3 }, { -90,0,0 }, { 1,1,1 });
	//m_scene->CreateObject("ak2", mesh, mat, { 3,0,-3 }, { -90,0,0 }, { 1,1,1 });


	float aspect = m_vulkan->GetAspectRatio();
	m_camera->SetAspect(aspect);

	m_vulkan->SetImguiDrawCallback([this]()
		{
			this->DrawImgui();
		});

	m_running = true;
}

Engine::~Engine() = default;

void Engine::Run()
{
	while (m_window->IsOpen() && m_running)
	{
		m_timer.Tick();
		m_window->PollEvents();
		m_window->UpdateInput();

		// when window is minimized
		if (m_window->GetWidth() == 0 || m_window->GetHeight() == 0)
		{
			glfwWaitEvents();
			continue;
		}

		if (m_window->WasResized())
		{
			OnResize(m_window->GetWidth(), m_window->GetHeight());

			m_window->ResetResizeFlag();
		}

		OnUpdate();
	}

	m_renderer->WaitIdle();
}

//random variables for testing behaviour...
float m_rotationAngle = 0.0f;
float xPos = 6.0f;
float y = 0.0f;

void Engine::OnUpdate()
{
	float dt = m_timer.GetDeltaTime();

	double x, y;
	m_window->GetMousePos(x, y);

	HandleCameraInput(dt);

	m_renderer->SetFPS(1 / dt);

	// Increase rotation
	m_rotationAngle += dt * 45.0f; // degrees/sec


	//creating objects at runtime...
	if (m_window->IsKeyDown(GLFW_KEY_SPACE))
	{
		Mesh* akMesh = m_renderer->LoadMesh("Assets/Models/ak47.fbx");
		Material* akMat = m_renderer->LoadMaterial("Assets/Textures/ak47_albedo.png");

		//number
		auto num = m_renderer->GetRenderObjects(*m_scene).size() + 1;
		std::string index = std::to_string(num);

		m_renderer->CreateRenderObject(*m_scene, "new_ak_" + index, akMesh, akMat, { xPos,0,-3 }, { -90,0,0 }, { 1,1,1 });
		xPos += 3.0f;
	}

	//obj deletion code...
	if (m_window->IsKeyDown(GLFW_KEY_DELETE))
	{
		auto& objects = m_renderer->GetRenderObjects(*m_scene);

		if (!objects.empty())
		{
			RenderObject* current = &objects.back();
			m_renderer->DestroyRenderObject(*m_scene, current);
			xPos -= 3.0f;
		}
		else
		{
			std::cout << "render objs are empty?" << std::endl;
		}
	}


	// Some object rotation code...
	float speed = 45.0f;
	if (m_window->IsKey(GLFW_KEY_RIGHT))
	{
		auto& objects = m_renderer->GetRenderObjects(*m_scene);
		if (!objects.empty())
		{
			RenderObject& current = objects.back();

			Log("rotating right...");
			current.rotation.y -= speed * dt;
		}
	}

	if (m_window->IsKey(GLFW_KEY_LEFT))
	{
		auto& objects = m_renderer->GetRenderObjects(*m_scene);
		if (!objects.empty())
		{
			RenderObject& current = objects.back();

			Log("rotating right...");
			current.rotation.y += speed * dt;
		}
	}

	m_renderer->DrawFrame(*m_camera, *m_scene);
}

void Engine::OnResize(int width, int height)
{
	std::cout << "Window Resized: " << m_window->GetWidth() << " x " << m_window->GetHeight() << "\n";

	int fbWidth = 0, fbHeight = 0;

	// when window minimized it becomes 0
	while (fbWidth == 0 || fbHeight == 0)
	{
		glfwGetFramebufferSize(m_window->GetWindow(), &fbWidth, &fbHeight);
		glfwWaitEvents();
	}

	float aspect =
		static_cast<float>(fbWidth) /
		static_cast<float>(fbHeight);

	m_camera->SetAspect(aspect);

	//m_vulkan->RecreateSwapchain();
	m_renderer->OnResize();
}

void Engine::HandleCameraInput(float deltaTime)
{

	// -------- Keyboard movement (WASD) --------
	if (m_window->IsKey(GLFW_KEY_W))
		m_camera->ProcessKeyboard(GLFW_KEY_W, deltaTime);

	if (m_window->IsKey(GLFW_KEY_S))
		m_camera->ProcessKeyboard(GLFW_KEY_S, deltaTime);

	if (m_window->IsKey(GLFW_KEY_A))
		m_camera->ProcessKeyboard(GLFW_KEY_A, deltaTime);

	if (m_window->IsKey(GLFW_KEY_D))
		m_camera->ProcessKeyboard(GLFW_KEY_D, deltaTime);

	if (m_window->IsKey(GLFW_KEY_E))
		m_camera->ProcessKeyboard(GLFW_KEY_E, deltaTime);

	if (m_window->IsKey(GLFW_KEY_Q))
		m_camera->ProcessKeyboard(GLFW_KEY_Q, deltaTime);


	// -------- Speed boost (Shift like Unity) --------
	if (m_window->IsKey(GLFW_KEY_LEFT_SHIFT))
		m_camera->SetMoveSpeed(10.0f);
	else
		m_camera->SetMoveSpeed(5.0f);

	// -------- Mouse look (Right Mouse Button) --------
	static bool firstMouse = true;
	static double lastX = 0.0;
	static double lastY = 0.0;

	if (m_window->GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
	{
		m_window->SetCursorMode(false);

		double xpos, ypos;
		m_window->GetMousePos(xpos, ypos);

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xOffset = static_cast<float>(xpos - lastX);
		float yOffset = static_cast<float>(lastY - ypos); // inverted Y

		lastX = xpos;
		lastY = ypos;

		m_camera->ProcessMouse(xOffset, yOffset);
	}
	else
	{
		m_window->SetCursorMode(true);
		firstMouse = true;
	}
}


void Engine::DrawImgui()
{

	//imguizmo stuff here...
	ImGuizmo::BeginFrame();
	ImGuizmo::SetRect(0,0, 
		m_window->GetWidth(),
		m_window->GetHeight()
	);

	const glm::mat4& view = m_camera->GetView();
	const glm::mat4& proj = m_camera->GetProjection();

	


	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(ImVec2(
		viewport->Pos.x + viewport->Size.x - 400,
		viewport->Pos.y));
	ImGui::SetNextWindowSize(ImVec2(400, viewport->Size.y));

	ImGui::Begin("Vivek Engine");

	ImGui::Text("FPS:: %.2f", m_renderer->GetFPS());

	auto& objects = m_renderer->GetRenderObjects(*m_scene);

	//adding all objects as a text field
	for (uint32_t i = 0; i < objects.size(); i++)
	{
		auto obj = objects[i];

		ImGui::Text("Render Obj:: %s", objects[i].name.c_str());

		ImGuizmo::Manipulate(
			glm::value_ptr(view),
			glm::value_ptr(proj),
			ImGuizmo::TRANSLATE,   // or ROTATE / SCALE
			ImGuizmo::LOCAL,       // or WORLD
			glm::value_ptr(obj.transform)
		);

	}

	ImGui::End();
}

