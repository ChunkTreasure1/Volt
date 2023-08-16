#pragma once

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <filesystem>
#include <fstream>
#include <cassert>

namespace Volt::RHI
{
	namespace Utility
	{
		inline static std::filesystem::path GetShaderCacheSubDirectory()
		{
			const auto api = GraphicsContext::GetAPI();
			std::filesystem::path subDir;

			switch (api)
			{
				case GraphicsAPI::Vulkan: subDir = "Vulkan"; break;
				case GraphicsAPI::D3D12: subDir = "DX12"; break;
				case GraphicsAPI::MoltenVk: subDir = "MoltenVK"; break;
			}

			return { subDir };
		}

		inline void CreateCacheDirectoryIfNeeded()
		{
			if (!std::filesystem::exists(GetShaderCacheSubDirectory()))
			{
				std::filesystem::create_directories(GetShaderCacheSubDirectory());
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

		inline static const wchar_t* HLSLShaderProfile(const ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:		return L"vs_6_5";
				case ShaderStage::Pixel:		return L"ps_6_5";
				case ShaderStage::Hull:			return L"hs_6_5";
				case ShaderStage::Domain:		return L"hs_6_5";
				case ShaderStage::Geometry:		return L"gs_6_5";
				case ShaderStage::Compute:		return L"cs_6_5";
					
				case ShaderStage::RayGen:		return L"lib_6_5";
				case ShaderStage::Miss:			return L"lib_6_5";
				case ShaderStage::ClosestHit:   return L"lib_6_5";
				case ShaderStage::AnyHit:		return L"lib_6_5";
				case ShaderStage::Intersection: return L"lib_6_5";
			
				case ShaderStage::Task:			return L"ms_6_5";
				case ShaderStage::Mesh:			return L"as_6_5";
			}

			assert(false);
			return L"";
		}

		inline static std::string GetShaderStageCachedFileExtension(const ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:		return ".vertex.cached";
				case ShaderStage::Pixel:		return ".fragment.cached";
				case ShaderStage::Hull:			return "tessControl.cached";
				case ShaderStage::Domain:		return "tessEvaluation.cached";
				case ShaderStage::Geometry:		return ".geometry.cached";
				case ShaderStage::Compute:		return ".compute.cached";

				case ShaderStage::RayGen:		return "raygen.cached";
				case ShaderStage::Miss:			return "raymiss.cached";
				case ShaderStage::ClosestHit:	return "raychit.cached";
				case ShaderStage::AnyHit:		return "rayahit.cached";
				case ShaderStage::Intersection:	return "rayinter.cached";

				case ShaderStage::Task:			return "task.cached";
				case ShaderStage::Mesh:			return "mesh.cached";
			}
			
			assert(false);
			return "";
		}

		inline std::string StageToString(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex: return "Vertex";
				case ShaderStage::Hull: return "Hull";
				case ShaderStage::Domain: return "Domain";
				case ShaderStage::Geometry: return "Geometry";
				case ShaderStage::Pixel: return "Fragment";
				case ShaderStage::Compute: return "Compute";
				
				case ShaderStage::RayGen: return "Ray Generation";
				case ShaderStage::Miss: return "Miss";
				case ShaderStage::ClosestHit: return "Close Hit";
				case ShaderStage::AnyHit: return "Any Hit";
			
				case ShaderStage::Mesh: return "Mesh";
				case ShaderStage::Task: return "Task";
			}

			return "Unsupported";
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
