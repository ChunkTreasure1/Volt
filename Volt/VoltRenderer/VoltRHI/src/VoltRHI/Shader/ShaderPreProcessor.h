#pragma once

#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Shader/BufferLayout.h"
#include "VoltRHI/Shader/Shader.h"

namespace Volt::RHI
{
	struct PreProcessorResult
	{
		std::string preProcessedResult;
	
		std::vector<PixelFormat> outputFormats;
		BufferLayout vertexLayout;
		BufferLayout instanceLayout;
		ShaderRenderGraphConstantsData renderGraphConstants;
	};

	struct PreProcessorData
	{
		std::string shaderSource;
		std::string entryPoint = "main";

		ShaderStage shaderStage;
	};

	class ShaderPreProcessor
	{
	public:
		static bool PreProcessShaderSource(const PreProcessorData& data, PreProcessorResult& outResult);

	private:
		static bool PreProcessPixelSource(const PreProcessorData& data, PreProcessorResult& outResult);
		static bool PreProcessVertexSource(const PreProcessorData& data, PreProcessorResult& outResult);

		static bool GenerateConstantsInformation(const PreProcessorData& data, PreProcessorResult& outResult);
		static void GetConstantsInformationFromMemberStructRecursive(const std::string& memberType, const std::string& memberName, const PreProcessorData& data, PreProcessorResult& outResult);
		
		static PixelFormat FindDefaultFormatFromString(std::string_view str);
		static PixelFormat FindFormatFromLayoutQualifier(const std::string& str);

		static ElementType FindDefaultElementTypeFromString(std::string_view str);
		static ElementType FindElementTypeFromTag(std::string_view tagStr);

		static ShaderUniformType FindUniformTypeFromString(std::string_view str);

		static void ErasePreProcessData(PreProcessorResult& outResult);

		ShaderPreProcessor() = delete;
		~ShaderPreProcessor() = delete;
	};
}
