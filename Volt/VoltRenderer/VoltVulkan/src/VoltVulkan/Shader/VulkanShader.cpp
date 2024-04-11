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
			GraphicsContext::LogTagged(Severity::Error, "[VulkanShader]", "Trying to create a shader {0} without any sources!", m_specification.name);
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

		if (!CompileOrGetBinary(forceCompile))
		{
			return false;
		}

		auto vertexLayout = m_resources.vertexLayout;
		auto instanceLayout = m_resources.instanceLayout;
		auto outputFormats = m_resources.outputFormats;
		auto renderGraphConstants = m_resources.renderGraphConstantsData;

		Release();

		m_resources.outputFormats = outputFormats;
		m_resources.vertexLayout = vertexLayout;
		m_resources.instanceLayout = instanceLayout;
		m_resources.renderGraphConstantsData = renderGraphConstants;

		LoadAndCreateShaders(m_shaderData);
		ReflectAllStages(m_shaderData);

		m_shaderData.clear(); // Unnecessary to store old shader binary

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

		for (const auto& descriptorSetLayout : m_nullPaddedDescriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(device->GetHandle<VkDevice>(), descriptorSetLayout, nullptr);
		}

		m_nullPaddedDescriptorSetLayouts.clear();
		m_descriptorSetLayouts.clear();
		m_pipelineStageInfo.clear();

		m_resources = {};
	}

	const bool VulkanShader::CompileOrGetBinary(bool forceCompile)
	{
		ShaderCompiler::Specification compileSpec{};
		compileSpec.forceCompile = forceCompile;
		compileSpec.entryPoint = m_specification.entryPoint;

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
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "Reflecting {0}", m_specification.name);
		for (const auto& [stage, data] : shaderData)
		{
			ReflectStage(stage, data);
		}

		CreateDescriptorSetLayouts();
		CalculateDescriptorPoolSizes();
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

			const auto& bufferType = compiler.get_type(ubo.base_type_id);

			const size_t size = compiler.get_declared_struct_size(bufferType);
			const uint32_t binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
			const std::string& name = compiler.get_name(ubo.id);

			if (!TryAddShaderBinding(name, set, binding))
			{
				GraphicsContext::LogTagged(Severity::Error, "[VulkanShader]", "Unable to add binding with name {0} to list. It already exists!", name);
			}

			if (name == "$Globals")
			{
				GraphicsContext::LogTagged(Severity::Error, "[VulkanShader]", "Shader {0} seems to have incorrectly defined global variables!", m_specification.name);
				continue;
			}

			auto& buffer = m_resources.uniformBuffers[set][binding];
			buffer.usageStages = buffer.usageStages | stage;
			buffer.size = size;

			m_perStageUBOCount[stage].count++;
			m_resources.usedSets.emplace(set);
		}

		for (const auto& ssbo : resources.storage_buffers)
		{
			//if (compiler.get_active_buffer_ranges(ssbo.id).empty()) // #TODO_Ivar: Crashes for some reason when using buffer arrays
			//{
			//	continue;
			//}

			const auto& bufferBaseType = compiler.get_type(ssbo.base_type_id);
			const auto& bufferType = compiler.get_type(ssbo.type_id);

			const size_t size = compiler.get_declared_struct_size(bufferBaseType);
			const uint32_t binding = compiler.get_decoration(ssbo.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(ssbo.id, spv::DecorationDescriptorSet);
			const std::string& name = compiler.get_name(ssbo.id);

			if (!TryAddShaderBinding(name, set, binding))
			{
				GraphicsContext::LogTagged(Severity::Error, "[VulkanShader]", "Unable to add binding with name {0} to list. It already exists!", name);
			}

			const bool firstEntry = !m_resources.storageBuffers[set].contains(binding);

			auto& buffer = m_resources.storageBuffers[set][binding];
			buffer.usageStages = buffer.usageStages | stage;
			buffer.size = size;

			if (firstEntry && !bufferType.array.empty())
			{
				const int32_t arraySize = static_cast<int32_t>(bufferType.array[0]);

				if (arraySize == 0)
				{
					buffer.arraySize = -1;
				}
				else
				{
					buffer.arraySize = arraySize;
				}
			}

			m_perStageSSBOCount[stage].count++;
			m_resources.usedSets.emplace(set);
		}

		for (const auto& image : resources.storage_images)
		{
			const uint32_t binding = compiler.get_decoration(image.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
			const auto& imageType = compiler.get_type(image.type_id);
			const std::string& name = compiler.get_name(image.id);

			if (!TryAddShaderBinding(name, set, binding))
			{
				GraphicsContext::LogTagged(Severity::Error, "[VulkanShader]", "Unable to add binding with name {0} to list. It already exists!", name);
			}

			const bool firstEntry = !m_resources.storageImages[set].contains(binding);

			auto& shaderImage = m_resources.storageImages[set][binding];
			shaderImage.usageStages = shaderImage.usageStages | stage;

			if (firstEntry && !imageType.array.empty())
			{
				const int32_t arraySize = static_cast<int32_t>(imageType.array[0]);

				if (arraySize == 0)
				{
					shaderImage.arraySize = -1;
				}
				else
				{
					shaderImage.arraySize = arraySize;
				}
			}

			m_perStageStorageImageCount[stage].count++;
			m_resources.usedSets.emplace(set);
		}

		for (const auto& image : resources.separate_images)
		{
			const uint32_t binding = compiler.get_decoration(image.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
			const auto& imageType = compiler.get_type(image.type_id);
			const std::string& name = compiler.get_name(image.id);

			if (!TryAddShaderBinding(name, set, binding))
			{
				GraphicsContext::LogTagged(Severity::Error, "[VulkanShader]", "Unable to add binding with name {0} to list. It already exists!", name);
			}

			const bool firstEntry = !m_resources.images[set].contains(binding);

			auto& shaderImage = m_resources.images[set][binding];
			shaderImage.usageStages = shaderImage.usageStages | stage;

			if (firstEntry && !imageType.array.empty())
			{
				const int32_t arraySize = static_cast<int32_t>(imageType.array[0]);

				if (arraySize == 0)
				{
					shaderImage.arraySize = -1;
				}
				else
				{
					shaderImage.arraySize = arraySize;
				}
			}

			m_perStageImageCount[stage].count++;
			m_resources.usedSets.emplace(set);
		}

		for (const auto& sampler : resources.separate_samplers)
		{
			const uint32_t binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);
			const std::string& name = compiler.get_name(sampler.id);

			if (!TryAddShaderBinding(name, set, binding))
			{
				GraphicsContext::LogTagged(Severity::Error, "[VulkanShader]", "Unable to add binding with name {0} to list. It already exists!", name);
			}

			auto& shaderSampler = m_resources.samplers[set][binding];
			shaderSampler.usageStages = shaderSampler.usageStages | stage;
			m_perStageSamplerCount[stage].count++;
			m_resources.usedSets.emplace(set);
		}

		for (const auto& pushConstant : resources.push_constant_buffers)
		{
			const auto& bufferType = compiler.get_type(pushConstant.base_type_id);
			const size_t pushConstantSize = compiler.get_declared_struct_size(bufferType);

			m_resources.constantsBuffer.SetSize(pushConstantSize);

			m_resources.constants.size = static_cast<uint32_t>(pushConstantSize);
			m_resources.constants.offset = 0;
			m_resources.constants.stageFlags = m_resources.constants.stageFlags | stage;

			for (uint32_t i = 0; const auto & member : bufferType.member_types)
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
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "			Storage Buffers: {0}", m_perStageSSBOCount[stage].count);
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "			Storage Images: {0}", m_perStageStorageImageCount[stage].count);
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "			Images: {0}", m_perStageImageCount[stage].count);
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "			Samplers: {0}", m_perStageSamplerCount[stage].count);

		if (stage == ShaderStage::Vertex)
		{
			GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "			Vertex Layout:");

			for (const auto& element : m_resources.vertexLayout.GetElements())
			{
				GraphicsContext::LogTagged(Severity::Trace, "[VulkanShader]", "				{0}: {1}", element.name, BufferLayout::GetNameFromElementType(element.type));
			}
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

	void VulkanShader::CalculateDescriptorPoolSizes()
	{
		uint32_t uboCount = 0;
		uint32_t ssboCount = 0;
		uint32_t storageImageCount = 0;
		uint32_t imageCount = 0;
		uint32_t seperateSamplerCount = 0;

		std::for_each(m_perStageUBOCount.begin(), m_perStageUBOCount.end(), [&](auto pair) { uboCount += pair.second.count; });
		std::for_each(m_perStageSSBOCount.begin(), m_perStageSSBOCount.end(), [&](auto pair) { ssboCount += pair.second.count; });
		std::for_each(m_perStageStorageImageCount.begin(), m_perStageStorageImageCount.end(), [&](auto pair) { storageImageCount += pair.second.count; });
		std::for_each(m_perStageImageCount.begin(), m_perStageImageCount.end(), [&](auto pair) { imageCount += pair.second.count; });
		std::for_each(m_perStageSamplerCount.begin(), m_perStageSamplerCount.end(), [&](auto pair) { seperateSamplerCount += pair.second.count; });

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

	const bool VulkanShader::TryAddShaderBinding(const std::string& name, uint32_t set, uint32_t binding)
	{
		if (m_resources.bindings.contains(name))
		{
			return false;
		}

		m_resources.bindings[name] = { set, binding };

		return true;
	}
}
