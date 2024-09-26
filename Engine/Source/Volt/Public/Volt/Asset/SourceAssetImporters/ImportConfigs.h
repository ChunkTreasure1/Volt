#pragma once

#include <glm/glm.hpp>

#include <filesystem>

namespace Volt
{
	enum class MeshSourceImportType : uint8_t
	{
		StaticMesh,
		SkeletalMesh,
		Animation
	};

	enum class GenerateNormalsMode : uint8_t
	{
		Never,
		Smooth, // Generate smooth, if not exists
		Hard, // Generate hard, if not exists
		OverwriteSmooth, // Overwrite with smooth normals
		OverwriteHard // Overwrite with hard normals
	};

	struct MeshSourceImportConfig
	{
		std::filesystem::path destinationDirectory;
		std::string destinationFilename;
		std::string password;

		glm::vec3 translation = 0.f;
		glm::vec3 rotation = 0.f;
		glm::vec3 scale = 1.f;

		MeshSourceImportType importType = MeshSourceImportType::StaticMesh;
		GenerateNormalsMode generateNormalsMode = GenerateNormalsMode::Never;

		bool importAnimationIfSkeletalMesh = false;
		bool importVertexColors = false;
		bool importMaterial = true;

		bool convertScene = true;
		bool combineMeshes = false;
		bool removeDegeneratePolygons = true;
		bool triangulate = true;
		bool generateTangents = true;
	};

	struct TextureSourceImportConfig
	{
		std::filesystem::path destinationDirectory;
		std::string destinationFilename;

		bool importMipMaps = true;
		bool generateMipMaps = true;
		bool createAsMemoryAsset = false;
	};
}
