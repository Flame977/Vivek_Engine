#include "Window.h"


Window::Window(int width, int height, std::string title) :m_width(width), m_height(height)
{
	// Initialize GLFW
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	// Tell GLFW we are NOT using OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// To make window resizable
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(m_window, this);

	glfwSetFramebufferSizeCallback(m_window, Window::FrameBufferSizeCallback);

	if (!m_window)
		throw std::runtime_error("Failed to create GLFW window");
}

Window::~Window()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Window::PollEvents()
{
	glfwPollEvents();
}

bool Window::IsOpen() const
{
	return !glfwWindowShouldClose(m_window);
}

GLFWwindow* Window::GetWindow() const
{
	return m_window;
}

int Window::GetWidth() const
{
	return m_width;
}

int Window::GetHeight() const
{
	return m_height;
}

bool Window::WasResized() const
{
	return m_resized;
}

void Window::ResetResizeFlag()
{
	m_resized = false;
}

bool Window::IsMouseButtonDown(int button) const
{
	return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void Window::GetMousePos(double& x, double& y) const
{
	return glfwGetCursorPos(m_window, &x, &y);
}

bool Window::GetMouseButton(int button) const
{
	return glfwGetMouseButton(m_window, button);
}

bool Window::IsMouseInside() const
{
	return glfwGetWindowAttrib(m_window, GLFW_HOVERED);
}

bool Window::IsKey(int key) const
{
	return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::IsKeyDown(int key) const
{
	return m_currentKeys[key] && !m_previousKeys[key];
}

void Window::UpdateInput()
{
	m_previousKeys = m_currentKeys;

	for (size_t i = 0; i < GLFW_KEY_LAST; i++)
	{
		m_currentKeys[i] = glfwGetKey(m_window, i) == GLFW_PRESS;
	}
}

void Window::SetCursorMode(bool mode)
{
	if (!mode)
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}



void Window::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	m_resized = true;
}

void Window::FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
	win->Resize(width, height);
}





