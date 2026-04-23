#include "Shader.h"

//the working of this shader compilation process is inspired by the cherno !
void Shader::CompileShaders()
{
	namespace fs = std::filesystem;

	fs::path scriptPath = GetCompileShadersScriptPath();

	if (!fs::exists(scriptPath))
	{
		std::cerr << "Shader compile script not found: "
			<< scriptPath << std::endl;
		return;
	}

	// cmd.exe is REQUIRED for .bat files
	std::string command = "cmd.exe /c \"" + scriptPath.string() + "\"";

	std::cout << "Running shader compiler:\n" << command << std::endl;

	int result = std::system(command.c_str());

	if (result != 0)
	{
		std::cerr << "Shader compilation failed. Exit code: "
			<< result << std::endl;
	}
	else
	{
		std::cout << "Shaders compiled successfully." << std::endl;
	}
}

std::filesystem::path Shader::GetExecutableDirectory()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(nullptr, buffer, MAX_PATH);
	return std::filesystem::path(buffer).parent_path();
}

std::filesystem::path Shader::GetCompileShadersScriptPath()
{
	namespace fs = std::filesystem;

	fs::path exeDir = GetExecutableDirectory();

	// Adjust ".." count if your layout differs
	fs::path scriptPath =
		exeDir / ".." / ".." / "src" / "shaders" / "compile_shaders.bat";

	return fs::absolute(scriptPath);
}
