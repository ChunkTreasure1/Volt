#include "vtpch.h"
#include "Shader.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Core/Application.h"

#include "Volt/Rendering/Shader/ShaderUtility.h"

#include <spirv_cross/spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>

namespace Volt
{
	namespace Utility
	{
		inline static const uint32_t GetSizeFromShaderUniformType(const ShaderUniformType type)
		{
			switch (type)
			{
				case ShaderUniformType::Bool: return sizeof(bool);

				case ShaderUniformType::UInt: return sizeof(uint32_t);
				case ShaderUniformType::UInt2: return sizeof(uint32_t) * 2;
				case ShaderUniformType::UInt3: return sizeof(uint32_t) * 3;
				case ShaderUniformType::UInt4: return sizeof(uint32_t) * 4;

				case ShaderUniformType::Int: return sizeof(int32_t);
				case ShaderUniformType::Int2: return sizeof(int32_t) * 2;
				case ShaderUniformType::Int3: return sizeof(int32_t) * 3;
				case ShaderUniformType::Int4: return sizeof(int32_t) * 4;

				case ShaderUniformType::Float: return sizeof(float);
				case ShaderUniformType::Float2: return sizeof(float) * 2;
				case ShaderUniformType::Float3: return sizeof(float) * 3;
				case ShaderUniformType::Float4: return sizeof(float) * 4;
				case ShaderUniformType::Float4x4: return sizeof(float) * 4 * 4;
			}

			return 0;
		}

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

		inline static const ElementType GetBufferElementTypeFromSPIRV(const spirv_cross::SPIRType& type)
		{
			if (type.columns == 4)
			{
				switch (type.basetype)
				{
					case spirv_cross::SPIRType::Float: return ElementType::Float4x4;
				}
			}
			else
			{
				switch (type.basetype)
				{
					case spirv_cross::SPIRType::Boolean: return ElementType::Bool;
					case spirv_cross::SPIRType::UInt:
					{
						if (type.vecsize == 1) return ElementType::UInt;
						if (type.vecsize == 2) return ElementType::UInt2;
						if (type.vecsize == 3) return ElementType::UInt3;
						if (type.vecsize == 4) return ElementType::UInt4;

						break;
					}

					case spirv_cross::SPIRType::Int:
					{
						if (type.vecsize == 1) return ElementType::Int;
						if (type.vecsize == 2) return ElementType::Int2;
						if (type.vecsize == 3) return ElementType::Int3;
						if (type.vecsize == 4) return ElementType::Int4;

						break;
					}

					case spirv_cross::SPIRType::Float:
					{
						if (type.vecsize == 1) return ElementType::Float;
						if (type.vecsize == 2) return ElementType::Float2;
						if (type.vecsize == 3) return ElementType::Float3;
						if (type.vecsize == 4) return ElementType::Float4;

						break;
					}

					case spirv_cross::SPIRType::Half:
					{
						if (type.vecsize == 1) return ElementType::Half;
						if (type.vecsize == 2) return ElementType::Half2;
						if (type.vecsize == 3) return ElementType::Half3;
						if (type.vecsize == 4) return ElementType::Half4;
					
						break;
					}
				}
			}

			return ElementType::Float;
		}

		inline static std::any GetDefaultValueFromSPIRVConstant(const spirv_cross::SPIRConstant& constant, ShaderUniformType type)
		{
			switch (type)
			{
				case ShaderUniformType::Bool: return constant.scalar_i8();

				case ShaderUniformType::UInt: return constant.scalar();
				case ShaderUniformType::UInt2: return glm::uvec2{ constant.scalar(0), constant.scalar(1) };
				case ShaderUniformType::UInt3: return glm::uvec3{ constant.scalar(0), constant.scalar(1), constant.scalar(2) };
				case ShaderUniformType::UInt4: return glm::uvec4{ constant.scalar(0), constant.scalar(1), constant.scalar(2), constant.scalar(3) };

				case ShaderUniformType::Int: return constant.scalar_i32();
				case ShaderUniformType::Int2: return glm::ivec2{ constant.scalar_i32(0), constant.scalar_i32(1) };
				case ShaderUniformType::Int3: return glm::ivec3{ constant.scalar_i32(0), constant.scalar_i32(1), constant.scalar_i32(2) };
				case ShaderUniformType::Int4: return glm::ivec4{ constant.scalar_i32(0), constant.scalar_i32(1), constant.scalar_i32(2), constant.scalar_i32(3) };

				case ShaderUniformType::Float: return constant.scalar_f32();
				case ShaderUniformType::Float2: return glm::vec2{ constant.scalar_f32(0), constant.scalar_f32(1) };
				case ShaderUniformType::Float3: return glm::vec3{ constant.scalar_f32(0), constant.scalar_f32(1), constant.scalar_f32(2) };
				case ShaderUniformType::Float4: return glm::vec4{ constant.scalar_f32(0), constant.scalar_f32(1), constant.scalar_f32(2), constant.scalar_f32(3) };
			}

			return 0;
		}

