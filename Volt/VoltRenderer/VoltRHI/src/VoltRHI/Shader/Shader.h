#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

#include "VoltRHI/Shader/BufferLayout.h"
#include "VoltRHI/Shader/ShaderCommon.h"

#include <CoreUtilities/StringHash.h>

#include <vector>
#include <filesystem>
#include <map>
#include <set>

namespace Volt::RHI
{
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

		static RefPtr<Shader> Create(const ShaderSpecification& createInfo);

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
