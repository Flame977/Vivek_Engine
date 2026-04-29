#include <iostream>
#include "Engine.h"

Engine::Engine()
{
	// Automatic shader compilation!
	Shader::CompileShaders();

	m_window = std::make_unique<Window>(1600, 900, "Vivek Engine");
	m_vulkan = std::make_unique<VulkanContext>(*m_window);

	// Renderer
	m_renderer = std::make_unique<Renderer>(*m_vulkan);

	float aspect = m_vulkan->GetAspectRatio();
	m_camera = std::make_unique<Camera>(60, aspect, 0.1f, 100);

	// scene instance...
	m_scene = std::make_unique<Scene>();
	m_scene->LoadSkybox(*m_renderer);


	Mesh* mesh = m_renderer->LoadMesh("Assets/Models/ak47.fbx");
	Mesh* monkey = m_renderer->LoadMesh("Assets/Models/suzzane.glb");

	Material* mat = m_renderer->LoadMaterial("Assets/Textures/ak47_albedo.png");
	Material* defaultMat = m_renderer->LoadDefaultMaterial();


	m_scene->CreateSphere(*m_renderer);

	// AK
	auto e = m_scene->CreateEntity();
	m_scene->names[e] = ECS::Name{ "ak_1" };
	m_scene->transforms[e] = ECS::Transform{
	{0, 0, -3},
	{-90, 0, -90},
	{1, 1, 1} };

	m_scene->meshs[e] = ECS::MeshRenderer{ mesh };
	m_scene->materials[e] = ECS::MaterialRenderer{ mat };

	// Suzzane
	auto s = m_scene->CreateEntity();
	m_scene->names[s] = ECS::Name{ "Suzzane" };
	m_scene->transforms[s] = ECS::Transform{
	{6, 0, -3},
	{0, 0, 0},
	{1, 1, 1} };

	m_scene->meshs[s] = ECS::MeshRenderer{ monkey };
	m_scene->materials[s] = ECS::MaterialRenderer{ defaultMat };


	// Sun light
	ECS::Entity sunlight = m_scene->CreateEntity();

	m_scene->names[sunlight] = { "Sun_Light" };

	m_scene->lights[sunlight] = {
		ECS::LightType::Directional,
		{1.0f, 1.0f, 1.0f}, // color
		1.0f,               // intensity
		{0.0f, -1.0f, 0.0f}, // direction
		{0.0f, 0.0f, -3.0f} // position
	};

	/*
	// Spot light
	ECS::Entity spot = m_scene->CreateEntity();

	m_scene->names[spot] = { "Spot_Light" };
	//m_scene->transforms[spot] = ECS::Transform{ };
	//need to make it so that the transforms position matches that of the spot light pos

	m_scene->lights[spot] = {
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

	*/

	// Spot light
	ECS::Entity point = m_scene->CreateEntity();

	m_scene->names[point] = { "Point_Light" };
	//m_scene->transforms[spot] = ECS::Transform{ };
	//need to make it so that the transforms position matches that of the spot light pos

	m_scene->lights[point] = {
		ECS::LightType::Point,
		{1.0f, 0.0f, 0.0f}, // color
		10.0f,               // intensity
		{0.0f, -1.0f, 0.0f}, // direction

		//replace with transform pos
		{0.0f, 3.0f, -3.0f}, // position

		10.0f,				// range
		12.9f,				// inner
		17.8f				// outer
	};




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


	/*


	//creating objects at runtime...
	if (m_window->IsKeyDown(GLFW_KEY_SPACE))
	{
		Mesh* akMesh = m_renderer->LoadMesh("Assets/Models/ak47.fbx");
		Material* akMat = m_renderer->LoadMaterial("Assets/Textures/ak47_albedo.png");

		auto e = m_scene->CreateEntity();
		m_scene->names[e] = ECS::Name{ "ak_" };
		m_scene->transforms[e] = ECS::Transform{
		{xPos, 0, -3},
		{-90, 0, 0},
		{1, 1, 1} };
		m_scene->meshs[e] = ECS::MeshRenderer{ akMesh };
		m_scene->materials[e] = ECS::MaterialRenderer{ akMat };

		xPos += 3;

	}

	//obj deletion code...
	if (m_window->IsKeyDown(GLFW_KEY_DELETE))
	{
		const auto& entities = m_scene->GetEntities();

		if (!entities.empty())
		{
			auto last = entities.back();
			m_scene->DestroyEntity(last);
			xPos -= 3;
		}
	}

	*/


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
	// -------- Mouse look (Right Mouse Button) --------
	static bool firstMouse = true;
	static double lastX = 0.0;
	static double lastY = 0.0;

	// -------- Keyboard movement (WASD) --------

	if (m_window->GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
	{
		//m_selectedEntity = (ECS::Entity)-1;

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
	ImGuizmo::BeginFrame();

	ImGuizmo::SetRect(
		0, 0,
		(float)m_window->GetWidth(),
		(float)m_window->GetHeight()
	);

	ImGuiViewport* vp = ImGui::GetMainViewport();

	auto& scene = *m_scene;

	float menuHeight = DrawMenuBar(scene);

	DrawHierarchy(vp, menuHeight, scene);

	DrawInspector(vp, menuHeight, scene);

	DrawGizmos(scene);

}

float Engine::DrawMenuBar(Scene& scene)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Scene"))
		{
			ImGui::MenuItem("Show Skybox", nullptr, &scene.DrawSkybox);

			if (ImGui::MenuItem("Create Cube"))
			{
				scene.CreateCube(*m_renderer);
			}

			ImGui::EndMenu();
		}


	}

	ImGui::EndMainMenuBar();

	return ImGui::GetFrameHeight();
}