		inline static const ImageDimension GetDimensionFromSPIRV(const spv::Dim dimension)
		{
			switch (dimension)
			{
				case spv::Dim1D: return ImageDimension::Dim1D;
				case spv::Dim2D: return ImageDimension::Dim2D;
				case spv::Dim3D: return ImageDimension::Dim3D;
				case spv::Dim::DimCube: return ImageDimension::DimCube; break;
			}

			return ImageDimension::Dim2D;
		}

		inline static VkWriteDescriptorSet CreateWriteDescriptor(VkDescriptorType descriptorType, uint32_t binding)
		{
			VkWriteDescriptorSet writeDescriptor;
			writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptor.pNext = nullptr;
			writeDescriptor.descriptorCount = 1;
			writeDescriptor.dstArrayElement = 0;
			writeDescriptor.dstBinding = binding;
			writeDescriptor.descriptorType = descriptorType;
			writeDescriptor.pBufferInfo = nullptr;
			writeDescriptor.pImageInfo = nullptr;
			writeDescriptor.pTexelBufferView = nullptr;

			return writeDescriptor;
		}

		inline static bool IsBindlessDescriptorBinding(const VkDescriptorSetLayoutBinding& binding)
		{
			if (binding.descriptorCount > 1 && (binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER))
			{
				return true;
			}

			return false;
		}
	}

	Shader::Shader(const std::string& name, const std::vector<std::filesystem::path>& shaderFiles, bool forceCompile)
		: myName(name), myShaderFiles(shaderFiles)
	{

		if (shaderFiles.empty())
		{
			VT_CORE_ERROR("Trying to create a shader {0} without sources!", name);
			return;
		}

		Reload(forceCompile);
		GenerateHash();
	}

	Shader::~Shader()
	{
		Release();
	}

	const bool Shader::Reload(bool forceCompile)
	{
		Utility::CreateCacheDirectoryIfNeeded();

		myShaderSources.clear();
		LoadShaderFromFiles();

		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> shaderData;
		if (!CompileOrGetBinary(shaderData, forceCompile))
		{
			return false;
		}

		Release();
		LoadAndCreateShaders(shaderData);
		ReflectAllStages(shaderData);

		return true;
	}

	VkDescriptorSet Shader::AllocateDescriptorSet(uint32_t set, VkDescriptorPool pool)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &myResources.nullPaddedDescriptorSetLayouts.at(set);

		//VkDescriptorSet descriptorSet;
		//VT_VK_CHECK(vkAllocateDescriptorSets(GraphicsContextVolt::GetDevice()->GetHandle(), &allocInfo, &descriptorSet));

