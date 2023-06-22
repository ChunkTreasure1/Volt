#pragma once

#include "Volt/Log/Log.h"

#include <glm/glm.hpp>
#include <filesystem>

#include <d3dcompiler.h>
#include <vulkan/vulkan.h> 
#include <shaderc/shaderc.h>

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

		inline VkShaderStageFlagBits GetShaderStageFromFilename(const std::string& filename)
		{
			if (filename.find("_fs.glsl") != std::string::npos || filename.find("_ps.hlsl") != std::string::npos)
			{
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			}
			else if (filename.find("_vs.glsl") != std::string::npos || filename.find("_vs.hlsl") != std::string::npos)
			{
				return VK_SHADER_STAGE_VERTEX_BIT;
			}
			else if (filename.find("_cs.glsl") != std::string::npos || filename.find("_cs.hlsl") != std::string::npos)
			{
				return VK_SHADER_STAGE_COMPUTE_BIT;
			}
			else if (filename.find("_gs.glsl") != std::string::npos || filename.find("_gs.hlsl") != std::string::npos)
			{
				return VK_SHADER_STAGE_GEOMETRY_BIT;
			}
			else if (filename.find("_te.glsl") != std::string::npos)
			{
				return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			}
			else if (filename.find("_tc.glsl") != std::string::npos)
			{
				return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			}
			else if (filename.find("_rgen.glsl") != std::string::npos || filename.find("_rgen.hlsl") != std::string::npos)
			{
				return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
			}
			else if (filename.find("_rmiss.glsl") != std::string::npos || filename.find("_rmiss.hlsl") != std::string::npos)
			{
				return VK_SHADER_STAGE_MISS_BIT_KHR;
			}
			else if (filename.find("_rchit.glsl") != std::string::npos || filename.find("_rchit.hlsl") != std::string::npos)
			{
				return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			}
			else if (filename.find("_rahit.glsl") != std::string::npos || filename.find("_rahit.hlsl") != std::string::npos)
			{
				return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			}

			return (VkShaderStageFlagBits)0;
		}

		inline std::string GetShaderStageCachedFileExtension(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT: return ".vertex.cached";
				case VK_SHADER_STAGE_FRAGMENT_BIT: return ".fragment.cached";
				case VK_SHADER_STAGE_COMPUTE_BIT: return ".compute.cached";
				case VK_SHADER_STAGE_GEOMETRY_BIT: return ".geometry.cached";
				case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return "tessControl.cached";
				case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return "tessEvaluation.cached";
				case VK_SHADER_STAGE_RAYGEN_BIT_KHR: return "raygen.cached";
				case VK_SHADER_STAGE_MISS_BIT_KHR: return "raymiss.cached";
				case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return "raychit.cached";
				case VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return "rayahit.cached";
			}

			VT_CORE_ASSERT(false, "Stage not supported!");
			return "";
		}

		inline shaderc_shader_kind VulkanToShaderCStage(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT: return shaderc_vertex_shader;
				case VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_fragment_shader;
				case VK_SHADER_STAGE_COMPUTE_BIT: return shaderc_compute_shader;
				case VK_SHADER_STAGE_GEOMETRY_BIT: return shaderc_geometry_shader;
				case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return shaderc_tess_control_shader;
				case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return shaderc_tess_evaluation_shader;
				case VK_SHADER_STAGE_RAYGEN_BIT_KHR: return shaderc_raygen_shader;
				case VK_SHADER_STAGE_MISS_BIT_KHR: return shaderc_miss_shader;
				case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return shaderc_closesthit_shader;
				case VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return shaderc_anyhit_shader;
			}

			VT_CORE_ASSERT(false, "Stage not supported!");
			return (shaderc_shader_kind)0;
		}

		inline static const wchar_t* HLSLShaderProfile(const VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT:    return L"vs_6_5";
				case VK_SHADER_STAGE_FRAGMENT_BIT:  return L"ps_6_5";
				case VK_SHADER_STAGE_COMPUTE_BIT:   return L"cs_6_5";
				case VK_SHADER_STAGE_GEOMETRY_BIT:   return L"gs_6_5";
				case VK_SHADER_STAGE_RAYGEN_BIT_KHR:   return L"lib_6_5";
				case VK_SHADER_STAGE_MISS_BIT_KHR:   return L"lib_6_5";
				case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:   return L"lib_6_5";
				case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:   return L"lib_6_5";
			}
			VT_CORE_ASSERT(false, "");
			return L"";
		}

		inline std::string StageToString(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT: return "Vertex";
				case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return "Tessellation Control";
				case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return "Tessellation Evaluation";
				case VK_SHADER_STAGE_GEOMETRY_BIT: return "Geometry";
				case VK_SHADER_STAGE_FRAGMENT_BIT: return "Fragment";
				case VK_SHADER_STAGE_COMPUTE_BIT: return "Compute";
				case VK_SHADER_STAGE_RAYGEN_BIT_KHR: return "Ray Generation";
				case VK_SHADER_STAGE_MISS_BIT_KHR: return "Miss";
				case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return "Close Hit";
				case VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return "Any Hit";
			}

			return "Unsupported";
		}

		inline std::string ReadStringFromFile(const std::filesystem::path& path)
		{
			std::string result;
			std::ifstream in(path, std::ios::in | std::ios::binary);
			if (in)
			{
				in.seekg(0, std::ios::end);
				result.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(&result[0], result.size());
			}
			else
			{
				VT_CORE_ERROR("Unable to read shader {0}!", path.string());
				return {};
			}

			in.close();

			return result;
		}

		inline size_t HashCombine(size_t lhs, size_t rhs)
		{
			return lhs ^ (rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2));
		}

		inline uint64_t GetAlignedSize(uint64_t size, uint64_t alignment)
		{
			return (size + alignment - 1) & ~(alignment - 1);
		}
	}
}
