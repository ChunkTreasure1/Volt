#include <rhipch.h>
#include "ShaderPreProcessor.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

namespace Volt::RHI
{
	namespace Utility
	{
		inline static bool IsDefaultType(std::string_view str)
		{
			if (str.find("float") != std::string::npos || str.find("int") != std::string::npos ||
				str.find("uint") != std::string::npos || str.find("half") != std::string::npos)
			{
				return true;
			}

			return false;
		}

		inline static std::string ToLower(const std::string& str)
		{
			std::string newStr(str);
			std::transform(str.begin(), str.end(), newStr.begin(), [](unsigned char c) { return (uint8_t)std::tolower((int32_t)c); });

			return newStr;
		}
	}

	bool ShaderPreProcessor::PreProcessShaderSource(const PreProcessorData& data, PreProcessorResult& outResult)
	{
		switch (data.shaderStage)
		{
			case ShaderStage::Pixel: return PreProcessPixelSource(data, outResult); break;

			case ShaderStage::Vertex:
			case ShaderStage::Hull:
			case ShaderStage::Domain:
			case ShaderStage::Geometry:
			case ShaderStage::Compute:
			case ShaderStage::RayGen:
			case ShaderStage::AnyHit:
			case ShaderStage::ClosestHit:
			case ShaderStage::Miss:
			case ShaderStage::Intersection:
			case ShaderStage::Task:
			case ShaderStage::Mesh:
			case ShaderStage::All:
			case ShaderStage::Common:
				break;
			default:
				break;
		}

		return false;
	}

	bool ShaderPreProcessor::PreProcessPixelSource(const PreProcessorData& data, PreProcessorResult& outResult)
	{
		const auto& entryPoint = data.entryPoint;
		std::string processedSource = data.shaderSource;

		const size_t entryPointLocation = processedSource.find(entryPoint);
		if (entryPointLocation == std::string::npos)
		{
			GraphicsContext::LogTagged(Severity::Error, "[ShaderPreProcessor]", "Unable to find Entry Point {0} in shader!", entryPoint);
			return false;
		}

		// Find return value of "main" function
		std::string entryPointSubStr = processedSource.substr(0, entryPointLocation);

		const size_t lastSpace = entryPointSubStr.find_last_of(' ');
		std::string outputSubStr = entryPointSubStr.substr(0, lastSpace);

		const size_t preReturnValueChar = outputSubStr.find_last_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
		std::string returnValueStr = outputSubStr.substr(preReturnValueChar + 1);

		// Check return value type and find it if necessary
		const bool isDefaultType = Utility::IsDefaultType(returnValueStr);

		if (isDefaultType)
		{
			std::string fullTypeSubStr = outputSubStr.substr(0, outputSubStr.size() - returnValueStr.size());

			// Remove all prefix characters
			while (fullTypeSubStr[fullTypeSubStr.size() - 1] != ']' && !fullTypeSubStr.empty())
			{
				if (fullTypeSubStr[fullTypeSubStr.size() - 1] != '\n' && fullTypeSubStr[fullTypeSubStr.size() - 1] != ' ')
				{
					break;
				}

				fullTypeSubStr.pop_back();
			}

			if (fullTypeSubStr.empty())
			{
				return false; // #TODO_Ivar: Handle
			}

			const size_t lastChar = fullTypeSubStr.find_last_not_of("[[]]abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890:");
			const std::string lastSubStr = fullTypeSubStr.substr(lastChar + 1);

			if (lastSubStr.empty())
			{
				outResult.outputFormats.emplace_back(FindDefaultFormatFromString(returnValueStr));
				return true;
			}
			else
			{
				outResult.outputFormats.emplace_back(FindFormatFromLayoutQualifier(lastSubStr));
			}

			return true;
		}

		const size_t structPos = outputSubStr.find("struct " + returnValueStr);
		if (structPos == std::string::npos)
		{
			return false;
		}

		std::string structSubStr = outputSubStr.substr(structPos);
		size_t openBracketPos = structSubStr.find_first_of('{') + 1;
		const size_t closingBracketPos = structSubStr.find_first_of('}');
		
		openBracketPos = structSubStr.find_first_not_of("\n ", openBracketPos);

		std::string structBracketSubStr = structSubStr.substr(openBracketPos, closingBracketPos - openBracketPos);

		size_t currentSemicolon = structBracketSubStr.find_first_of(';');

		while (currentSemicolon != std::string::npos)
		{
			std::string outputTypeStr = structBracketSubStr.substr(0, currentSemicolon);
			while (outputTypeStr[0] == ' ' || outputTypeStr[0] == '\n')
			{
				outputTypeStr.erase(outputTypeStr.begin());
			}

			const size_t firstSpace = outputTypeStr.find_first_of(' ');
			std::string typeStr = outputTypeStr.substr(0, firstSpace);

			const size_t qualifierPos = typeStr.find_first_of("[");

			if (qualifierPos == std::string::npos)
			{
				outResult.outputFormats.emplace_back(FindDefaultFormatFromString(typeStr));
			}
			else
			{
				std::string qualifierStr = typeStr.substr(qualifierPos, qualifierPos + typeStr.find_last_of("]") + 1);
				outResult.outputFormats.emplace_back(FindFormatFromLayoutQualifier(qualifierStr));
			}

			structBracketSubStr = structBracketSubStr.substr(currentSemicolon + 1);
			currentSemicolon = structBracketSubStr.find_first_of(';');
		}

		outResult.preProcessedResult = data.shaderSource;
		ErasePreProcessData(outResult);

 		return true;
	}

