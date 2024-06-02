#include "rhipch.h"
#include "ShaderCache.h"

#include <CoreUtilities/FileIO/BinaryStreamWriter.h>
#include <CoreUtilities/FileIO/BinaryStreamReader.h>

namespace Volt::RHI
{
	struct SerializedShaderData
	{
		ShaderStage stage;
		std::vector<uint32_t> shaderData;

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

	ShaderCompiler::CompilationResultData ShaderCache::TryGetCachedShader(const ShaderCompiler::Specification& shaderSpecification)
	{


		return ShaderCompiler::CompilationResultData();
	}

	void ShaderCache::CacheShader(const ShaderCompiler::CompilationResultData& compilationResult)
	{
		BinaryStreamWriter streamWriter{};

		std::vector<SerializedShaderData> serializedShaderData;

		for (const auto& [stage, shaderData] : compilationResult.shaderData)
		{
			serializedShaderData.emplace_back(stage, shaderData);
		}

		streamWriter.Write(serializedShaderData.size());
		
		for (const auto& shaderData : serializedShaderData)
		{
			streamWriter.Write(shaderData);
		}

		// Pixel shader
		//streamWriter.Write(compilationResult.outputFormats);

		//// Vertex shader
		//streamWriter.Write(compilationResult.vertexLayout);
		//streamWriter.Write(compilationResult.instanceLayout);

		//// Common
		//streamWriter.Write(compilationResult.renderGraphConstants);
		//streamWriter.Write(compilationResult.constantsBuffer);
		//streamWriter.Write(compilationResult.constants);

		//streamWriter.Write(compilationResult.bindings);
		//streamWriter.Write(compilationResult.uniformBuffers);
		//streamWriter.Write(compilationResult.storageBuffers);
		//streamWriter.Write(compilationResult.storageImages);
		//streamWriter.Write(compilationResult.images);
		//streamWriter.Write(compilationResult.samplers);
	}
}
