#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

#include "VoltRHI/Utility/Buffer.h"

#include "VoltRHI/Shader/BufferLayout.h"

#include <vector>
#include <filesystem>

namespace Volt::RHI
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
		ShaderUniform(ShaderUniformType type, size_t size, size_t offset);
		ShaderUniform() = default;
		~ShaderUniform() = default;

		size_t size = 0;
		size_t offset = 0;
		ShaderUniformType type = ShaderUniformType::Invalid;
	};

	class ShaderDataBuffer
	{
	public:
		void AddMember(std::string_view name, ShaderUniformType type, size_t size, size_t offset);
		void SetSize(const size_t size);
		void Allocate();

		[[nodiscard]] inline const bool HasMember(std::string_view memberName) const { return !m_uniforms.contains(memberName); }
		[[nodiscard]] inline const bool IsValid() const { return !m_uniforms.empty(); }

		[[nodiscard]] inline const ShaderUniform& GetMember(std::string_view memberName) const { return m_uniforms.at(memberName); }
		[[nodiscard]] inline const size_t GetSize() const { return m_size; }
		[[nodiscard]] inline const Buffer& GetBuffer() const { return m_buffer; }

		std::unordered_map<std::string_view, ShaderUniform>::iterator begin() { return m_uniforms.begin(); }
		std::unordered_map<std::string_view, ShaderUniform>::iterator end() { return m_uniforms.end(); }

		template<typename T>
		T& GetMemberData(std::string_view memberName);

	private:
		std::unordered_map<std::string_view, ShaderUniform> m_uniforms;
		Buffer m_buffer{};
		size_t m_size = 0;
	};

	struct ShaderConstantData
	{
		uint32_t size = 0;
		uint32_t offset = 0;
		ShaderStage	stageFlags = ShaderStage::None;
	};

	struct ShaderResources
	{
		std::map<uint32_t, std::set<uint32_t>> constantBuffers;
		std::map<uint32_t, std::set<uint32_t>> storageBuffers;
		std::map<uint32_t, std::set<uint32_t>> storageImages;
		std::map<uint32_t, std::set<uint32_t>> images;
		std::map<uint32_t, std::set<uint32_t>> samplers;

		ShaderConstantData constants{};
		ShaderDataBuffer constantsBuffer{};
		BufferLayout vertexLayout{};
	
		std::vector<Format> outputFormats;
	};

	class Shader : public RHIInterface
	{
	public:
		virtual const bool Reload(bool forceCompile) = 0;
		virtual std::string_view GetName() const = 0;
		virtual const ShaderResources& GetResources() const = 0;
		virtual const std::vector<std::filesystem::path>& GetSourceFiles() const = 0;
		virtual ShaderDataBuffer GetConstantsBuffer() const = 0;

		static Ref<Shader> Create(std::string_view name, const std::vector<std::filesystem::path>& sourceFiles, bool forceCompile = false);

	protected:
		Shader() = default;
		virtual ~Shader() = default;

	};

	template<typename T>
	T& ShaderDataBuffer::GetMemberData(std::string_view memberName)
	{
		return m_uniforms.at(memberName);
	}
}
