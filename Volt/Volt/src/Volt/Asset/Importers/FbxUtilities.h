#pragma once

#include <string>
#include <filesystem>

namespace Volt
{
	struct FbxInformation
	{
		std::string fileVersion;
		std::string fileCreator;
		std::string fileCreatorApplication;
		std::string fileUnits;
		std::string fileAxisDirection;

		bool hasSkeleton = false;
		bool hasMesh = false;
		bool hasAnimation = false;
	};

	namespace FbxUtilities
	{
		FbxInformation GetFbxInformation(const std::filesystem::path& fbxFilePath);
	}
}