	Format ShaderPreProcessor::FindDefaultFormatFromString(std::string_view str)
	{
		if (str == "float")
		{
			return Format::R32_SFLOAT;
		}
		else if (str == "float2")
		{
			return Format::R32G32_SFLOAT;
		}
		else if (str == "float3")
		{
			return Format::R32G32B32_SFLOAT;
		}
		else if (str == "float4")
		{
			return Format::R32G32B32A32_SFLOAT;
		}
		else if (str == "int")
		{
			return Format::R32_SINT;
		}
		else if (str == "int2")
		{
			return Format::R32G32_SINT;
		}
		else if (str == "int3")
		{
			return Format::R32G32B32_SINT;
		}
		else if (str == "int4")
		{
			return Format::R32G32B32A32_SINT;
		}
		else if (str == "uint")
		{
			return Format::R32_UINT;
		}
		else if (str == "uint2")
		{
			return Format::R32G32_UINT;
		}
		else if (str == "uint3")
		{
			return Format::R32G32B32_UINT;
		}
		else if (str == "uint4")
		{
			return Format::R32G32B32A32_UINT;
		}
		else if (str == "half")
		{
			return Format::R16_SFLOAT;
		}
		else if (str == "half2")
		{
			return Format::R16G16_SFLOAT;
		}
		else if (str == "half3")
		{
			return Format::R16G16B16_SFLOAT;
		}
		else if (str == "half4")
		{
			return Format::R16G16B16A16_SFLOAT;
		}

		return Format::UNDEFINED;
	}

