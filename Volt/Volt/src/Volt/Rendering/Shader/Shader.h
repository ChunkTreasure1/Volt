#pragma once

#include "Volt/Core/Buffer.h"

#include "Volt/Asset/Asset.h"
#include "Volt/Rendering/Texture/ImageCommon.h"
#include "Volt/Rendering/Shader/ShaderCommon.h"
#include "Volt/Rendering/Buffer/BufferLayout.h"

#include <vulkan/vulkan.h>
#include <string>
#include <any>

namespace Volt
{
	enum class ShaderUniformType
	{
		Invalid,
		Bool,

		UInt,
		UInt2,
		UInt3,
		UInt4,

		Int,
		Int2,
		Int3,
		Int4,

		Float,
		Float2,
		Float3,
		Float4,

		Float4x4
	};

	struct ShaderUniform
	{
		ShaderUniform(const ShaderUniformType type, const size_t size, const size_t offset);
		~ShaderUniform() = default;

		size_t size = 0;
		size_t offset = 0;
		ShaderUniformType type;
	};

	class ShaderDataBuffer
	{
	public:
		ShaderDataBuffer() = default;
		~ShaderDataBuffer();

		void Release();

		void AddMember(const std::string& name, const ShaderUniformType type, const size_t size, const size_t offset);

		inline const std::unordered_map<std::string, ShaderUniform>& GetMembers() const { return myUniforms; }
		inline const uint32_t GetMemberCount() const { return (uint32_t)myUniforms.size(); }
		inline const bool IsValid() const { return !myUniforms.empty(); }
		inline const bool HasMember(const std::string& memberName) const { return myUniforms.contains(memberName); }
		inline const ShaderUniform& GetMember(const std::string& memberName) const { return myUniforms.at(memberName); }

		inline void SetSize(const size_t size) { mySize = size; }
		inline const void* GetData() const { return myDataBuffer.As<void>(); }
		inline const size_t GetSize() const { return mySize; }

		void SetValue(const std::string& name, const void* data, const size_t size);
		const void* GetValueRaw(const std::string& name, const size_t size) const;

		template<typename T>
		void SetValue(const std::string& name, const T& value);

		template<typename T>
		const T& GetValue(const std::string& name) const;

		template<typename T>
		T& GetValue(const std::string& name);

		ShaderDataBuffer& operator=(const ShaderDataBuffer& lhs);

	private:
		friend class Shader;

		std::unordered_map<std::string, ShaderUniform> myUniforms;
		Buffer myDataBuffer{}; // #TODO_Ivar: Memory leak
		size_t mySize = 0;
	};

	struct ShaderSSBO
	{
		VkDescriptorBufferInfo info{};
	};

	struct ShaderUniformBuffer
	{
		VkDescriptorBufferInfo info{};
		bool isDynamic = false;
		std::string name;
	};

	struct ShaderStorageImage
	{
		VkDescriptorImageInfo info{};
		ImageDimension dimension{};
	};

	struct ShaderSampledImage
	{
		VkDescriptorImageInfo info{};
		ImageDimension dimension{};
	};

	struct ShaderSeperateImage
	{
		VkDescriptorImageInfo info{};
		ImageDimension dimension{};
		std::string name;
	};

	struct ShaderSampler
	{
		VkDescriptorImageInfo info{};
	};

	struct ShaderSpecializationConstant
	{
		std::string name;
		std::any defaultValue;
		ShaderUniformType type;
		VkSpecializationMapEntry constantInfo{};
	};

	struct ShaderTexture
	{
		std::string shaderName;
		std::string editorName;
	};

	struct ShaderResources
	{
		std::vector<VkDescriptorSetLayout> nullPaddedDescriptorSetLayouts;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		std::vector<VkDescriptorPoolSize> descriptorPoolSizes;

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;

		std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptorSetBindings{};
		std::map<uint32_t, std::map<uint32_t, VkWriteDescriptorSet>> writeDescriptors{};

		std::map<uint32_t, std::map<uint32_t, ShaderUniformBuffer>> uniformBuffers{};
		std::map<uint32_t, std::map<uint32_t, ShaderSSBO>> shaderStorageBuffers{};
		std::map<uint32_t, std::map<uint32_t, ShaderStorageImage>> storageImages{};
		std::map<uint32_t, std::map<uint32_t, ShaderSampledImage>> sampledImages{};
		std::map<uint32_t, std::map<uint32_t, ShaderSeperateImage>> separateImages{};
		std::map<uint32_t, std::map<uint32_t, ShaderSampler>> samplers;

		std::vector<VkWriteDescriptorSet> materialWriteDescriptors{};

		std::map<VkShaderStageFlagBits, std::map<uint32_t, ShaderSpecializationConstant>> specializationConstants{};
		std::vector<ShaderTexture> shaderTextureDefinitions{}; // shader name -> editor name

		VkPushConstantRange pushConstantRange;
		ShaderDataBuffer pushConstantBuffer{};
		std::map<VkShaderStageFlagBits, ShaderDataBuffer> specializationConstantBuffers{};

		BufferLayout vertexLayout;

		const bool HasUniformBufferAt(uint32_t set, uint32_t binding) const;
		const bool HasStorageBufferAt(uint32_t set, uint32_t binding) const;
		const bool HasSampledImageAt(uint32_t set, uint32_t binding) const;
		const bool HasSeparateImageAt(uint32_t set, uint32_t binding) const;
		const bool HasStorageImageAt(uint32_t set, uint32_t binding) const;

