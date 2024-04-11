#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

#include "VoltRHI/Shader/BufferLayout.h"

#include <vector>
#include <filesystem>
#include <map>
#include <set>

namespace Volt::RHI
{
	enum class ShaderUniformBaseType : uint8_t
	{
		Invalid,
		Bool,
		
		Short,
		UShort,

		UInt,
		Int,
		
		Int64,
		UInt64,

		Double,
		Float,
		Half,

		Buffer,
		UniformBuffer,
		Texture,
		RWBuffer,
		RWTexture,
		Sampler
	};

	struct VTRHI_API ShaderUniformType
	{
		ShaderUniformBaseType baseType = ShaderUniformBaseType::Invalid;
		uint32_t vecsize = 1;
		uint32_t columns = 1;

		inline const size_t GetSize() const
		{
			size_t size = 0;

			switch (baseType)
			{
				case ShaderUniformBaseType::Bool: size = 4; break; // HLSL Bool size
				case ShaderUniformBaseType::UInt: size = 4; break;
				case ShaderUniformBaseType::Int: size = 4; break;
				case ShaderUniformBaseType::Float: size = 4; break;
				case ShaderUniformBaseType::Half: size = 2; break;
				case ShaderUniformBaseType::Short: size = 2; break;
				case ShaderUniformBaseType::UShort: size = 2; break;
				case ShaderUniformBaseType::Double: size = 8; break;
				case ShaderUniformBaseType::Int64: size = 8; break;
				case ShaderUniformBaseType::UInt64: size = 8; break;

				case ShaderUniformBaseType::Buffer: size = 4; break;
				case ShaderUniformBaseType::UniformBuffer: size = 4; break;
				case ShaderUniformBaseType::Texture: size = 4; break;
				case ShaderUniformBaseType::RWBuffer: size = 4; break;
				case ShaderUniformBaseType::RWTexture: size = 4; break;
				case ShaderUniformBaseType::Sampler: size = 4; break;
 			}

			size = size * vecsize * columns;

			return size;
		}

		inline bool operator==(const ShaderUniformType& rhs)
		{
			return baseType == rhs.baseType && vecsize == rhs.vecsize && columns == rhs.columns;
		}
	};

	struct VTRHI_API ShaderUniform
	{
		ShaderUniform(ShaderUniformType type, size_t size, size_t offset);
		ShaderUniform() = default;
		~ShaderUniform() = default;

		ShaderUniformType type;

		size_t size = 0;
		size_t offset = 0;
	};

	class VTRHI_API ShaderDataBuffer
	{
	public:
		void AddMember(const std::string& name, ShaderUniformType type, size_t size, size_t offset);
		void SetSize(const size_t size);

		[[nodiscard]] inline const bool HasMember(const std::string& memberName) const { return !m_uniforms.contains(memberName); }
		[[nodiscard]] inline const bool IsValid() const { return !m_uniforms.empty(); }

		[[nodiscard]] inline const ShaderUniform& GetMember(const std::string& memberName) const { return m_uniforms.at(memberName); }
		[[nodiscard]] inline const size_t GetSize() const { return m_size; }
		[[nodiscard]] inline const uint8_t* GetBuffer() const { return m_data; }

		std::unordered_map<std::string, ShaderUniform>::iterator begin() { return m_uniforms.begin(); }
		std::unordered_map<std::string, ShaderUniform>::iterator end() { return m_uniforms.end(); }

		template<typename T>
		T& GetMemberData(const std::string& memberName);

		template<typename T>
		void SetMemberData(const std::string& memberName, const T& value);

	private:
		std::unordered_map<std::string, ShaderUniform> m_uniforms;
		uint8_t m_data[128]; // Max push constant size for all platforms are 128 bytes
		size_t m_size = 0;
	};

	struct VTRHI_API ShaderConstantData
	{
		uint32_t size = 0;
		uint32_t offset = 0;
		ShaderStage	stageFlags = ShaderStage::None;
	};

	struct VTRHI_API ShaderRenderGraphConstantsData
	{
		inline bool IsValid() const { return !uniforms.empty() && size > 0; }

		std::unordered_map<std::string, ShaderUniform> uniforms;
		size_t size = 0;
	};

	// Representation of shader types
	struct ShaderConstantBuffer
	{
		ShaderStage usageStages;
		size_t size = 0;
	};

	struct ShaderStorageBuffer
	{
		ShaderStage usageStages;
		size_t size = 0;
		int32_t arraySize = 1; // -1 Means unsized array
	};

	struct ShaderStorageImage
	{
		ShaderStage usageStages;
		int32_t arraySize = 1; // -1 Means unsized array
	};

	struct ShaderImage
	{
		ShaderStage usageStages;
		int32_t arraySize = 1; // -1 Means unsized array
	};

	struct ShaderSampler
	{
		ShaderStage usageStages;
	};
	/////////////////////////////////

	struct VTRHI_API ShaderResourceBinding
	{
		uint32_t set = std::numeric_limits<uint32_t>::max();
		uint32_t binding = std::numeric_limits<uint32_t>::max();

		inline const bool IsValid() const { return set != std::numeric_limits<uint32_t>::max() && binding != std::numeric_limits<uint32_t>::max(); }
	};

	struct VTRHI_API ShaderResources
	{
		std::map<uint32_t, std::map<uint32_t, ShaderConstantBuffer>> uniformBuffers;
		std::map<uint32_t, std::map<uint32_t, ShaderStorageBuffer>> storageBuffers;
		std::map<uint32_t, std::map<uint32_t, ShaderStorageImage>> storageImages;
		std::map<uint32_t, std::map<uint32_t, ShaderImage>> images;
		std::map<uint32_t, std::map<uint32_t, ShaderSampler>> samplers;
		std::set<uint32_t> usedSets{};

		std::unordered_map<std::string, ShaderResourceBinding> bindings;

		ShaderConstantData constants{};
		ShaderDataBuffer constantsBuffer{};
		ShaderRenderGraphConstantsData renderGraphConstantsData{};
		BufferLayout vertexLayout{};
		BufferLayout instanceLayout{};
	
		std::vector<PixelFormat> outputFormats;
	};

	struct ShaderSpecification
	{
		std::string_view name;
		std::string_view entryPoint;
		std::vector<std::filesystem::path> sourceFiles;
		std::vector<std::string> permutations;
		bool forceCompile = false;
	};

	class VTRHI_API Shader : public RHIInterface
	{
	public:
		virtual const bool Reload(bool forceCompile) = 0;
		virtual std::string_view GetName() const = 0;
		virtual const ShaderResources& GetResources() const = 0;
		virtual const std::vector<std::filesystem::path>& GetSourceFiles() const = 0;
		virtual ShaderDataBuffer GetConstantsBuffer() const = 0;
		virtual const ShaderResourceBinding& GetResourceBindingFromName(std::string_view name) const = 0;

		static Ref<Shader> Create(const ShaderSpecification& createInfo);

	protected:
		Shader() = default;
		virtual ~Shader() = default;

	};

	template<typename T>
	T& ShaderDataBuffer::GetMemberData(const std::string& memberName)
	{
		return *reinterpret_cast<T*>(&m_data[m_uniforms.at(memberName).offset]);
	}

	template<typename T>
	void ShaderDataBuffer::SetMemberData(const std::string& memberName, const T& value)
	{
		*reinterpret_cast<T*>(&m_data[m_uniforms.at(memberName).offset]) = value;
	}
}
