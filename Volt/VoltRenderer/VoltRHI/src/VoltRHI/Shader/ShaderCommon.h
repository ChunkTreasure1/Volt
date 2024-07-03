#pragma once

#include "VoltRHI/Core/Core.h"

#include <CoreUtilities/StringHash.h>

#include <cstdint>
#include <unordered_map>

namespace Volt::RHI
{
	enum class ShaderStage : uint32_t
	{
		None = 0,
		Vertex = 0x00000001,
		Pixel = 0x00000010,
		Hull = 0x00000002,
		Domain = 0x00000004,
		Geometry = 0x00000008,
		Compute = 0x00000020,

		RayGen = 0x00000100,
		AnyHit = 0x00000200,
		ClosestHit = 0x00000400,
		Miss = 0x00000800,
		Intersection = 0x00001000,

		Amplification = 0x00000040,
		Mesh = 0x00000080,

		All = Vertex | Pixel | Hull | Domain | Geometry | Compute,
		Common = Vertex | Pixel | Geometry | Compute
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(ShaderStage);

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

		static void Serialize(BinaryStreamWriter& streamWriter, const ShaderUniformType& data)
		{
			streamWriter.Write(data.baseType);
			streamWriter.Write(data.vecsize);
			streamWriter.Write(data.columns);
		}

		static void Deserialize(BinaryStreamReader& streamReader, ShaderUniformType& outData)
		{
			streamReader.Read(outData.baseType);
			streamReader.Read(outData.vecsize);
			streamReader.Read(outData.columns);
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

		static void Serialize(BinaryStreamWriter& streamWriter, const ShaderUniform& data)
		{
			streamWriter.Write(data.type);
			streamWriter.Write(data.size);
			streamWriter.Write(data.offset);
		}

		static void Deserialize(BinaryStreamReader& streamReader, ShaderUniform& outData)
		{
			streamReader.Read(outData.type);
			streamReader.Read(outData.size);
			streamReader.Read(outData.offset);
		}
	};

	class VTRHI_API ShaderDataBuffer
	{
	public:
		ShaderDataBuffer() = default;
		ShaderDataBuffer(const ShaderDataBuffer& rhs);

		void AddMember(const std::string& name, ShaderUniformType type, size_t size, size_t offset);
		void SetSize(const size_t size);

		VT_NODISCARD VT_INLINE const bool HasMember(const std::string& memberName) const { return !m_uniforms.contains(memberName); }
		VT_NODISCARD VT_INLINE const bool IsValid() const { return !m_uniforms.empty(); }

		VT_NODISCARD VT_INLINE const ShaderUniform& GetMember(const std::string& memberName) const { return m_uniforms.at(memberName); }
		VT_NODISCARD VT_INLINE const size_t GetSize() const { return m_size; }
		VT_NODISCARD VT_INLINE const uint8_t* GetBuffer() const { return m_data; }

		VT_NODISCARD VT_INLINE std::unordered_map<std::string, ShaderUniform>::iterator begin() { return m_uniforms.begin(); }
		VT_NODISCARD VT_INLINE std::unordered_map<std::string, ShaderUniform>::iterator end() { return m_uniforms.end(); }

		template<typename T>
		T& GetMemberData(const std::string& memberName);

		template<typename T>
		void SetMemberData(const std::string& memberName, const T& value);

		ShaderDataBuffer& operator=(const ShaderDataBuffer& rhs);

		static void Serialize(BinaryStreamWriter& streamWriter, const ShaderDataBuffer& data);
		static void Deserialize(BinaryStreamReader& streamReader, ShaderDataBuffer& outData);

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

		static void Serialize(BinaryStreamWriter& streamWriter, const ShaderConstantData& data);
		static void Deserialize(BinaryStreamReader& streamReader, ShaderConstantData& outData);
	};

	struct VTRHI_API ShaderRenderGraphConstantsData
	{
		VT_NODISCARD VT_INLINE bool IsValid() const { return !uniforms.empty() && size > 0; }

		std::unordered_map<StringHash, ShaderUniform> uniforms;
		size_t size = 0;

		static void Serialize(BinaryStreamWriter& streamWriter, const ShaderRenderGraphConstantsData& data)
		{
			streamWriter.Write(data.uniforms);
			streamWriter.Write(data.size);
		}

		static void Deserialize(BinaryStreamReader& streamReader, ShaderRenderGraphConstantsData& outData)
		{
			streamReader.Read(outData.uniforms);
			streamReader.Read(outData.size);
		}
	};

	// Representation of shader types
	struct ShaderConstantBuffer
	{
		ShaderStage usageStages;
		size_t size = 0;
		uint32_t usageCount = 0;
	};

	struct ShaderStorageBuffer
	{
		ShaderStage usageStages;
		size_t size = 0;
		int32_t arraySize = 1; // -1 Means unsized array
		uint32_t usageCount = 0;
		bool isWrite = false;
	};

	struct ShaderStorageImage
	{
		ShaderStage usageStages;
		int32_t arraySize = 1; // -1 Means unsized array
		uint32_t usageCount = 0;
	};

	struct ShaderImage
	{
		ShaderStage usageStages;
		int32_t arraySize = 1; // -1 Means unsized array
		uint32_t usageCount = 0;
	};

	struct ShaderSampler
	{
		ShaderStage usageStages;
		uint32_t usageCount = 0;
	};
	/////////////////////////////////

	struct VTRHI_API ShaderResourceBinding
	{
		uint32_t set = std::numeric_limits<uint32_t>::max();
		uint32_t binding = std::numeric_limits<uint32_t>::max();

		inline const bool IsValid() const { return set != std::numeric_limits<uint32_t>::max() && binding != std::numeric_limits<uint32_t>::max(); }

		static void Serialize(BinaryStreamWriter& streamWriter, const ShaderResourceBinding& data);
		static void Deserialize(BinaryStreamReader& streamReader, ShaderResourceBinding& outData);
	};

	struct ShaderSourceEntry
	{
		std::string entryPoint = "main";
		RHI::ShaderStage shaderStage;
		std::filesystem::path filePath;
	};

	struct ShaderSourceInfo
	{
		ShaderSourceEntry sourceEntry;
		std::string source;
	};
}