		inline ShaderUniformBuffer& GetUniformBufferAt(uint32_t set, uint32_t binding) { return uniformBuffers.at(set).at(binding); }
		inline ShaderSSBO& GetStorageBufferAt(uint32_t set, uint32_t binding) { return shaderStorageBuffers.at(set).at(binding); }
		inline ShaderStorageImage& GetStorageImageAt(uint32_t set, uint32_t binding) { return storageImages.at(set).at(binding); }
		inline ShaderSampledImage& GetSampledImageAt(uint32_t set, uint32_t binding) { return sampledImages.at(set).at(binding); }
		inline ShaderSeperateImage& GetSeparateImageAt(uint32_t set, uint32_t binding) { return separateImages.at(set).at(binding); }
		inline ShaderSampler& GetSamplerAt(uint32_t set, uint32_t binding) { return samplers.at(set).at(binding); }
	};

	class Shader : public Asset
	{
	public:
		enum class Language
		{
			Invalid,
			GLSL,
			HLSL
		};

		Shader() = default;
		~Shader();

		void Initialize(const std::string& name, const std::vector<std::filesystem::path>& shaderFiles, bool forceCompile);
		const bool Reload(bool forceCompile);

		VkDescriptorSet AllocateDescriptorSet(uint32_t set, VkDescriptorPool pool);
		const ShaderDataBuffer CreateShaderBuffer();
		const ShaderDataBuffer CreateSpecialiazationConstantsBuffer(ShaderStage stage);

		inline const std::string& GetName() const { return myName; }
		inline const ShaderResources& GetResources() const { return myResources; }
		inline const auto& GetStageInfos() const { return myPipelineShaderStageInfos; }
		inline const auto& GetShaderGroups() const { return myShaderGroupInfos; }
		inline const std::vector<std::filesystem::path> GetSourcePaths() const { return myShaderFiles; }
		inline const size_t GetHash() const { return myHash; }
		inline const bool IsInternal() const { return myIsInternal; }

		static AssetType GetStaticType() { return AssetType::Shader; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Shader> Create(const std::string& name, std::vector<std::filesystem::path> paths, bool forceCompile = false);

	private:
		friend class ShaderImporter;
		friend class ShaderSerializer;

		struct TypeCount
		{
			uint32_t count = 0;
		};

		void GenerateHash();
		void LoadShaderFromFiles();
		void Release();

		const bool CompileOrGetBinary(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData, bool forceCompile);
		void LoadAndCreateShaders(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData);
		void ReflectAllStages(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData);
		void ReflectStage(VkShaderStageFlagBits stage, const std::vector<uint32_t>& data);

		void CreateDescriptorSetLayouts();
		void CalculateDescriptorPoolSizes();

		std::unordered_map<VkShaderStageFlagBits, std::string> myShaderSources;
		std::vector<std::filesystem::path> myShaderFiles;

		std::vector<VkPipelineShaderStageCreateInfo> myPipelineShaderStageInfos;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> myShaderGroupInfos;
		ShaderResources myResources{};

		std::string myName;
		size_t myHash{};
		bool myIsInternal = false;

		std::unordered_map<VkShaderStageFlagBits, TypeCount> myPerStageUBOCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> myPerStageDynamicUBOCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> myPerStageSSBOCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> myPerStageStorageImageCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> myPerStageImageCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> myPerStageSeperateImageCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> myPerStageSamplerCount;
	};

	template<typename T>
	inline void ShaderDataBuffer::SetValue(const std::string& name, const T& value)
	{
		if (!myUniforms.contains(name))
		{
			VT_CORE_WARN("Trying to set uniform {0} but it does not exist!", name);
			return;
		}

		const auto& uniform = myUniforms.at(name);
		if (sizeof(T) != uniform.size)
		{
			VT_CORE_WARN("Trying to set uniform {0} to a value with size {1}, but uniform has a size of {2}", name, sizeof(T), uniform.size);
			return;
		}

		myDataBuffer.Copy(&value, sizeof(T), uniform.offset);
	}

	template<typename T>
	inline const T& ShaderDataBuffer::GetValue(const std::string& name) const
	{
		static T nullValue{};

		if (!myUniforms.contains(name))
		{
			VT_CORE_WARN("Trying to get uniform {0} but it does not exist!", name);
			return nullValue;
		}

		const auto& uniform = myUniforms.at(name);
		if (sizeof(T) != uniform.size)
		{
			VT_CORE_WARN("Trying to get uniform {0} to a value with size {1}, but uniform has a size of {2}", name, sizeof(T), uniform.size);
			return nullValue;
		}

		return *myDataBuffer.As<T>(uniform.offset);
	}

	template<typename T>
	inline T& ShaderDataBuffer::GetValue(const std::string& name)
	{
		static T nullValue{};

		if (!myUniforms.contains(name))
		{
			VT_CORE_WARN("Trying to get uniform {0} but it does not exist!", name);
			return nullValue;
		}

		const auto& uniform = myUniforms.at(name);
		if (sizeof(T) != uniform.size)
		{
			VT_CORE_WARN("Trying to get uniform {0} to a value with size {1}, but uniform has a size of {2}", name, sizeof(T), uniform.size);
			return nullValue;
		}

		return *myDataBuffer.As<T>(uniform.offset);
	}
}
