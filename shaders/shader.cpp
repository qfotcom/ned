/*
	File: shader.cpp
	Description: Individual shader management for loading and using GLSL shaders
*/

#include "shaders/shader.h"
#include <GL/glew.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>
#ifndef PLATFORM_WINDOWS
#include <unistd.h>
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace {

std::string getExecutableDirectory()
{
#ifdef _WIN32
	wchar_t buffer[MAX_PATH];
	DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
	if (len == 0 || len >= MAX_PATH)
		return "";
	return std::filesystem::path(buffer).parent_path().string();
#elif defined(__linux__)
	char exePath[4096];
	ssize_t count = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
	if (count <= 0)
		return "";
	exePath[count] = '\0';
	return std::filesystem::path(exePath).parent_path().string();
#else
	return "";
#endif
}

std::string resolveShaderPath(const std::string &relativePath)
{
	const std::string debianPackageBasePath = "/usr/share/Ned/";
	std::vector<std::string> candidates;
	candidates.push_back(relativePath);
	candidates.push_back(debianPackageBasePath + relativePath);

	const std::string exeDir = getExecutableDirectory();
	if (!exeDir.empty())
	{
		const auto exeBase = std::filesystem::path(exeDir);
		candidates.push_back((exeBase / relativePath).string());
		candidates.push_back((exeBase / ".." / relativePath).lexically_normal().string());
		candidates.push_back(
			(exeBase / "../.." / relativePath).lexically_normal().string());
	}

	candidates.push_back(
		(std::filesystem::path("..") / relativePath).lexically_normal().string());
	candidates.push_back(
		(std::filesystem::path("../..") / relativePath).lexically_normal().string());

	for (const auto &candidate : candidates)
	{
		if (std::filesystem::exists(candidate))
		{
			return candidate;
		}
	}

	return "";
}

} // namespace

Shader::Shader() { shaderProgram = 0; }

// Add static set to track warned uniforms
static std::set<std::string> warnedUniforms;

Shader::~Shader()
{
	if (shaderProgram != 0)
	{
		glDeleteProgram(shaderProgram);
	}
}

bool Shader::loadShader(const std::string &vertexShaderRelativePath,
						const std::string &fragmentShaderRelativePath)
{
	const std::string finalVertexShaderPath = resolveShaderPath(vertexShaderRelativePath);
	const std::string finalFragmentShaderPath =
		resolveShaderPath(fragmentShaderRelativePath);

	if (finalVertexShaderPath.empty())
	{
		std::cerr << "🔴 ERROR: Cannot find vertex shader. Tried relative path, "
					 "Debian package path, and paths next to the executable: "
				  << vertexShaderRelativePath << std::endl;
		return false;
	}

	if (finalFragmentShaderPath.empty())
	{
		std::cerr << "🔴 ERROR: Cannot find fragment shader. Tried relative path, "
					 "Debian package path, and paths next to the executable: "
				  << fragmentShaderRelativePath << std::endl;
		return false;
	}

	// Read vertex shader
	std::string vertexShaderCode;
	std::ifstream vertexShaderFile(finalVertexShaderPath);
	if (!vertexShaderFile.is_open())
	{
		std::cerr << "🔴 ERROR: Cannot open resolved vertex shader file: "
				  << finalVertexShaderPath << std::endl;
		return false;
	}
	std::stringstream vertexShaderStream;
	vertexShaderStream << vertexShaderFile.rdbuf();
	vertexShaderCode = vertexShaderStream.str();

	// Read fragment shader
	std::string fragmentShaderCode;
	std::ifstream fragmentShaderFile(finalFragmentShaderPath);
	if (!fragmentShaderFile.is_open())
	{
		std::cerr << "🔴 ERROR: Cannot open resolved fragment shader file: "
				  << finalFragmentShaderPath << std::endl;
		return false;
	}
	std::stringstream fragmentShaderStream;
	fragmentShaderStream << fragmentShaderFile.rdbuf();
	fragmentShaderCode = fragmentShaderStream.str();

	// Compile vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char *vertexShaderSource = vertexShaderCode.c_str();
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Check vertex shader compilation
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "🔴 ERROR: Vertex shader compilation failed: " << infoLog
				  << std::endl;
		glDeleteShader(vertexShader);
		return false;
	}

	// Compile fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fragmentShaderSource = fragmentShaderCode.c_str();
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Check fragment shader compilation
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "🔴 ERROR: Fragment shader compilation failed: " << infoLog
				  << std::endl;
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return false;
	}

	// Create shader program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check linking
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "🔴 ERROR: Shader program linking failed: " << infoLog << std::endl;
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteProgram(shaderProgram);
		shaderProgram = 0;
		return false;
	}

	// Clean up shaders (they're now linked into the program)
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return true;
}

void Shader::useShader() { glUseProgram(shaderProgram); }

void Shader::setFloat(const std::string &name, float value)
{
	GLint location = glGetUniformLocation(shaderProgram, name.c_str());
	if (location != -1)
	{
		glUniform1f(location, value);
	} else
	{
		// Only warn once per uniform name per session
		if (warnedUniforms.find(name) == warnedUniforms.end())
		{
			std::cerr << "⚠️  Warning: Uniform '" << name
					  << "' not found in shader program" << std::endl;
			warnedUniforms.insert(name);
		}
	}
}

void Shader::setInt(const std::string &name, int value)
{
	GLint location = glGetUniformLocation(shaderProgram, name.c_str());
	if (location != -1)
	{
		glUniform1i(location, value);
	} else
	{
		// Only warn once per uniform name per session
		if (warnedUniforms.find(name) == warnedUniforms.end())
		{
			std::cerr << "⚠️  Warning: Uniform '" << name
					  << "' not found in shader program" << std::endl;
			warnedUniforms.insert(name);
		}
	}
}

void Shader::setMatrix4fv(const std::string &name, const float *matrix)
{
	GLint location = glGetUniformLocation(shaderProgram, name.c_str());
	if (location != -1)
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
	} else
	{
		// Only warn once per uniform name per session
		if (warnedUniforms.find(name) == warnedUniforms.end())
		{
			std::cerr << "⚠️  Warning: Uniform '" << name
					  << "' not found in shader program" << std::endl;
			warnedUniforms.insert(name);
		}
	}
}