void Engine::DrawHierarchy(const ImGuiViewport* vp, float menuHeight, Scene& scene)
{
	ImGui::SetNextWindowPos(ImVec2(
		vp->Pos.x,
		vp->Pos.y + menuHeight));

	ImGui::SetNextWindowSize(ImVec2(300, vp->Size.y - menuHeight));

	ImGui::Begin("Hierarchy");

	// -------------------------------
	// Selection
	// -------------------------------

	for (ECS::Entity e : scene.GetEntities())
	{
		std::string label;

		if (scene.names.count(e))
			label = scene.names[e].name + " ";
		else
			label = "Entity_" + std::to_string(e);

		if (ImGui::Selectable(label.c_str(), m_selectedEntity == e))
		{
			m_selectedEntity = e;
		}
	}

	ImGui::End();
}


void Engine::DrawInspector(const ImGuiViewport* vp, float menuHeight, Scene& scene)
{
	ImGui::SetNextWindowPos(ImVec2(
		vp->Pos.x + vp->Size.x - 400,
		vp->Pos.y + menuHeight));

	ImGui::SetNextWindowSize(ImVec2(400, vp->Size.y - menuHeight));

	ImGui::Begin("Inspector");


	// -------------------------------
	// Selected entity inspector
	// -------------------------------
	if (m_selectedEntity != (ECS::Entity)-1)
	{
		ImGui::Separator();
		ImGui::Text("Inspector");

		// -------- Name --------
		if (scene.names.count(m_selectedEntity))
		{
			auto& name = scene.names[m_selectedEntity].name;

			static char buffer[256];

			auto lastSelected = (ECS::Entity)-1;
			if (lastSelected != m_selectedEntity)
			{
				memset(buffer, 0, sizeof(buffer));
				strncpy_s(buffer, name.c_str(), sizeof(buffer) - 1);
				lastSelected = m_selectedEntity;
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Name");
			ImGui::SameLine();
			if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
			{
				name = buffer;
			}
		}

		// -------- Transform --------
		if (scene.transforms.count(m_selectedEntity))
		{
			auto& t = scene.transforms[m_selectedEntity];

			ImGui::Separator();
			ImGui::Text("Transform");

			ImGui::DragFloat3("Position", glm::value_ptr(t.position), 0.1f);
			ImGui::DragFloat3("Rotation", glm::value_ptr(t.rotation), 0.5f);
			ImGui::DragFloat3("Scale", glm::value_ptr(t.scale), 0.1f);
		}
	}


	// Lights Inspector Menu
	if (scene.lights.count(m_selectedEntity))
	{
		auto& l = scene.lights[m_selectedEntity];

		ImGui::ColorEdit3("Color", glm::value_ptr(l.color));
		ImGui::DragFloat("Intensity", &l.intensity, 0.1f);

		l.intensity = glm::clamp(l.intensity, 0.0f, 1000.0f);


		if (l.type == ECS::LightType::Directional)
		{
			ImGui::DragFloat3("Direction", glm::value_ptr(l.direction), 0.1f);
		}
		else if (l.type == ECS::LightType::Point)
		{
			ImGui::DragFloat3("Position", glm::value_ptr(l.position), 0.1f);
			//ImGui::DragFloat3("Direction", glm::value_ptr(l.direction), 0.1f);

			ImGui::DragFloat("Range", &l.range, 0.1f);

		}
		else if (l.type == ECS::LightType::Spot)
		{
			ImGui::DragFloat3("Position", glm::value_ptr(l.position), 0.1f);
			ImGui::DragFloat3("Direction", glm::value_ptr(l.direction), 0.1f);

			ImGui::DragFloat("Range", &l.range, 0.1f);


			ImGui::DragFloat("InnerCone", &l.innerCone, 0.1f);
			ImGui::DragFloat("OuterCone", &l.outerCone, 0.1f);

			// enforce relationship
			l.innerCone = glm::clamp(l.innerCone, 0.0f, l.outerCone - 0.1f);

		}

	}


	ImGui::End();
}



void Engine::DrawGizmos(Scene& scene)
{
	// -------------------------------
	// GIZMOS
	// -------------------------------
	if (m_selectedEntity != (ECS::Entity)-1 && scene.transforms.count(m_selectedEntity))
	{
		auto& t = scene.transforms[m_selectedEntity];

		glm::mat4 view = m_camera->GetView();
		glm::mat4 proj = m_camera->GetProjection();

		// Undo Vulkan flip for ImGuizmo
		proj[1][1] *= -1.0f;

		// Build matrix from TRS
		glm::mat4 matrix = t.GetMatrix();

		static ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;

		// (optional) hotkeys
		if (ImGui::IsKeyPressed(ImGuiKey_W)) op = ImGuizmo::TRANSLATE;
		if (ImGui::IsKeyPressed(ImGuiKey_E)) op = ImGuizmo::ROTATE;
		if (ImGui::IsKeyPressed(ImGuiKey_R)) op = ImGuizmo::SCALE;

		ImGuizmo::Manipulate(
			glm::value_ptr(view),
			glm::value_ptr(proj),
			op,
			ImGuizmo::WORLD,
			glm::value_ptr(matrix)
		);

		if (ImGuizmo::IsUsing())
		{
			// -------- Apply back to TRS --------

			// Position
			t.position = glm::vec3(matrix[3]);

			// Scale
			t.scale.x = glm::length(glm::vec3(matrix[0]));
			t.scale.y = glm::length(glm::vec3(matrix[1]));
			t.scale.z = glm::length(glm::vec3(matrix[2]));

			// Rotation
			glm::mat3 rotMat;
			rotMat[0] = glm::vec3(matrix[0]) / t.scale.x;
			rotMat[1] = glm::vec3(matrix[1]) / t.scale.y;
			rotMat[2] = glm::vec3(matrix[2]) / t.scale.z;

			glm::quat q = glm::quat_cast(rotMat);
			t.rotation = glm::degrees(glm::eulerAngles(q));
		}
	}



}










