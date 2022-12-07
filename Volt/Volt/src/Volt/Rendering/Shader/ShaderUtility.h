#pragma once

#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Log/Log.h"

#include <GEM/gem.h>
#include <filesystem>

#include <d3dcompiler.h>

namespace Volt
{
	namespace Utility
	{
		inline std::filesystem::path GetShaderCacheDirectory()
		{
			return std::filesystem::path("Engine/Shaders/Cache/");
		}

		inline void CreateCacheDirectoryIfNeeded()
		{
			if (!std::filesystem::exists(GetShaderCacheDirectory()))
			{
				std::filesystem::create_directories(GetShaderCacheDirectory());
			}
		}

		inline ElementType GetTypeFromTypeDesc(D3D11_SHADER_TYPE_DESC desc)
		{
			if (desc.Class == D3D_SVC_SCALAR)
			{
				switch (desc.Type)
				{
					case D3D_SVT_BOOL: return ElementType::Bool;
					case D3D_SVT_INT: return ElementType::Int;
					case D3D_SVT_FLOAT: return ElementType::Float;
					case D3D_SVT_UINT: return ElementType::UInt;
				}
			}
			else if (desc.Class == D3D_SVC_VECTOR)
			{
				switch (desc.Type)
				{
					case D3D_SVT_FLOAT:
					{
						switch (desc.Columns)
						{
							case 2: return ElementType::Float2;
							case 3: return ElementType::Float3;
							case 4: return ElementType::Float4;
						}
						break;
					}
					case D3D_SVT_UINT:
					{
						switch (desc.Columns)
						{
							case 2: return ElementType::UInt2;
							case 3: return ElementType::UInt3;
							case 4: return ElementType::UInt4;
						}
						break;
					}
				}
			}

			return ElementType::Bool;
		}

		inline ShaderStage GetStageFromPath(const std::filesystem::path& path)
		{
			if (path.string().find("_vs") != std::string::npos)
			{
				return ShaderStage::Vertex;
			}
			else if (path.string().find("_ps") != std::string::npos)
			{
				return ShaderStage::Pixel;
			}
			else if (path.string().find("_cs") != std::string::npos)
			{
				return ShaderStage::Compute;
			}
			else if (path.string().find("_hs") != std::string::npos)
			{
				return ShaderStage::Hull;
			}
			else if (path.string().find("_ds") != std::string::npos)
			{
				return ShaderStage::Domain;
			}
			else if (path.string().find("_gs") != std::string::npos)
			{
				return ShaderStage::Geometry;
			}

			VT_CORE_ASSERT(false, "");
			return ShaderStage::Vertex;
		}

		inline std::string GetProfileFromStage(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex: return "vs_5_0";
				case ShaderStage::Pixel: return "ps_5_0";
				case ShaderStage::Hull: return "hs_5_0";
				case ShaderStage::Domain: return "ds_5_0";
				case ShaderStage::Geometry: return "gs_5_0";
				case ShaderStage::Compute: return "cs_5_0";
				default:
					break;
			}

			return "Null";
		}

		inline std::string GetShaderStageCachedFileExtension(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex: return ".vertex.cached";
				case ShaderStage::Pixel: return ".pixel.cached";
				case ShaderStage::Compute: return ".compute.cached";
				case ShaderStage::Hull: return "hull.cached";
				case ShaderStage::Domain: return "domain.cached";
				case ShaderStage::Geometry: return "geometry.cached";
			}

			VT_CORE_ASSERT(false, "Stage not supported!");
			return "";
		}

		inline std::string StageToString(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex: return "Vertex";
				case ShaderStage::Domain: return "Domain";
				case ShaderStage::Hull: return "Hull";
				case ShaderStage::Geometry: return "Geometry";
				case ShaderStage::Pixel: return "Pixel";
				case ShaderStage::Compute: return "Compute";
			}

			return "Unsupported";
		}

		inline size_t HashCombine(size_t lhs, size_t rhs)
		{
			return lhs ^ (rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2));
		}
	}
}