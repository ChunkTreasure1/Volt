#pragma once

#include "VoltRHI/Core/RHICommon.h"

#include <filesystem>
#include <fstream>

namespace Volt::RHI
{
	namespace Utility
	{
		inline std::filesystem::path GetShaderCacheDirectory()
		{
			return { "Engine/Shaders/Cache/" };
		}

		inline void CreateCacheDirectoryIfNeeded()
		{
			if (!std::filesystem::exists(GetShaderCacheDirectory()))
			{
				std::filesystem::create_directories(GetShaderCacheDirectory());
			}
		}

		inline ShaderStage GetShaderStageFromFilename(const std::string& filename)
		{
			if (filename.find("_fs.glsl") != std::string::npos || filename.find("_ps.hlsl") != std::string::npos)
			{
				return ShaderStage::Pixel;
			}
			else if (filename.find("_vs.glsl") != std::string::npos || filename.find("_vs.hlsl") != std::string::npos)
			{
				return ShaderStage::Vertex;
			}
			else if (filename.find("_cs.glsl") != std::string::npos || filename.find("_cs.hlsl") != std::string::npos)
			{
				return ShaderStage::Compute;
			}
			else if (filename.find("_gs.glsl") != std::string::npos || filename.find("_gs.hlsl") != std::string::npos)
			{
				return ShaderStage::Geometry;
			}
			else if (filename.find("_tes.glsl") != std::string::npos || filename.find("_hull.hlsl"))
			{
				return ShaderStage::Hull;
			}
			else if (filename.find("_tc.glsl") != std::string::npos || filename.find("_dom.hlsl"))
			{
				return ShaderStage::Domain;
			}
			else if (filename.find("_rgen.glsl") != std::string::npos || filename.find("_rgen.hlsl") != std::string::npos)
			{
				return ShaderStage::RayGen;
			}
			else if (filename.find("_rmiss.glsl") != std::string::npos || filename.find("_rmiss.hlsl") != std::string::npos)
			{
				return ShaderStage::Miss;
			}
			else if (filename.find("_rchit.glsl") != std::string::npos || filename.find("_rchit.hlsl") != std::string::npos)
			{
				return ShaderStage::ClosestHit;
			}
			else if (filename.find("_rahit.glsl") != std::string::npos || filename.find("_rahit.hlsl") != std::string::npos)
			{
				return ShaderStage::AnyHit;
			}
			else if (filename.find("_rinter.hlsl") != std::string::npos)
			{
				return ShaderStage::Intersection;
			}
			else if (filename.find("_task.hlsl") != std::string::npos)
			{
				return ShaderStage::Task;
			}
			else if (filename.find("_mesh.hlsl") != std::string::npos)
			{
				return ShaderStage::Mesh;
			}

			return ShaderStage::None;
		}

		inline std::string ReadStringFromFile(const std::filesystem::path& path)
		{
			std::string result{};
			std::ifstream in{ path, std::ios::in, std::ios::binary };

			if (in)
			{
				in.seekg(0, std::ios::end);
				result.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(&result[0], result.size());
			}

			in.close();
			return result;
		}
	}
}
