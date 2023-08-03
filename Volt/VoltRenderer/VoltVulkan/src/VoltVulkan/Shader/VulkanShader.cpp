#include "vkpch.h"
#include "VulkanShader.h"

#include "VoltVulkan/Common/VulkanCommon.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Shader/ShaderUtility.h>
#include <VoltRHI/Shader/ShaderCompiler.h>

#include <spirv_cross/spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	namespace Utility
	{
		inline static const ShaderUniformType GetShaderUniformTypeFromSPIRV(const spirv_cross::SPIRType& type)
		{
			if (type.columns == 4)
			{
				switch (type.basetype)
				{
					case spirv_cross::SPIRType::Float: return ShaderUniformType::Float4x4;
				}
			}
			else
			{
				switch (type.basetype)
				{
					case spirv_cross::SPIRType::Boolean: return ShaderUniformType::Bool;
					case spirv_cross::SPIRType::UInt:
					{
						if (type.vecsize == 1) return ShaderUniformType::UInt;
						if (type.vecsize == 2) return ShaderUniformType::UInt2;
						if (type.vecsize == 3) return ShaderUniformType::UInt3;
						if (type.vecsize == 4) return ShaderUniformType::UInt4;

						break;
					}

					case spirv_cross::SPIRType::Int:
					{
						if (type.vecsize == 1) return ShaderUniformType::Int;
						if (type.vecsize == 2) return ShaderUniformType::Int2;
						if (type.vecsize == 3) return ShaderUniformType::Int3;
						if (type.vecsize == 4) return ShaderUniformType::Int4;

						break;
					}

					case spirv_cross::SPIRType::Float:
					{
						if (type.vecsize == 1) return ShaderUniformType::Float;
						if (type.vecsize == 2) return ShaderUniformType::Float2;
						if (type.vecsize == 3) return ShaderUniformType::Float3;
						if (type.vecsize == 4) return ShaderUniformType::Float4;
					}
				}
			}

			return ShaderUniformType::Invalid;
		}
	}

	VulkanShader::VulkanShader(std::string_view name, const std::vector<std::filesystem::path>& sourceFiles, bool forceCompile)
		: m_name(name), m_sourceFiles(sourceFiles)
	{
		if (sourceFiles.empty())
		{
			GraphicsContext::LogTagged(Severity::Error, "[VulkanShader]", "Trying to create a shader {0} without any sources!", name);
			return;
		}

		Reload(forceCompile);
	}

	VulkanShader::~VulkanShader()
	{
		Release();
	}

	const bool VulkanShader::Reload(bool forceCompile)
	{
		Utility::CreateCacheDirectoryIfNeeded();

		m_shaderSources.clear();
		LoadShaderFromFiles();

		if (!CompileOrGetBinary(forceCompile))
		{
			return false;
		}

		Release();
		LoadAndCreateShaders(m_shaderData);
		ReflectAllStages(m_shaderData);

		m_shaderData.clear(); // Unnecessary to store old shader binary

		return true;
	}

	std::string_view VulkanShader::GetName() const
	{
		return m_name;
	}

	const ShaderResources& VulkanShader::GetResources() const
	{
		return m_resources;
	}

	const std::vector<std::filesystem::path>& VulkanShader::GetSourceFiles() const
	{
		return m_sourceFiles;
	}

	ShaderDataBuffer VulkanShader::GetConstantsBuffer() const
	{
		ShaderDataBuffer dataBuffer = m_resources.constantsBuffer;
		dataBuffer.Allocate();
		return dataBuffer;
	}

	void* VulkanShader::GetHandleImpl()
	{
		return nullptr;
	}

	void VulkanShader::LoadShaderFromFiles()
	{
		for (const auto& path : m_sourceFiles)
		{
			const ShaderStage stage = Utility::GetShaderStageFromFilename(path.filename().string());
			const std::string source = Utility::ReadStringFromFile(path);
	
			if (source.empty())
			{
				continue;
			}

			if (m_shaderSources.find(stage) != m_shaderSources.end())
			{
				GraphicsContext::LogTagged(Severity::Error, "[VulkanShader]", "Multiple shaders of same stage defined in file {0}!", path.string().c_str());
				continue;
			}

			m_shaderSources[stage].source = source;
			m_shaderSources[stage].filepath = path;
		}
	}

	void VulkanShader::Release()
	{
		auto device = GraphicsContext::GetDevice();

		for (const auto& [stage, data] : m_pipelineStageInfo)
		{
			vkDestroyShaderModule(device->GetHandle<VkDevice>(), data.shaderModule, nullptr);
		}

		for (const auto& descriptorSetLayout : m_descriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(device->GetHandle<VkDevice>(), descriptorSetLayout, nullptr);
		}

		m_descriptorSetLayouts.clear();
		m_pipelineStageInfo.clear();

		m_resources = {};
	}

	const bool VulkanShader::CompileOrGetBinary(bool forceCompile)
	{
		ShaderCompiler::Specification compileSpec{};
		compileSpec.forceCompile = forceCompile;

		ShaderCompiler::CompilationResult result = ShaderCompiler::TryCompile(compileSpec, *this);
		return result == ShaderCompiler::CompilationResult::Success;
	}

	void VulkanShader::LoadAndCreateShaders(const std::unordered_map<ShaderStage, std::vector<uint32_t>>& shaderData)
	{
		auto device = GraphicsContext::GetDevice();

		for (const auto& [stage, stageData] : shaderData)
		{
			VkShaderModuleCreateInfo moduleInfo{};
			moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleInfo.codeSize = stageData.size() * sizeof(uint32_t);
			moduleInfo.pCode = stageData.data();

			auto& data = m_pipelineStageInfo[stage];
			VT_VK_CHECK(vkCreateShaderModule(device->GetHandle<VkDevice>(), &moduleInfo, nullptr, &data.shaderModule));
		}
	}

	void VulkanShader::ReflectAllStages(const std::unordered_map<ShaderStage, std::vector<uint32_t>>& shaderData)
	{
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "Reflecting {0}", m_name);
		for (const auto& [stage, data] : shaderData)
		{
			ReflectStage(stage, data);
		}
	}

	void VulkanShader::ReflectStage(ShaderStage stage, const std::vector<uint32_t>& data)
	{
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "		Reflecting stage {0}", Utility::StageToString(stage));

		spirv_cross::Compiler compiler{ data };
		const auto resources = compiler.get_shader_resources();

		for (const auto& ubo : resources.uniform_buffers)
		{
			if (compiler.get_active_buffer_ranges(ubo.id).empty())
			{
				continue;
			}

			const uint32_t binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
			const std::string name = compiler.get_name(ubo.id);

			if (name == "$Globals")
			{
				GraphicsContext::LogTagged(Severity::Error, "[VulkanShader]", "Shader {0} seems to have incorrectly defined global variables!", m_name);
				continue;
			}

			m_resources.constantBuffers[set].emplace(binding);

			m_perStageUBOCount[stage].count++;
		}

		for (const auto& ssbo : resources.storage_buffers)
		{
			if (compiler.get_active_buffer_ranges(ssbo.id).empty())
			{
				continue;
			}

			const uint32_t binding = compiler.get_decoration(ssbo.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(ssbo.id, spv::DecorationDescriptorSet);

			m_resources.storageBuffers[set].emplace(binding);
			m_perStageSSBOCount[stage].count++;
		}

		for (const auto& image : resources.storage_images)
		{
			const uint32_t binding = compiler.get_decoration(image.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
			
			m_resources.storageImages[set].emplace(binding);
			m_perStageStorageImageCount[stage].count++;
		}

		for (const auto& image : resources.separate_images)
		{
			const uint32_t binding = compiler.get_decoration(image.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);

			m_resources.images[set].emplace(binding);
			m_perStageImageCount[stage].count++;
		}

		for (const auto& sampler : resources.separate_samplers)
		{
			const uint32_t binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);

			m_resources.images[set].emplace(binding);
			m_perStageSamplerCount[stage].count++;
		}

		for (const auto& pushConstant : resources.push_constant_buffers)
		{
			const auto& bufferType = compiler.get_type(pushConstant.base_type_id);
			const size_t pushConstantSize = compiler.get_declared_struct_size(bufferType);

			m_resources.constantsBuffer.SetSize(pushConstantSize);

			m_resources.constants.size = static_cast<uint32_t>(pushConstantSize);
			m_resources.constants.offset = 0;
			m_resources.constants.stageFlags = m_resources.constants.stageFlags | stage;

			for (uint32_t i = 0; const auto& member : bufferType.member_types)
			{
				const auto& memberType = compiler.get_type(member);
				const size_t memberSize = compiler.get_declared_struct_member_size(bufferType, i);
				const size_t memberOffset = compiler.type_struct_member_offset(bufferType, i);
				const std::string& memberName = compiler.get_member_name(pushConstant.base_type_id, i);

				const auto type = Utility::GetShaderUniformTypeFromSPIRV(memberType);

				m_resources.constantsBuffer.AddMember(memberName, type, memberSize, memberOffset);
				i++;
			}
		}

		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "			Uniform Buffers: {0}", m_perStageUBOCount[stage].count);
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "			Shader Storage Buffers: {0}", m_perStageSSBOCount[stage].count);
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "			Storage Images: {0}", m_perStageStorageImageCount[stage].count);
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "			Images: {0}", m_perStageImageCount[stage].count);
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "			Samplers: {0}", m_perStageSamplerCount[stage].count);
	}
}
