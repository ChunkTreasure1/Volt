#include "vtpch.h"
#include "FbxUtilities.h"

#include <TGAFbx.h>

namespace Volt::FbxUtilities
{
	FbxInformation GetFbxInformation(const std::filesystem::path& fbxFilePath)
	{
		TGA::FBX::Importer::InitImporter();
		const auto info = TGA::FBX::Importer::GetFbxInformation(fbxFilePath.string());
		TGA::FBX::Importer::UninitImporter();

		FbxInformation result{};
		result.fileVersion = info.fileVersion;
		result.fileCreator = info.fileCreator;
		result.fileCreatorApplication = info.fileCreatorApplication;
		result.fileUnits = info.fileUnits;
		result.fileAxisDirection = info.fileAxisDirection;

		result.hasSkeleton = info.hasSkeleton;
		result.hasMesh = info.hasMesh;
		result.hasAnimation = info.hasAnimation;

		return result;
	}
}
