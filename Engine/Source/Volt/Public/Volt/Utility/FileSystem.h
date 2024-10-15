#pragma once

#include <filesystem>

namespace FileSystem
{
	static std::filesystem::path GetEnginePath()
	{
		return "Engine";
	}

	static std::filesystem::path GetScriptsPath()
	{
		return "Scripts";
	}

	static std::filesystem::path GetShadersPath()
	{
		return "Engine/Shaders";
	}

	static std::filesystem::path GetDebugDumpPath()
	{
		return "User/DebugDumps/";
	}
};