		return nullptr;
	}

	const ShaderDataBuffer Shader::CreateShaderBuffer()
	{
		ShaderDataBuffer newBuffer = myResources.pushConstantBuffer;
		newBuffer.myDataBuffer.Allocate(newBuffer.mySize);
		newBuffer.myDataBuffer.Clear();

		return newBuffer;
	}

	const ShaderDataBuffer Shader::CreateSpecialiazationConstantsBuffer(ShaderStage shaderStage)
	{
		if (!myResources.specializationConstantBuffers.contains((VkShaderStageFlagBits)shaderStage))
		{
			VT_CORE_WARN("[VulkanShader] Trying to create specialization constant buffer of invalid stage!");
			return {};
		}

		ShaderDataBuffer newBuffer = myResources.specializationConstantBuffers.at((VkShaderStageFlagBits)shaderStage);
		newBuffer.myDataBuffer.Allocate(newBuffer.mySize);
		newBuffer.myDataBuffer.Clear();

		for (const auto& [constantId, constant] : myResources.specializationConstants.at((VkShaderStageFlagBits)shaderStage))
		{
			switch (constant.type)
			{
				case ShaderUniformType::Bool: newBuffer.SetValue(constant.name, std::any_cast<int8_t>(constant.defaultValue)); break;

				case ShaderUniformType::UInt: newBuffer.SetValue(constant.name, std::any_cast<uint32_t>(constant.defaultValue)); break;
				case ShaderUniformType::UInt2: newBuffer.SetValue(constant.name, std::any_cast<glm::uvec2>(constant.defaultValue)); break;
				case ShaderUniformType::UInt3: newBuffer.SetValue(constant.name, std::any_cast<glm::uvec3>(constant.defaultValue)); break;
				case ShaderUniformType::UInt4: newBuffer.SetValue(constant.name, std::any_cast<glm::uvec4>(constant.defaultValue)); break;

				case ShaderUniformType::Int: newBuffer.SetValue(constant.name, std::any_cast<int32_t>(constant.defaultValue)); break;
				case ShaderUniformType::Int2: newBuffer.SetValue(constant.name, std::any_cast<glm::ivec2>(constant.defaultValue)); break;
				case ShaderUniformType::Int3: newBuffer.SetValue(constant.name, std::any_cast<glm::ivec3>(constant.defaultValue)); break;
				case ShaderUniformType::Int4: newBuffer.SetValue(constant.name, std::any_cast<glm::ivec4>(constant.defaultValue)); break;

				case ShaderUniformType::Float: newBuffer.SetValue(constant.name, std::any_cast<float>(constant.defaultValue)); break;
				case ShaderUniformType::Float2: newBuffer.SetValue(constant.name, std::any_cast<glm::vec2>(constant.defaultValue)); break;
				case ShaderUniformType::Float3: newBuffer.SetValue(constant.name, std::any_cast<glm::vec3>(constant.defaultValue)); break;
				case ShaderUniformType::Float4: newBuffer.SetValue(constant.name, std::any_cast<glm::vec4>(constant.defaultValue)); break;
			}
		}

		return newBuffer;
	}

	Ref<Shader> Shader::Create(const std::string& name, std::vector<std::filesystem::path> paths, bool forceCompile)
	{
		return CreateRef<Shader>(name, paths, forceCompile);
	}

	void Shader::GenerateHash()
	{
		size_t hash = std::hash<std::string>()(myName);
		for (const auto& path : myShaderFiles)
		{
			size_t pathHash = std::filesystem::hash_value(path);
			hash = Utility::HashCombine(hash, pathHash);
		}

		myHash = hash;
	}

	void Shader::LoadShaderFromFiles()
	{
		for (const auto& path : myShaderFiles)
		{
			const VkShaderStageFlagBits stage = Utility::GetShaderStageFromFilename(path.filename().string());
			const std::string source = Utility::ReadStringFromFile(AssetManager::GetContextPath(path) / path);

			if (source.empty())
			{
				continue;
			}

			if (myShaderSources.find(stage) != myShaderSources.end())
			{
				VT_CORE_ERROR("Multiple shaders of same stage defined in file {0}!", path.string().c_str());
				return;
			}

			myShaderSources[stage] = source;
		}
	}

	void Shader::Release()
	{
		//auto device = GraphicsContextVolt::GetDevice();

		//for (const auto& stage : myPipelineShaderStageInfos)
		//{
		//	vkDestroyShaderModule(device->GetHandle(), stage.module, nullptr);
		//}

		//for (const auto& descriptorSetLayout : myResources.nullPaddedDescriptorSetLayouts)
		//{
		//	vkDestroyDescriptorSetLayout(device->GetHandle(), descriptorSetLayout, nullptr);
		//}

		myPipelineShaderStageInfos.clear();

		auto oldTextureDefinitions = myResources.shaderTextureDefinitions;
		myResources = {};
		myResources.shaderTextureDefinitions = oldTextureDefinitions;
		myResources.pushConstantBuffer = {};
		myResources.specializationConstantBuffers.clear();

		myPerStageUBOCount.clear();
		myPerStageDynamicUBOCount.clear();
		myPerStageSSBOCount.clear();
		myPerStageStorageImageCount.clear();
		myPerStageImageCount.clear();
		myPerStageSeperateImageCount.clear();
		myPerStageSamplerCount.clear();
	}

	const bool Shader::CompileOrGetBinary(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData, bool forceCompile)
	{
		auto cacheDirectory = Utility::GetShaderCacheDirectory();

		for (const auto& [stage, source] : myShaderSources)
		{
			auto extension = Utility::GetShaderStageCachedFileExtension(stage);
			auto& data = outShaderData[stage];

			std::filesystem::path currentStagePath;
			for (const auto& shaderPath : myShaderFiles)
			{
				if (stage == Utility::GetShaderStageFromFilename(shaderPath.filename().string()))
				{
					currentStagePath = shaderPath;
					break;
				}
			}

			auto cachedPath = cacheDirectory / (currentStagePath.filename().string() + extension);
			if (!forceCompile)
			{
				std::ifstream file(cachedPath.string(), std::ios::binary | std::ios::in | std::ios::ate);
				if (file.is_open())
				{
					const uint64_t size = file.tellg();
					data.resize(size / sizeof(uint32_t)); // We store the data as 4 byte blocks

					file.seekg(0, std::ios::beg);
					file.read((char*)data.data(), size);
					file.close();
				}
			}

			// Check if we should recompile (shader files are out of date)
			{
				if (std::filesystem::exists(cachedPath) && std::filesystem::exists(currentStagePath))
				{
					const auto cachedWriteTime = std::filesystem::last_write_time(cachedPath);
					const auto sourceWriteTime = std::filesystem::last_write_time(currentStagePath);

					if (cachedWriteTime < sourceWriteTime)
					{
						VT_CORE_TRACE("Compiling shader {0} because it was out of date!", myName);
					}
				}

			}
		}

		return true;
	}

	void Shader::LoadAndCreateShaders(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData)
	{
		//auto device = GraphicsContextVolt::GetDevice();
		myPipelineShaderStageInfos.clear();
		myShaderGroupInfos.clear();

		for (const auto& [stage, data] : shaderData)
		{
			VkShaderModuleCreateInfo moduleInfo{};
			moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleInfo.codeSize = data.size() * sizeof(uint32_t);
			moduleInfo.pCode = data.data();

			VkShaderModule shaderModule{};
			//VT_VK_CHECK(vkCreateShaderModule(device->GetHandle(), &moduleInfo, nullptr, &shaderModule));

			VkPipelineShaderStageCreateInfo& shaderStage = myPipelineShaderStageInfos.emplace_back();
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = stage;
			shaderStage.module = shaderModule;
			shaderStage.pName = "main";

			switch (stage)
			{
				case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
				{
					auto& groupInfo = myShaderGroupInfos.emplace_back();
					groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
					groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
					groupInfo.generalShader = static_cast<uint32_t>(myPipelineShaderStageInfos.size() - 1);
					groupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
					groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
					groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
					break;
				}

				case VK_SHADER_STAGE_MISS_BIT_KHR:
				{
					auto& groupInfo = myShaderGroupInfos.emplace_back();
					groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
					groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
					groupInfo.generalShader = static_cast<uint32_t>(myPipelineShaderStageInfos.size() - 1);
					groupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
					groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
					groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
					break;
				}

				case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
				{
					auto& groupInfo = myShaderGroupInfos.emplace_back();
					groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
					groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
					groupInfo.generalShader = VK_SHADER_UNUSED_KHR;
					groupInfo.closestHitShader = static_cast<uint32_t>(myPipelineShaderStageInfos.size() - 1);;
					groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
					groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
					break;
				}
			}
		}
	}

	void Shader::ReflectAllStages(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData)
	{
		VT_CORE_INFO("Shader - Reflecting {0}", myName.c_str());
		for (const auto& [stage, data] : shaderData)
		{
			ReflectStage(stage, data);
		}

		CalculateDescriptorPoolSizes();
		CreateDescriptorSetLayouts();
	}

	void Shader::ReflectStage(VkShaderStageFlagBits stage, const std::vector<uint32_t>& data)
	{
		//auto device = GraphicsContextVolt::GetDevice();

		VT_CORE_INFO("	Reflecting stage {0}", Utility::StageToString(stage).c_str());
		spirv_cross::Compiler compiler(data);
		const auto resources = compiler.get_shader_resources();

		for (const auto& ubo : resources.uniform_buffers)
		{
			if (compiler.get_active_buffer_ranges(ubo.id).empty())
			{
				continue;
			}

			const auto& bufferType = compiler.get_type(ubo.base_type_id);

			const uint32_t binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
			const size_t size = compiler.get_declared_struct_size(bufferType);
			const std::string name = compiler.get_name(ubo.id);
			const bool isDynamic = false;

			VT_CORE_ASSERT(name != "$Globals", "Shader seems to have incorrectly defined global variables!");

			// Create descriptor binding
			{
				auto& descriptorSetBindings = myResources.descriptorSetBindings;
				auto it = std::find_if(descriptorSetBindings[set].begin(), descriptorSetBindings[set].end(), [binding](const auto& lhs) { return lhs.binding == binding; });

				if (it == descriptorSetBindings[set].end())
				{
					auto& vkBinding = myResources.descriptorSetBindings[set].emplace_back();
					vkBinding.binding = binding;
					vkBinding.descriptorCount = 1;
					vkBinding.descriptorType = isDynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					vkBinding.stageFlags = stage;
				}
				else
				{
					(*it).stageFlags |= stage;
				}
			}

			// Add Uniform Buffer
			{
				auto& uniformBuffer = myResources.uniformBuffers[set][binding];
				uniformBuffer.info.offset = 0;
				uniformBuffer.info.range = size;
				uniformBuffer.name = name;

				if (isDynamic)
				{
					//const uint64_t minUBOAlignment = GraphicsContextVolt::GetPhysicalDevice()->GetCapabilities().minUBOOffsetAlignment;
					uint32_t dynamicAlignment = (uint32_t)size;

					//if (minUBOAlignment > 0)
					//{
					//	dynamicAlignment = (uint32_t)Utility::GetAlignedSize((uint64_t)dynamicAlignment, minUBOAlignment);
					//}

					uniformBuffer.isDynamic = true;
					uniformBuffer.info.range = dynamicAlignment;
				}

				myPerStageUBOCount[stage].count++;
			}

			myResources.writeDescriptors[set][binding] = Utility::CreateWriteDescriptor(isDynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding);
		}

		for (const auto& ssbo : resources.storage_buffers)
		{
			if (compiler.get_active_buffer_ranges(ssbo.id).empty())
			{
				continue;
			}

			const auto& bufferType = compiler.get_type(ssbo.base_type_id);

			const uint32_t binding = compiler.get_decoration(ssbo.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(ssbo.id, spv::DecorationDescriptorSet);
			const size_t size = compiler.get_declared_struct_size(bufferType);

			// Create descriptor binding
			{
				auto& descriptorSetBindings = myResources.descriptorSetBindings;
				auto it = std::find_if(descriptorSetBindings[set].begin(), descriptorSetBindings[set].end(), [binding](const auto& lhs) { return lhs.binding == binding; });

				if (it == descriptorSetBindings[set].end())
				{
					auto& vkBinding = descriptorSetBindings[set].emplace_back();
					vkBinding.binding = binding;
					vkBinding.descriptorCount = 1;
					vkBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					vkBinding.stageFlags = stage;
				}
				else
				{
					(*it).stageFlags |= stage;
				}
			}

			// Add SSBO
			{
				auto& storageBuffer = myResources.shaderStorageBuffers[set][binding];
				storageBuffer.info.offset = 0;
				storageBuffer.info.range = size;

				myPerStageSSBOCount[stage].count++;
			}

			myResources.writeDescriptors[set][binding] = Utility::CreateWriteDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding);
		}

		for (const auto& image : resources.storage_images)
		{
			const uint32_t binding = compiler.get_decoration(image.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);

			// Create storage images
			{
				auto& descriptorSetBindings = myResources.descriptorSetBindings;
				auto it = std::find_if(descriptorSetBindings[set].begin(), descriptorSetBindings[set].end(), [binding](const auto& lhs) { return lhs.binding == binding; });

				if (it == descriptorSetBindings[set].end())
				{
					auto& vkBinding = descriptorSetBindings[set].emplace_back();
					vkBinding.binding = binding;
					vkBinding.descriptorCount = 1;
					vkBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					vkBinding.stageFlags = stage;
				}
				else
				{
					(*it).stageFlags |= stage;
				}
			}

			// Add Storage Image
			{
				const auto& imageType = compiler.get_type(image.type_id);

				auto& storageImage = myResources.storageImages[set][binding];
				storageImage.info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				storageImage.dimension = Utility::GetDimensionFromSPIRV(imageType.image.dim);

				myPerStageStorageImageCount[stage].count++;
			}

			myResources.writeDescriptors[set][binding] = Utility::CreateWriteDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, binding);

			if (set == Sets::MATERIAL)
			{
				myResources.materialWriteDescriptors.emplace_back(Utility::CreateWriteDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, binding));
			}
		}

		for (const auto& sampledImage : resources.sampled_images)
		{
			const uint32_t binding = compiler.get_decoration(sampledImage.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);

			// Create sampled images
			{
				auto& descriptorSetBindings = myResources.descriptorSetBindings;
				auto it = std::find_if(descriptorSetBindings[set].begin(), descriptorSetBindings[set].end(), [binding](const auto& lhs) { return lhs.binding == binding; });

				if (it == descriptorSetBindings[set].end())
				{
					auto& vkBinding = descriptorSetBindings[set].emplace_back();
					vkBinding.binding = binding;
					vkBinding.descriptorCount = 1;
					vkBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					vkBinding.stageFlags = stage;
				}
				else
				{
					(*it).stageFlags |= stage;
				}
			}

			// Add Sampled Image
			{
				const auto& imageType = compiler.get_type(sampledImage.type_id);

				auto& sampledImageResource = myResources.sampledImages[set][binding];
				sampledImageResource.info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				sampledImageResource.dimension = Utility::GetDimensionFromSPIRV(imageType.image.dim);

				myPerStageImageCount[stage].count++;
			}

			myResources.writeDescriptors[set][binding] = Utility::CreateWriteDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding);

			if (set == Sets::MATERIAL)
			{
				myResources.materialWriteDescriptors.emplace_back(Utility::CreateWriteDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding));
			}
		}

		for (const auto& seperateImage : resources.separate_images)
		{
			const uint32_t binding = compiler.get_decoration(seperateImage.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(seperateImage.id, spv::DecorationDescriptorSet);
			const auto& imageType = compiler.get_type(seperateImage.type_id);

			// Create sampled images
			{
				auto& descriptorSetBindings = myResources.descriptorSetBindings;
				auto it = std::find_if(descriptorSetBindings[set].begin(), descriptorSetBindings[set].end(), [binding](const auto& lhs) { return lhs.binding == binding; });

				if (it == descriptorSetBindings[set].end())
				{
					uint32_t descriptorCount = 1;

					if (!imageType.array.empty())
					{
						descriptorCount = imageType.array[0];
					}

					auto& vkBinding = descriptorSetBindings[set].emplace_back();
					vkBinding.binding = binding;
					vkBinding.descriptorCount = descriptorCount;
					vkBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
					vkBinding.stageFlags = stage;
				}
				else
				{
					(*it).stageFlags |= stage;
				}
			}

			// Add seperate Image
			{
				auto& seperateImageResource = myResources.separateImages[set][binding];
				seperateImageResource.info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				seperateImageResource.dimension = Utility::GetDimensionFromSPIRV(imageType.image.dim);
				seperateImageResource.name = compiler.get_name(seperateImage.id);

				myPerStageSeperateImageCount[stage].count++;
			}

			myResources.writeDescriptors[set][binding] = Utility::CreateWriteDescriptor(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, binding);

			if (set == Sets::MATERIAL)
			{
				myResources.materialWriteDescriptors.emplace_back(Utility::CreateWriteDescriptor(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, binding));
			}
		}

		for (const auto& sampler : resources.separate_samplers)
		{
			const uint32_t binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);

			// Create sampler binding
			{
				auto& descriptorSetBindings = myResources.descriptorSetBindings;
				auto it = std::find_if(descriptorSetBindings[set].begin(), descriptorSetBindings[set].end(), [binding](const auto& lhs) { return lhs.binding == binding; });

				if (it == descriptorSetBindings[set].end())
				{
					auto& vkBinding = descriptorSetBindings[set].emplace_back();
					vkBinding.binding = binding;
					vkBinding.descriptorCount = 1;
					vkBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
					vkBinding.stageFlags = stage;
				}
				else
				{
					(*it).stageFlags |= stage;
				}
			}

			// Add Sampler
			{
				auto& samplerResource = myResources.samplers[set][binding];
				samplerResource.info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				myPerStageSamplerCount[stage].count++;
			}

			myResources.writeDescriptors[set][binding] = Utility::CreateWriteDescriptor(VK_DESCRIPTOR_TYPE_SAMPLER, binding);
		}

		uint32_t specializationConstantOffset = 0;
		for (const auto& constant : compiler.get_specialization_constants())
		{
			const uint32_t constantId = constant.constant_id;
			VT_CORE_ASSERT(!myResources.specializationConstants[stage].contains(constantId), "Specialization constant has already been defined!");

			const auto& spirConstant = compiler.get_constant(constant.id);
			const auto& constantType = compiler.get_type(spirConstant.constant_type);
			const auto uniformType = Utility::GetShaderUniformTypeFromSPIRV(constantType);

			auto& specConstant = myResources.specializationConstants[stage][constantId];
			specConstant.name = compiler.get_name(constant.id);
			specConstant.type = uniformType;
			specConstant.defaultValue = Utility::GetDefaultValueFromSPIRVConstant(spirConstant, uniformType);
			specConstant.constantInfo.constantID = constantId;
			specConstant.constantInfo.size = Utility::GetSizeFromShaderUniformType(uniformType);
			specConstant.constantInfo.offset = specializationConstantOffset;

			myResources.specializationConstantBuffers[stage].AddMember(specConstant.name, specConstant.type, specConstant.constantInfo.size, specConstant.constantInfo.offset);
			myResources.specializationConstantBuffers[stage].SetSize(myResources.specializationConstantBuffers[stage].GetSize() + specConstant.constantInfo.size);
			specializationConstantOffset += (uint32_t)specConstant.constantInfo.size;
		}

		for (const auto& pushConstant : resources.push_constant_buffers)
		{
			const auto& bufferType = compiler.get_type(pushConstant.base_type_id);
			const size_t pushConstantSize = compiler.get_declared_struct_size(bufferType);

			myResources.pushConstantRange.size = (uint32_t)pushConstantSize;
			myResources.pushConstantRange.offset = 0;
			myResources.pushConstantRange.stageFlags |= stage;

			myResources.pushConstantBuffer.SetSize(pushConstantSize);

			for (uint32_t i = 0; const auto & member : bufferType.member_types)
			{
				const auto& memberType = compiler.get_type(member);
				const size_t memberSize = compiler.get_declared_struct_member_size(bufferType, i);
				const size_t memberOffset = compiler.type_struct_member_offset(bufferType, i);
				const std::string& memberName = compiler.get_member_name(pushConstant.base_type_id, i);

				const auto type = Utility::GetShaderUniformTypeFromSPIRV(memberType);

				myResources.pushConstantBuffer.AddMember(memberName, type, memberSize, memberOffset);
				i++;
			}
		}

		if (stage == VK_SHADER_STAGE_VERTEX_BIT)
		{
			std::vector<BufferElement> inputElements;

			for (const auto& input : resources.stage_inputs)
			{
				const auto& bufferType = compiler.get_type(input.base_type_id);
				const std::string& memberName = compiler.get_name(input.base_type_id);
				const auto type = Utility::GetBufferElementTypeFromSPIRV(bufferType);

				inputElements.emplace_back(type, memberName);
			}

			myResources.vertexLayout = { inputElements };
		}