	Format ShaderPreProcessor::FindFormatFromLayoutQualifier(const std::string& layoutStr)
	{
		std::string tempStr = layoutStr;
		tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '['), tempStr.end());
		tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), ']'), tempStr.end());

		tempStr = tempStr.substr(4); // Remove "vt::"
		tempStr = Utility::ToLower(tempStr);

		// Floating point
		if (tempStr == "rgba32f")
		{
			return Format::R32G32B32A32_SFLOAT;
		}
		else if (tempStr == "rgb32f")
		{
			return Format::R32G32B32_SFLOAT;
		}
		else if (tempStr == "rg32f")
		{
			return Format::R32G32_SFLOAT;
		}
		else if (tempStr == "r32f")
		{
			return Format::R32_SFLOAT;
		}
		else if (tempStr == "rgba16f")
		{
			return Format::R16G16B16A16_SFLOAT;
		}
		else if (tempStr == "rgb16f")
		{
			return Format::R16G16B16_SFLOAT;
		}
		else if (tempStr == "rg16f")
		{
			return Format::R16G16_SFLOAT;
		}
		else if (tempStr == "r16f")
		{
			return Format::R16_SFLOAT;
		}
		else if (tempStr == "rgba16")
		{
			return Format::R16G16B16A16_UNORM;
		}
		else if (tempStr == "rgb16")
		{
			return Format::R16G16B16_UNORM;
		}
		else if (tempStr == "rg16")
		{
			return Format::R16G16_UNORM;
		}
		else if (tempStr == "r16")
		{
			return Format::R16_UNORM;
		}
		else if (tempStr == "rgba8")
		{
			return Format::R8G8B8A8_UNORM;
		}
		else if (tempStr == "rgb8")
		{
			return Format::R8G8B8_UNORM;
		}
		else if (tempStr == "rg8")
		{
			return Format::R8G8_UNORM;
		}
		else if (tempStr == "r8")
		{
			return Format::R8_UNORM;
		}
		else if (tempStr == "rgba16_snorm")
		{
			return Format::R16G16B16A16_SNORM;
		}
		else if (tempStr == "rgb16_snorm")
		{
			return Format::R16G16B16_SNORM;
		}
		else if (tempStr == "rg16_snorm")
		{
			return Format::R16G16_SNORM;
		}
		else if (tempStr == "r16_snorm")
		{
			return Format::R16_SNORM;
		}
		else if (tempStr == "rgba8_snorm")
		{
			return Format::R8G8B8A8_SNORM;
		}
		else if (tempStr == "rgb8_snorm")
		{
			return Format::R8G8B8_SNORM;
		}
		else if (tempStr == "rg8_snorm")
		{
			return Format::R8G8_SNORM;
		}
		else if (tempStr == "r8_snorm")
		{
			return Format::R8_SNORM;
		}
		else if (tempStr == "r11f_g11f_b10f")
		{
			return Format::B10G11R11_UFLOAT_PACK32;
		}
		else if (tempStr == "rgb10_a2")
		{
			Format::A2R10G10B10_UNORM_PACK32;
		}

		// Signed int
		if (tempStr == "rgba32i")
		{
			return Format::R32G32B32A32_SINT;
		}
		else if (tempStr == "rgb32i")
		{
			return Format::R32G32B32_SINT;
		}
		else if (tempStr == "rg32i")
		{
			return Format::R32G32_SINT;
		}
		else if (tempStr == "r32i")
		{
			return Format::R32_SINT;
		}
		else if (tempStr == "rgba16i")
		{
			return Format::R16G16B16A16_SINT;
		}
		else if (tempStr == "rgb16i")
		{
			return Format::R16G16B16_SINT;
		}
		else if (tempStr == "rg16i")
		{
			return Format::R16G16_SINT;
		}
		else if (tempStr == "r16i")
		{
			return Format::R16_SINT;
		}
		else if (tempStr == "rgba8i")
		{
			return Format::R8G8B8A8_SINT;
		}
		else if (tempStr == "rgb8i")
		{
			return Format::R8G8B8_SINT;
		}
		else if (tempStr == "rg8i")
		{
			return Format::R8G8_SINT;
		}
		else if (tempStr == "r8i")
		{
			return Format::R8_SINT;
		}

		// Unsigned int
		if (tempStr == "rgba32ui")
		{
			return Format::R32G32B32A32_UINT;
		}
		else if (tempStr == "rgb32ui")
		{
			return Format::R32G32B32_UINT;
		}
		else if (tempStr == "rg32ui")
		{
			return Format::R32G32_UINT;
		}
		else if (tempStr == "r32ui")
		{
			return Format::R32_UINT;
		}
		else if (tempStr == "rgba16ui")
		{
			return Format::R16G16B16A16_UINT;
		}
		else if (tempStr == "rgb16ui")
		{
			return Format::R16G16B16_UINT;
		}
		else if (tempStr == "rg16ui")
		{
			return Format::R16G16_UINT;
		}
		else if (tempStr == "r16ui")
		{
			return Format::R16_UINT;
		}
		else if (tempStr == "rgba8ui")
		{
			return Format::R8G8B8A8_UINT;
		}
		else if (tempStr == "rgb8ui")
		{
			return Format::R8G8B8_UINT;
		}
		else if (tempStr == "rg8ui")
		{
			return Format::R8G8_UINT;
		}
		else if (tempStr == "r8ui")
		{
			return Format::R8_UINT;
		}
		else if (tempStr == "rgb10_a2ui")
		{
			return Format::A2B10G10R10_UINT_PACK32;
		}

		return Format::UNDEFINED;
	}

	void ShaderPreProcessor::ErasePreProcessData(PreProcessorResult& outResult)
	{
		std::string& result = outResult.preProcessedResult;

		size_t currentTagPos = result.find("[[vt::");

		while (currentTagPos != std::string::npos)
		{
			size_t endBracketPos = result.find("]]", currentTagPos);
			
			result.erase(currentTagPos, endBracketPos - currentTagPos + 2);

			currentTagPos = result.find("[[vt::");
		}
	}
}
