#pragma once

#include <GLfW/glfw3.h>
#include<string>
#include<stdexcept>
#include <array>

struct GLFW_Window;

class Window
{
public:

	Window(int width, int height, std::string title);

	~Window();

	void PollEvents();

	bool IsOpen() const;

	GLFWwindow* GetWindow() const;

	int GetWidth() const;

	int GetHeight() const;


	bool WasResized() const;

	void ResetResizeFlag();


	bool IsMouseButtonDown(int button) const;

	void GetMousePos(double& x, double& y) const;

	bool GetMouseButton(int button) const;

	bool IsMouseInside() const;

	bool IsKey(int key) const;

	bool IsKeyDown(int key) const;

	void UpdateInput();

	void SetCursorMode(bool mode);


private:

	GLFWwindow* m_window;

	void Resize(int width, int height);

	//this needs to be static cuz of glfw
	static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

	int m_width;

	int m_height;

	bool m_resized = false;

	std::array<bool, GLFW_KEY_LAST + 1> m_previousKeys{};

	std::array<bool, GLFW_KEY_LAST + 1> m_currentKeys{};
};