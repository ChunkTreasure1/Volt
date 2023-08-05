#pragma once

#include "VoltRHI/Core/RHICommon.h"

namespace Volt::RHI
{
	struct PreProcessorResult
	{
		std::string preProcessedResult;
		std::vector<Format> outputFormats;
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
		static Format FindDefaultFormatFromString(std::string_view str);
		static Format FindFormatFromLayoutQualifier(const std::string& str);

		static void ErasePreProcessData(PreProcessorResult& outResult);

		ShaderPreProcessor() = delete;
		~ShaderPreProcessor() = delete;
	};
}