#ifdef VT_SHADER_PRINT
		VT_CORE_INFO("		Uniform Buffers: {0}", myPerStageUBOCount[stage].count);
		VT_CORE_INFO("		Dynamic Uniform Buffers: {0}", myPerStageDynamicUBOCount[stage].count);
		VT_CORE_INFO("		Shader Storage Buffers: {0}", myPerStageSSBOCount[stage].count);
		VT_CORE_INFO("		Sampled Images: {0}", myPerStageImageCount[stage].count);
		VT_CORE_INFO("		Storage Images: {0}", myPerStageStorageImageCount[stage].count);
		VT_CORE_INFO("		Seperate Images: {0}", myPerStageSeperateImageCount[stage].count);
		VT_CORE_INFO("		Seperate Samplers: {0}", myPerStageSamplerCount[stage].count);
#endif
		}

	void Shader::CreateDescriptorSetLayouts()
	{
		//auto device = GraphicsContextVolt::GetDevice();
		constexpr VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		constexpr VkDescriptorBindingFlags materialFlags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

		// Create null binding padded layouts to be able to feed directly into pipeline
		int32_t lastSet = -1;
		for (const auto& [set, bindings] : myResources.descriptorSetBindings)
		{
			while ((int32_t)set > lastSet + 1)
			{
				VkDescriptorSetLayoutCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				info.pNext = nullptr;
				info.bindingCount = 0;
				info.pBindings = nullptr;

				//VT_VK_CHECK(vkCreateDescriptorSetLayout(device->GetHandle(), &info, nullptr, &myResources.nullPaddedDescriptorSetLayouts.emplace_back()));
				lastSet++;
			}

			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.pNext = nullptr;
			info.bindingCount = (uint32_t)bindings.size();
			info.pBindings = bindings.data();

			std::vector<VkDescriptorBindingFlags> bindingFlags{};

			VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
			extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			extendedInfo.bindingCount = info.bindingCount;

			for (const auto& binding : bindings)
			{
				auto& flags = bindingFlags.emplace_back();
				flags = 0;

				if (Utility::IsBindlessDescriptorBinding(binding) && set == 0)
				{
					info.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
					flags = bindlessFlags;
				}
				else if (set == Sets::MATERIAL)
				{
					info.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
					flags = materialFlags;
				}
			}

			extendedInfo.pBindingFlags = bindingFlags.data();
			info.pNext = &extendedInfo;

			//VT_VK_CHECK(vkCreateDescriptorSetLayout(device->GetHandle(), &info, nullptr, &myResources.nullPaddedDescriptorSetLayouts.emplace_back()));
			myResources.descriptorSetLayouts.emplace_back(myResources.nullPaddedDescriptorSetLayouts.back());
			lastSet = set;
		}

		VkDescriptorSetAllocateInfo& allocInfo = myResources.descriptorSetAllocateInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorSetCount = (uint32_t)myResources.descriptorSetLayouts.size();
		allocInfo.pSetLayouts = myResources.descriptorSetLayouts.data();
	}

	void Shader::CalculateDescriptorPoolSizes()
	{
		//const uint32_t framesInFlight = Application::Get().GetWindow().GetSwapchain().GetMaxFramesInFlight();

		uint32_t uboCount = 0;
		uint32_t dynamicUBOCount = 0;
		uint32_t ssboCount = 0;
		uint32_t storageImageCount = 0;
		uint32_t imageCount = 0;
		uint32_t seperateImageCount = 0;
		uint32_t seperateSamplerCount = 0;

		std::for_each(myPerStageUBOCount.begin(), myPerStageUBOCount.end(), [&](auto pair) { uboCount += pair.second.count; });
		std::for_each(myPerStageDynamicUBOCount.begin(), myPerStageDynamicUBOCount.end(), [&](auto pair) { dynamicUBOCount += pair.second.count; });
		std::for_each(myPerStageSSBOCount.begin(), myPerStageSSBOCount.end(), [&](auto pair) { ssboCount += pair.second.count; });
		std::for_each(myPerStageStorageImageCount.begin(), myPerStageStorageImageCount.end(), [&](auto pair) { storageImageCount += pair.second.count; });
		std::for_each(myPerStageImageCount.begin(), myPerStageImageCount.end(), [&](auto pair) { imageCount += pair.second.count; });
		std::for_each(myPerStageSeperateImageCount.begin(), myPerStageSeperateImageCount.end(), [&](auto pair) { seperateImageCount += pair.second.count; });
		std::for_each(myPerStageSamplerCount.begin(), myPerStageSamplerCount.end(), [&](auto pair) { seperateSamplerCount += pair.second.count; });

		//if (uboCount > 0)
		//{
		//	myResources.descriptorPoolSizes.emplace_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uboCount * framesInFlight);
		//}

		//if (dynamicUBOCount > 0)
		//{
		//	myResources.descriptorPoolSizes.emplace_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, dynamicUBOCount * framesInFlight);
		//}

		//if (ssboCount > 0)
		//{
		//	myResources.descriptorPoolSizes.emplace_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, ssboCount * framesInFlight);
		//}

		//if (storageImageCount > 0)
		//{
		//	myResources.descriptorPoolSizes.emplace_back(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, storageImageCount * framesInFlight);
		//}

		//if (imageCount > 0)
		//{
		//	myResources.descriptorPoolSizes.emplace_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount * framesInFlight);
		//}

		//if (seperateImageCount > 0)
		//{
		//	myResources.descriptorPoolSizes.emplace_back(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, seperateImageCount * framesInFlight);
		//}

		//if (seperateSamplerCount > 0)
		//{
		//	myResources.descriptorPoolSizes.emplace_back(VK_DESCRIPTOR_TYPE_SAMPLER, seperateSamplerCount * framesInFlight);
		//}
	}

	///// Shader Uniform /////
	ShaderUniform::ShaderUniform(const ShaderUniformType type, const size_t size, const size_t offset)
		: type(type), size(size), offset(offset)
	{
	}

	ShaderDataBuffer::~ShaderDataBuffer()
	{
		//myDataBuffer.Release();
	}

	void ShaderDataBuffer::Release()
	{
		//myDataBuffer.Release(); // #TODO_Ivar: Memory leak!
	}

	void ShaderDataBuffer::AddMember(const std::string& name, const ShaderUniformType type, const size_t size, const size_t offset)
	{
		myUniforms.emplace(name, ShaderUniform{ type, size, offset });
	}

	void ShaderDataBuffer::SetValue(const std::string& name, const void* data, const size_t size)
	{
		if (!myUniforms.contains(name))
		{
			VT_CORE_WARN("Trying to set uniform {0} but it does not exist!", name);
			return;
		}

		const auto& uniform = myUniforms.at(name);
		if (size != uniform.size)
		{
			VT_CORE_WARN("Trying to set uniform {0} to a value with size {1}, but uniform has a size of {2}", name, size, uniform.size);
			return;
		}

		myDataBuffer.Copy(data, size, uniform.offset);
	}

	const void* ShaderDataBuffer::GetValueRaw(const std::string& name, const size_t size) const
	{
		if (!myUniforms.contains(name))
		{
			VT_CORE_WARN("Trying to get uniform {0} but it does not exist!", name);
			return nullptr;
		}

		const auto& uniform = myUniforms.at(name);
		if (size != uniform.size)
		{
			VT_CORE_WARN("Trying to get uniform {0} to a value with size {1}, but uniform has a size of {2}", name, size, uniform.size);
			return nullptr;
		}

		return myDataBuffer.As<void>(uniform.offset);
	}

	ShaderDataBuffer& ShaderDataBuffer::operator=(const ShaderDataBuffer& lhs)
	{
		myUniforms = lhs.myUniforms;
		mySize = lhs.mySize;

		myDataBuffer.Allocate(lhs.myDataBuffer.GetSize());
		myDataBuffer.Copy(lhs.myDataBuffer.As<void*>(), lhs.myDataBuffer.GetSize());

		return *this;
	}

	const bool ShaderResources::HasUniformBufferAt(uint32_t set, uint32_t binding) const
	{
		return uniformBuffers.contains(set) && uniformBuffers.at(set).contains(binding);
	}

	const bool ShaderResources::HasStorageBufferAt(uint32_t set, uint32_t binding) const
	{
		return shaderStorageBuffers.contains(set) && shaderStorageBuffers.at(set).contains(binding);
	}

	const bool ShaderResources::HasSampledImageAt(uint32_t set, uint32_t binding) const
	{
		return sampledImages.contains(set) && sampledImages.at(set).contains(binding);
	}

	const bool ShaderResources::HasSeparateImageAt(uint32_t set, uint32_t binding) const
	{
		return separateImages.contains(set) && separateImages.at(set).contains(binding);
	}

	const bool ShaderResources::HasStorageImageAt(uint32_t set, uint32_t binding) const
	{
		return storageImages.contains(set) && storageImages.at(set).contains(binding);
	}
	}
