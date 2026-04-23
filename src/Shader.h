#pragma once

#define NOMINMAX
#include <windows.h>

#include <filesystem>
#include <cstdlib>
#include <stdexcept>
#include <iostream>


namespace Shader
{
	void CompileShaders();

	std::filesystem::path GetExecutableDirectory();

	std::filesystem::path GetCompileShadersScriptPath();
};
