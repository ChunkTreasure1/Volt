#pragma once

#include <string>

class FileSystem
{
public:
	static bool HasEnvironmentVariable(const std::string& key);
	static std::string GetEnvVariable(const std::string& key);
};