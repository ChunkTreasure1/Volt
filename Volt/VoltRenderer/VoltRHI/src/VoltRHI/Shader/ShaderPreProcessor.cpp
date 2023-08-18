#include <rhipch.h>
#include "ShaderPreProcessor.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#define ARRAYSIZE(array) (sizeof(array) / sizeof(array[0]))

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

		inline const std::vector<std::string> SplitStringsByCharacter(const std::string& src, const char character)
		{
			std::istringstream iss(src);

			std::vector<std::string> result;
			std::string token;

			while (std::getline(iss, token, character))
			{
				if (!token.empty())
				{
					result.emplace_back(token);
				}
			}

			return result;
		}

		inline static void RemoveAllNonLettNumCharacters(std::string& outResult)
		{
			auto newEnd = std::remove_if(outResult.begin(), outResult.end(), [](char c) 
			{ 
				std::string cStr{ c };
				if (cStr.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_") != std::string::npos)
				{
					return true;
				}

				return false;
			});

			outResult.erase(newEnd, outResult.end());
		}

		inline static const bool IsSystemValueSemantic(std::string_view semanticName)
		{
			static constexpr std::string_view svSemantics[] = 
			{
				"sv_clipdistance",
				"sv_culldistance",
				"sv_coverage",
				"sv_depth",
				"sv_depthgreaterequal",
				"sv_depthlessequal",
				"sv_dispatchthreadid",
				"sv_domainlocation",
				"sv_groupindex",
				"sv_groupthreadid",
				"sv_gsInstanceid",
				"sv_innercoverage",
				"sv_insidetessfactor",
				"sv_instanceid",
				"sv_isfrontface",
				"sv_position",
				"sv_primitiveid",
				"sv_rendertargetarrayindex",
				"sv_sampleindex",
				"sv_target",
				"sv_tessfactor",
				"sv_vertexid",
				"sv_viewportarrayindex",
				"sv_shadingrate",
			};

			const std::string strLower = ToLower(std::string(semanticName));

			for (auto svSemantic : svSemantics)
			{
				if (strLower == svSemantic)
				{
					return true;
				}
			}

			return false;
		}

		inline static const bool IsVulkanBuiltIn(std::string_view valueStr)
		{
			return valueStr.find("[[vk::builtin") != std::string_view::npos;
		}
	}

	bool ShaderPreProcessor::PreProcessShaderSource(const PreProcessorData& data, PreProcessorResult& outResult)
	{
		switch (data.shaderStage)
		{
			case ShaderStage::Pixel: return PreProcessPixelSource(data, outResult); break;
			case ShaderStage::Vertex: return PreProcessVertexSource(data, outResult); break;

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

		outResult.preProcessedResult = data.shaderSource;

		return true;
	}

	bool ShaderPreProcessor::PreProcessPixelSource(const PreProcessorData& data, PreProcessorResult& outResult)
	{
		const auto& entryPoint = data.entryPoint;
		outResult.preProcessedResult = data.shaderSource;
		ErasePreProcessData(outResult);

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

 		return true;
	}

	bool ShaderPreProcessor::PreProcessVertexSource(const PreProcessorData& data, PreProcessorResult& outResult)
	{
		const auto& entryPoint = data.entryPoint;
		outResult.preProcessedResult = data.shaderSource;
		ErasePreProcessData(outResult);

		std::string processedSource = data.shaderSource;

		const size_t entryPointLocation = processedSource.find(entryPoint);
		if (entryPointLocation == std::string::npos)
		{
			GraphicsContext::LogTagged(Severity::Error, "[ShaderPreProcessor]", "Unable to find Entry Point {0} in shader!", entryPoint);
			return false;
		}

		// Find entry point parentheses
		const size_t openParenthesesLoc = processedSource.find_first_of('(', entryPointLocation) + 1;
		const size_t closeParenthesesLoc = processedSource.find_first_of(')', entryPointLocation);

		const std::string parenthesesSubStr = processedSource.substr(openParenthesesLoc, closeParenthesesLoc - openParenthesesLoc);

		const auto arguments = Utility::SplitStringsByCharacter(parenthesesSubStr, ' ');
		std::string inputStruct;
		
		// #TODO: Handle non struct inputs
		for (const auto& arg : arguments)
		{
			const size_t argLoc = processedSource.find("struct " + arg);
			if (argLoc != std::string::npos)
			{
				inputStruct = arg;
				break;
			}
		}

		// There are no vertex inputs
		if (inputStruct.empty())
		{
			return true;
		}

		const size_t inputStructLoc = processedSource.find("struct " + inputStruct);
		const size_t openBracketLoc = processedSource.find_first_of('{', inputStructLoc);
		const size_t closeBracketLoc = processedSource.find("};", inputStructLoc);

		std::string structSubStr = processedSource.substr(openBracketLoc, closeBracketLoc - openBracketLoc);

		std::vector<BufferElement> inputElements{};

		size_t currentInputSemiColLoc = structSubStr.find_first_of(';');
		while (currentInputSemiColLoc != std::string::npos)
		{
			std::string currentValueStr = structSubStr.substr(0, currentInputSemiColLoc);

			const size_t divLoc = currentValueStr.find_last_of(':');
			if (divLoc == std::string::npos)
			{
				break;
			}

			std::string nameStr = currentValueStr.substr(divLoc);
			Utility::RemoveAllNonLettNumCharacters(nameStr);

			currentValueStr = currentValueStr.substr(0, divLoc);

			if (!Utility::IsSystemValueSemantic(nameStr) && !Utility::IsVulkanBuiltIn(currentValueStr))
			{
				ElementType elementType = ElementType::Bool;

				const size_t typeTagLoc = currentValueStr.find("[[vt::");
				if (typeTagLoc != std::string::npos)
				{
					const std::string lowerStr = Utility::ToLower(currentValueStr);
					elementType = FindElementTypeFromTag(lowerStr);
				}
				else
				{
					elementType = FindDefaultElementTypeFromString(currentValueStr);
				}

				inputElements.emplace_back(elementType, nameStr);
			}

			structSubStr = structSubStr.substr(currentInputSemiColLoc + 1);
			currentInputSemiColLoc = structSubStr.find_first_of(';');
		}

		outResult.vertexLayout = inputElements;

		return true;
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

	ElementType ShaderPreProcessor::FindDefaultElementTypeFromString(std::string_view str)
	{
		if (str.find(" half ") != std::string_view::npos)
		{
			return ElementType::Half;
		}
		else if (str.find(" half2 ") != std::string_view::npos)
		{
			return ElementType::Half2;
		}
		else if (str.find(" half3 ") != std::string_view::npos)
		{
			return ElementType::Half3;
		}
		else if (str.find(" half4 ") != std::string_view::npos)
		{
			return ElementType::Half4;
		}
		else if (str.find(" float ") != std::string_view::npos)
		{
			return ElementType::Float;
		}
		else if (str.find(" float2 ") != std::string_view::npos)
		{
			return ElementType::Float2;
		}
		else if (str.find(" float3 ") != std::string_view::npos)
		{
			return ElementType::Float3;
		}
		else if (str.find(" float4 ") != std::string_view::npos)
		{
			return ElementType::Float4;
		}
		else if (str.find(" int ") != std::string_view::npos)
		{
			return ElementType::Int;
		}
		else if (str.find(" int2 ") != std::string_view::npos)
		{
			return ElementType::Int2;
		}
		else if (str.find(" int3 ") != std::string_view::npos)
		{
			return ElementType::Int3;
		}
		else if (str.find(" int4 ") != std::string_view::npos)
		{
			return ElementType::Int4;
		}
		else if (str.find(" uint ") != std::string_view::npos)
		{
			return ElementType::UInt;
		}
		else if (str.find(" uint2 ") != std::string_view::npos)
		{
			return ElementType::UInt2;
		}
		else if (str.find(" uint3 ") != std::string_view::npos)
		{
			return ElementType::UInt3;
		}
		else if (str.find(" uint4 ") != std::string_view::npos)
		{
			return ElementType::UInt4;
		}
		else if (str.find(" float3x3 ") != std::string_view::npos)
		{
			return ElementType::Float3x3;
		}
		else if (str.find(" float4x4 ") != std::string_view::npos)
		{
			return ElementType::Float4x4;
		}

		return ElementType::Bool;
	}

	ElementType ShaderPreProcessor::FindElementTypeFromTag(std::string_view str)
	{
		if (str.find("[[vt::half]]") != std::string_view::npos)
		{
			return ElementType::Half;
		}
		else if (str.find("[[vt::half2]]") != std::string_view::npos)
		{
			return ElementType::Half2;
		}
		else if (str.find("[[vt::half3]]") != std::string_view::npos)
		{
			return ElementType::Half3;
		}
		else if (str.find("[[vt::half4]]") != std::string_view::npos)
		{
			return ElementType::Half4;
		}

		else if (str.find("[[vt::byte]]") != std::string_view::npos)
		{
			return ElementType::Byte;
		}
		else if (str.find("[[vt::byte2]]") != std::string_view::npos)
		{
			return ElementType::Byte2;
		}
		else if (str.find("[[vt::byte3]]") != std::string_view::npos)
		{
			return ElementType::Byte3;
		}
		else if (str.find("[[vt::byte4]]") != std::string_view::npos)
		{
			return ElementType::Byte4;
		}

		else if (str.find("[[vt::ushort]]") != std::string_view::npos)
		{
			return ElementType::UShort;
		}
		else if (str.find("[[vt::ushort2]]") != std::string_view::npos)
		{
			return ElementType::UShort2;
		}
		else if (str.find("[[vt::ushort3]]") != std::string_view::npos)
		{
			return ElementType::UShort3;
		}
		else if (str.find("[[vt::ushort4]]") != std::string_view::npos)
		{
			return ElementType::UShort4;
		}

		else if (str.find("[[vt::float]]") != std::string_view::npos)
		{
			return ElementType::Float;
		}
		else if (str.find("[[vt::float2]]") != std::string_view::npos)
		{
			return ElementType::Float2;
		}
		else if (str.find("[[vt::float3]]") != std::string_view::npos)
		{
			return ElementType::Float3;
		}
		else if (str.find("[[vt::float4]]") != std::string_view::npos)
		{
			return ElementType::Float4;
		}

		else if (str.find("[[vt::int]]") != std::string_view::npos)
		{
			return ElementType::Int;
		}
		else if (str.find("[[vt::int2]]") != std::string_view::npos)
		{
			return ElementType::Int2;
		}
		else if (str.find("[[vt::int3]]") != std::string_view::npos)
		{
			return ElementType::Int3;
		}
		else if (str.find("[[vt::int4]]") != std::string_view::npos)
		{
			return ElementType::Int4;
		}

		else if (str.find("[[vt::uint]]") != std::string_view::npos)
		{
			return ElementType::UInt;
		}
		else if (str.find("[[vt::uint2]]") != std::string_view::npos)
		{
			return ElementType::UInt2;
		}
		else if (str.find("[[vt::uint3]]") != std::string_view::npos)
		{
			return ElementType::UInt3;
		}
		else if (str.find("[[vt::uint4]]") != std::string_view::npos)
		{
			return ElementType::UInt4;
		}

		else if (str.find("[[vt::float3x3]]") != std::string_view::npos)
		{
			return ElementType::Float3x3;
		}
		else if (str.find("[[vt::float4x4]]") != std::string_view::npos)
		{
			return ElementType::Float4x4;
		}

		return ElementType::Bool;
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

		// Depth
		if (tempStr == "d32f")
		{
			return Format::D32_SFLOAT;
		}
		else if (tempStr == "d16u")
		{
			return Format::D16_UNORM;
		}
		else if (tempStr == "d16us8")
		{
			return Format::D16_UNORM_S8_UINT;
		}
		else if (tempStr == "d24us8")
		{
			return Format::D24_UNORM_S8_UINT;
		}
		else if (tempStr == "d32us8")
		{
			return Format::D32_SFLOAT_S8_UINT;
		}

		return Format::UNDEFINED;
	}
}
