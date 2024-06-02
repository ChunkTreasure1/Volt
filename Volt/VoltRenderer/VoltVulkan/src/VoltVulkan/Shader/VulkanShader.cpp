#include "vkpch.h"
#include "VulkanShader.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Graphics/VulkanSwapchain.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"

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
			ShaderUniformType resultType;

			switch (type.basetype)
			{
				case spirv_cross::SPIRType::Boolean: resultType.baseType = ShaderUniformBaseType::Bool; break;
				case spirv_cross::SPIRType::UInt: resultType.baseType = ShaderUniformBaseType::UInt; break;
				case spirv_cross::SPIRType::Int: resultType.baseType = ShaderUniformBaseType::Int; break;
				case spirv_cross::SPIRType::Float: resultType.baseType = ShaderUniformBaseType::Float; break;
				case spirv_cross::SPIRType::Half: resultType.baseType = ShaderUniformBaseType::Half; break;
				case spirv_cross::SPIRType::Short: resultType.baseType = ShaderUniformBaseType::Short; break;
				case spirv_cross::SPIRType::UShort: resultType.baseType = ShaderUniformBaseType::UShort; break;
			}

			resultType.columns = type.columns;
			resultType.vecsize = type.vecsize;

			return resultType;
		}
	}

	VulkanShader::VulkanShader(const ShaderSpecification& specification)
		: m_specification(specification)
	{
		if (m_specification.sourceFiles.empty())
		{
			RHILog::LogTagged(LogSeverity::Error, "[VulkanShader]", "Trying to create a shader {0} without any sources!", m_specification.name);
			return;
		}

		Reload(m_specification.forceCompile);
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

		const ShaderCompiler::CompilationResultData compilationResult = CompileOrGetBinary(forceCompile);
		if (compilationResult.result != ShaderCompiler::CompilationResult::Success)
		{
			return false;
		}

		Release();
		CopyCompilationResults(compilationResult);
		
		for (const auto& [name, bindings] : m_resources.bindings)
		{
			m_resources.usedSets.emplace(bindings.set);
		}

		LoadAndCreateShaders(compilationResult.shaderData);

		CreateDescriptorSetLayouts();
		CalculateDescriptorPoolSizes(compilationResult);
		return true;
	}

	std::string_view VulkanShader::GetName() const
	{
		return m_specification.name;
	}

	const ShaderResources& VulkanShader::GetResources() const
	{
		return m_resources;
	}

	const ShaderResourceBinding& VulkanShader::GetResourceBindingFromName(std::string_view name) const
	{
		static ShaderResourceBinding invalidBinding{};

		std::string nameStr = std::string(name);

		if (!m_resources.bindings.contains(nameStr))
		{
			return invalidBinding;
		}

		return m_resources.bindings.at(nameStr);
	}

	const std::vector<std::filesystem::path>& VulkanShader::GetSourceFiles() const
	{
		return m_specification.sourceFiles;
	}

	ShaderDataBuffer VulkanShader::GetConstantsBuffer() const
	{
		ShaderDataBuffer dataBuffer = m_resources.constantsBuffer;
		return dataBuffer;
	}

	void* VulkanShader::GetHandleImpl() const
	{
		return nullptr;
	}

	void VulkanShader::LoadShaderFromFiles()
	{
		for (const auto& path : m_specification.sourceFiles)
		{
			const ShaderStage stage = Utility::GetShaderStageFromFilename(path.filename().string());
			std::string source = Utility::ReadStringFromFile(path);

			if (source.empty())
			{
				continue;
			}

			if (m_shaderSources.contains(stage))
			{
				RHILog::LogTagged(LogSeverity::Error, "[VulkanShader]", "Multiple shaders of same stage defined in file {0}!", path.string().c_str());
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

		for (const auto& descriptorSetLayout : m_nullPaddedDescriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(device->GetHandle<VkDevice>(), descriptorSetLayout, nullptr);
		}

		m_nullPaddedDescriptorSetLayouts.clear();
		m_descriptorSetLayouts.clear();
		m_pipelineStageInfo.clear();

		m_resources = {};
	}

	ShaderCompiler::CompilationResultData VulkanShader::CompileOrGetBinary(bool forceCompile)
	{
		ShaderCompiler::Specification compileSpec{};
		compileSpec.forceCompile = forceCompile;
		compileSpec.entryPoint = m_specification.entryPoint;
		compileSpec.shaderSourceInfo = m_shaderSources;

		return ShaderCompiler::TryCompile(compileSpec);
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

	void VulkanShader::CreateDescriptorSetLayouts()
	{
		struct DefaultValue
		{
			bool value = false;
		};

		std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptorSetBindings{};
		std::map<uint32_t, std::map<uint32_t, DefaultValue>> isBindlessMap{};

		for (const auto& [set, bindings] : m_resources.uniformBuffers)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& descriptorBinding = descriptorSetBindings[set].emplace_back();
				descriptorBinding.binding = binding;
				descriptorBinding.descriptorCount = 1;
				descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorBinding.stageFlags = static_cast<VkShaderStageFlags>(data.usageStages);
			}
		}

		for (const auto& [set, bindings] : m_resources.storageBuffers)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& descriptorBinding = descriptorSetBindings[set].emplace_back();
				descriptorBinding.binding = binding;

				if (data.arraySize == -1)
				{
					descriptorBinding.descriptorCount = VulkanDefaults::STORAGE_BUFFER_BINDLESS_TABLE_SIZE;
					isBindlessMap[set][binding].value = true;
				}
				else
				{
					descriptorBinding.descriptorCount = static_cast<uint32_t>(data.arraySize);
				}

				descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorBinding.stageFlags = static_cast<VkShaderStageFlags>(data.usageStages);
			}
		}

		for (const auto& [set, bindings] : m_resources.storageImages)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& descriptorBinding = descriptorSetBindings[set].emplace_back();
				descriptorBinding.binding = binding;

				if (data.arraySize == -1)
				{
					descriptorBinding.descriptorCount = VulkanDefaults::STORAGE_IMAGE_BINDLESS_TABLE_SIZE;
					isBindlessMap[set][binding].value = true;
				}
				else
				{
					descriptorBinding.descriptorCount = static_cast<uint32_t>(data.arraySize);
				}

				descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				descriptorBinding.stageFlags = static_cast<VkShaderStageFlags>(data.usageStages);
			}
		}

		for (const auto& [set, bindings] : m_resources.images)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& descriptorBinding = descriptorSetBindings[set].emplace_back();
				descriptorBinding.binding = binding;

				if (data.arraySize == -1)
				{
					descriptorBinding.descriptorCount = VulkanDefaults::IMAGE_BINDLESS_TABLE_SIZE;
					isBindlessMap[set][binding].value = true;
				}
				else
				{
					descriptorBinding.descriptorCount = static_cast<uint32_t>(data.arraySize);
				}

				descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				descriptorBinding.stageFlags = static_cast<VkShaderStageFlags>(data.usageStages);
			}
		}

		for (const auto& [set, bindings] : m_resources.samplers)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& descriptorBinding = descriptorSetBindings[set].emplace_back();
				descriptorBinding.binding = binding;
				descriptorBinding.descriptorCount = 1;
				descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				descriptorBinding.stageFlags = static_cast<VkShaderStageFlags>(data.usageStages);
			}
		}

		constexpr VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

		auto device = GraphicsContext::GetDevice();

		const bool usingDescriptorBuffers = GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().AreDescriptorBuffersEnabled();

		int32_t lastSet = -1;
		for (const auto& [set, bindings] : descriptorSetBindings)
		{
			while (static_cast<int32_t>(set) > lastSet + 1)
			{
				VkDescriptorSetLayoutCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				info.pNext = nullptr;
				info.bindingCount = 0;
				info.pBindings = nullptr;
				info.flags = 0;

				if (usingDescriptorBuffers)
				{
					info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
				}

				VT_VK_CHECK(vkCreateDescriptorSetLayout(device->GetHandle<VkDevice>(), &info, nullptr, &m_nullPaddedDescriptorSetLayouts.emplace_back()));
				lastSet++;
			}

			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.pNext = nullptr;
			info.bindingCount = static_cast<uint32_t>(bindings.size());
			info.pBindings = bindings.data();
			info.flags = 0;

			if (usingDescriptorBuffers)
			{
				info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
			}

			std::vector<VkDescriptorBindingFlags> bindingFlags{};

			VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
			extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			extendedInfo.bindingCount = info.bindingCount;

			for (const auto& binding : bindings)
			{
				auto& flags = bindingFlags.emplace_back();
				flags = 0;

				if (isBindlessMap[set][binding.binding].value)
				{
					flags = bindlessFlags;
					if (!usingDescriptorBuffers)
					{
						info.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
						flags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
					}
				}
			}

			extendedInfo.pBindingFlags = bindingFlags.data();
			info.pNext = &extendedInfo;

			VT_VK_CHECK(vkCreateDescriptorSetLayout(device->GetHandle<VkDevice>(), &info, nullptr, &m_nullPaddedDescriptorSetLayouts.emplace_back()));
			m_descriptorSetLayouts.emplace_back(m_nullPaddedDescriptorSetLayouts.back());

			lastSet = set;
		}
	}

	void VulkanShader::CalculateDescriptorPoolSizes(const ShaderCompiler::CompilationResultData& compilationResult)
	{
		uint32_t uboCount = 0;
		uint32_t ssboCount = 0;
		uint32_t storageImageCount = 0;
		uint32_t imageCount = 0;
		uint32_t seperateSamplerCount = 0;

		// Find counts
		{
			for (const auto& [set, bindings] : compilationResult.uniformBuffers)
			{
				for (const auto& [binding, info] : bindings)
				{
					uboCount += info.usageCount;
				}
			}

			for (const auto& [set, bindings] : compilationResult.storageBuffers)
			{
				for (const auto& [binding, info] : bindings)
				{
					ssboCount += info.usageCount;
				}
			}

			for (const auto& [set, bindings] : compilationResult.storageImages)
			{
				for (const auto& [binding, info] : bindings)
				{
					storageImageCount += info.usageCount;
				}
			}

			for (const auto& [set, bindings] : compilationResult.images)
			{
				for (const auto& [binding, info] : bindings)
				{
					imageCount += info.usageCount;
				}
			}

			for (const auto& [set, bindings] : compilationResult.samplers)
			{
				for (const auto& [binding, info] : bindings)
				{
					seperateSamplerCount += info.usageCount;
				}
			}
		}

		if (uboCount > 0)
		{
			m_descriptorPoolSizes.emplace_back(static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER), uboCount * VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);
		}

		if (ssboCount > 0)
		{
			m_descriptorPoolSizes.emplace_back(static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER), ssboCount * VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);
		}

		if (storageImageCount > 0)
		{
			m_descriptorPoolSizes.emplace_back(static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE), storageImageCount * VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);
		}

		if (imageCount > 0)
		{
			m_descriptorPoolSizes.emplace_back(static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE), imageCount * VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);
		}

		if (seperateSamplerCount > 0)
		{
			m_descriptorPoolSizes.emplace_back(static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_SAMPLER), seperateSamplerCount * VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);
		}
	}

	void VulkanShader::CopyCompilationResults(const ShaderCompiler::CompilationResultData& compilationResult)
	{
		m_resources.outputFormats = compilationResult.outputFormats;
		m_resources.vertexLayout = compilationResult.vertexLayout;
		m_resources.instanceLayout = compilationResult.instanceLayout;

		m_resources.renderGraphConstantsData = compilationResult.renderGraphConstants;
		m_resources.constantsBuffer = compilationResult.constantsBuffer;
		m_resources.constants = compilationResult.constants;

		m_resources.bindings = compilationResult.bindings;

		m_resources.uniformBuffers = compilationResult.uniformBuffers;
		m_resources.storageBuffers = compilationResult.storageBuffers;
		m_resources.storageImages = compilationResult.storageImages;
		m_resources.images = compilationResult.images;
		m_resources.samplers = compilationResult.samplers;
	}
}
