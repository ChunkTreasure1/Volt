#pragma once

#include <filesystem>

struct Project
{
	std::string name;
	std::filesystem::path path;
};

struct EngineInfo
{
	std::filesystem::path engineDirectory;
	bool needsRepair = false;

	inline const bool IsValid() const
	{
		return !engineDirectory.empty();
	}
};