#include "rhipch.h"
#include "ShaderCache.h"

#include "RHIModule/Graphics/GraphicsContext.h"
#include "RHIModule/Utility/HashUtility.h"

#include <CoreUtilities/FileIO/BinaryStreamWriter.h>
#include <CoreUtilities/FileIO/BinaryStreamReader.h>
#include <CoreUtilities/Time/TimeUtility.h>

namespace Volt::RHI
{
	constexpr uint32_t SHADER_CACHE_VERSION = 1; // Increase this when updating the shader cache format!

	namespace Utility
	{
		inline static std::filesystem::path GetShaderCacheSubDirectory()
		{
			const auto api = GraphicsContext::GetAPI();
			std::filesystem::path subDir;

			switch (api)
			{
				case GraphicsAPI::Vulkan: subDir = "Vulkan"; break;
				case GraphicsAPI::D3D12: subDir = "D3D12"; break;
				case GraphicsAPI::MoltenVk: subDir = "MoltenVK"; break;
			}

			return { subDir };
		}
	}

	struct CachedShaderHeader
	{
		uint64_t timeSinceLastCompile;
	};

	struct SerializedShaderData
	{
		Vector<uint32_t> shaderData;
		ShaderStage stage;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedShaderData& data)
		{
			streamWriter.Write(data.stage);
			streamWriter.Write(data.shaderData);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedShaderData& outData)
		{
			streamReader.Read(outData.stage);
			streamReader.Read(outData.shaderData);
		}
	};

	template<typename T>
	struct SerializedShaderResource
	{
		uint32_t set;
		uint32_t binding;

		T data;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedShaderResource& data)
		{
			streamWriter.Write(data.set);
			streamWriter.Write(data.binding);
			streamWriter.Write(&data.data, sizeof(T));
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedShaderResource& outData)
		{
			streamReader.Read(outData.set);
			streamReader.Read(outData.binding);
			streamReader.Read(&outData.data);
		}
	};

	ShaderCache::ShaderCache(const ShaderCacheCreateInfo& cacheInfo)
		: m_info(cacheInfo)
	{
		VT_ASSERT(s_instance == nullptr);
		s_instance = this;
	}

	ShaderCache::~ShaderCache()
	{
		s_instance = nullptr;
	}

	CachedShaderResult ShaderCache::TryGetCachedShader(const ShaderCompiler::Specification& shaderSpecification)
	{
		VT_ASSERT(s_instance);

		uint64_t lastWriteTime = 0;
		for (const auto& [stage, sourceInfo] : shaderSpecification.shaderSourceInfo)
		{
			lastWriteTime = std::max(lastWriteTime, TimeUtility::GetLastWriteTime(sourceInfo.sourceEntry.filePath));
		}

		BinaryStreamReader streamReader{ s_instance->GetCachedFilePath(shaderSpecification) };
		if (!streamReader.IsStreamValid())
		{
			return {};
		}

		uint32_t shaderCacheVersion = 0;
		streamReader.Read(shaderCacheVersion);

		if (shaderCacheVersion != SHADER_CACHE_VERSION)
		{
			return {};
		}

		CachedShaderHeader cachedHeader{};
		streamReader.Read(cachedHeader);

		if (cachedHeader.timeSinceLastCompile < lastWriteTime)
		{
			return {};
		}

		Vector<SerializedShaderData> serializedShaderData;
		streamReader.Read(serializedShaderData);

		CachedShaderResult result{};
		result.timeSinceLastCompile = cachedHeader.timeSinceLastCompile;
		result.data.result = ShaderCompiler::CompilationResult::Success;

		ShaderCompiler::CompilationResultData& resultData = result.data;

		streamReader.Read(resultData.outputFormats);
		
		streamReader.Read(resultData.vertexLayout);
		streamReader.Read(resultData.instanceLayout);
		
		streamReader.Read(resultData.renderGraphConstants);
		streamReader.Read(resultData.constantsBuffer);
		streamReader.Read(resultData.constants);
		streamReader.Read(resultData.bindings);

		Vector<SerializedShaderResource<ShaderConstantBuffer>> uniformBuffers;
		Vector<SerializedShaderResource<ShaderStorageBuffer>> storageBuffers;
		Vector<SerializedShaderResource<ShaderStorageImage>> storageImages;
		Vector<SerializedShaderResource<ShaderImage>> images;
		Vector<SerializedShaderResource<ShaderSampler>> samplers;

		streamReader.Read(uniformBuffers);
		streamReader.Read(storageBuffers);
		streamReader.Read(storageImages);
		streamReader.Read(images);
		streamReader.Read(samplers);

		for (const auto& data : serializedShaderData)
		{
			resultData.shaderData[data.stage] = data.shaderData;
		}
		
		for (const auto& data : uniformBuffers)
		{
			resultData.uniformBuffers[data.set][data.binding] = data.data;
		}

		for (const auto& data : storageBuffers)
		{
			resultData.storageBuffers[data.set][data.binding] = data.data;
		}

		for (const auto& data : storageImages)
		{
			resultData.storageImages[data.set][data.binding] = data.data;
		}

		for (const auto& data : images)
		{
			resultData.images[data.set][data.binding] = data.data;
		}

		for (const auto& data : samplers)
		{
			resultData.samplers[data.set][data.binding] = data.data;
		}

		return result;
	}

	void ShaderCache::CacheShader(const ShaderCompiler::Specification& shaderSpec, const ShaderCompiler::CompilationResultData& compilationResult)
	{
		VT_ASSERT(s_instance);

		BinaryStreamWriter streamWriter{};

		CachedShaderHeader cachedShaderHeader{};
		cachedShaderHeader.timeSinceLastCompile = TimeUtility::GetTimeSinceEpoch();

		streamWriter.Write(SHADER_CACHE_VERSION);
		streamWriter.Write(cachedShaderHeader);

		Vector<SerializedShaderData> serializedShaderData;

		for (const auto& [stage, shaderData] : compilationResult.shaderData)
		{
			serializedShaderData.emplace_back(shaderData, stage);
		}

		streamWriter.Write(serializedShaderData);

		// Pixel shader
		streamWriter.Write(compilationResult.outputFormats);

		// Vertex shader
		streamWriter.Write(compilationResult.vertexLayout);
		streamWriter.Write(compilationResult.instanceLayout);

		// Common
		streamWriter.Write(compilationResult.renderGraphConstants);
		streamWriter.Write(compilationResult.constantsBuffer);
		streamWriter.Write(compilationResult.constants);

		streamWriter.Write(compilationResult.bindings);

		Vector<SerializedShaderResource<ShaderConstantBuffer>> uniformBuffers;
		Vector<SerializedShaderResource<ShaderStorageBuffer>> storageBuffers;
		Vector<SerializedShaderResource<ShaderStorageImage>> storageImages;
		Vector<SerializedShaderResource<ShaderImage>> images;
		Vector<SerializedShaderResource<ShaderSampler>> samplers;

		for (const auto& [set, bindings] : compilationResult.uniformBuffers)
		{
			for (const auto& [binding, data] : bindings)
			{
				uniformBuffers.emplace_back(set, binding, data);
			}
		}

		for (const auto& [set, bindings] : compilationResult.storageBuffers)
		{
			for (const auto& [binding, data] : bindings)
			{
				storageBuffers.emplace_back(set, binding, data);
			}
		}

		for (const auto& [set, bindings] : compilationResult.storageImages)
		{
			for (const auto& [binding, data] : bindings)
			{
				storageImages.emplace_back(set, binding, data);
			}
		}

		for (const auto& [set, bindings] : compilationResult.images)
		{
			for (const auto& [binding, data] : bindings)
			{
				images.emplace_back(set, binding, data);
			}
		}

		for (const auto& [set, bindings] : compilationResult.samplers)
		{
			for (const auto& [binding, data] : bindings)
			{
				samplers.emplace_back(set, binding, data);
			}
		}

		streamWriter.Write(uniformBuffers);
		streamWriter.Write(storageBuffers);
		streamWriter.Write(storageImages);
		streamWriter.Write(images);
		streamWriter.Write(samplers);

		streamWriter.WriteToDisk(s_instance->GetCachedFilePath(shaderSpec), false, 0);
	}

	std::filesystem::path ShaderCache::GetCachedFilePath(const ShaderCompiler::Specification& shaderSpec) const
	{
		size_t hash = 0;
		for (const auto& [stage, sourceInfo] : shaderSpec.shaderSourceInfo)
		{
			const size_t stageHash = Utility::HashCombine(std::hash<std::filesystem::path>()(sourceInfo.sourceEntry.filePath), std::hash<std::string>()(sourceInfo.sourceEntry.entryPoint));

			if (hash == 0)
			{
				hash = stageHash;
			}
			else
			{
				hash = Utility::HashCombine(hash, stageHash);
			}
		}

		const auto cacheDir = s_instance->m_info.cacheDirectory / Utility::GetShaderCacheSubDirectory();
		const auto cachePath = cacheDir / (std::to_string(hash) + ".vtshcache");

		if (!std::filesystem::exists(cacheDir))
		{
			std::filesystem::create_directories(cacheDir);
		}

		return cachePath;
	}
}
