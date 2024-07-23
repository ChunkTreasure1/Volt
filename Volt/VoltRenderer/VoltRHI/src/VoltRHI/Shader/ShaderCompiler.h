#pragma once

#include "VoltRHI/Core/RHIInterface.h"

#include "VoltRHI/Shader/BufferLayout.h"
#include "VoltRHI/Shader/ShaderCommon.h"
#include "VoltRHI/Core/RHICommon.h"

#include <span>
#include <filesystem>

namespace Volt::RHI
{
	class Shader;

	enum class ShaderCompilerFlags : uint32_t
	{
		None = BIT(0),
		WarningsAsErrors = BIT(1),
		EnableShaderValidator = BIT(2)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(ShaderCompilerFlags);

	struct ShaderCompilerCreateInfo
	{
		Vector<std::filesystem::path> includeDirectories;
		Vector<std::string> initialMacros;
	
		ShaderCompilerFlags flags = ShaderCompilerFlags::None;
	};

	class VTRHI_API ShaderCompiler : public RHIInterface
	{
	public:
		enum class CompilationResult : uint32_t
		{
			Success = 0,
			PreprocessFailed,
			Failure
		};

		enum class OptimizationLevel : uint32_t
		{
			Disable,
			Release,
			Dist,
		};

		struct CompilationResultData
		{
			CompilationResult result = CompilationResult::Failure;
			std::unordered_map<ShaderStage, Vector<uint32_t>> shaderData;

			// Pixel Shader
			Vector<RHI::PixelFormat> outputFormats;
			
			// Vertex Shader
			RHI::BufferLayout vertexLayout;
			RHI::BufferLayout instanceLayout;

			// Common
			ShaderRenderGraphConstantsData renderGraphConstants{};
			ShaderDataBuffer constantsBuffer{};
			ShaderConstantData constants{};

			std::unordered_map<std::string, ShaderResourceBinding> bindings;

			std::map<uint32_t, std::map<uint32_t, ShaderConstantBuffer>> uniformBuffers;
			std::map<uint32_t, std::map<uint32_t, ShaderStorageBuffer>> storageBuffers;
			std::map<uint32_t, std::map<uint32_t, ShaderStorageImage>> storageImages;
			std::map<uint32_t, std::map<uint32_t, ShaderImage>> images;
			std::map<uint32_t, std::map<uint32_t, ShaderSampler>> samplers;

			VT_NODISCARD VT_INLINE bool IsValid() const { return !shaderData.empty(); }
		};

		struct Specification
		{
			OptimizationLevel optimizationLevel = OptimizationLevel::Disable;
			std::unordered_map<ShaderStage, ShaderSourceInfo> shaderSourceInfo;
			bool forceCompile = false;
		};

		virtual ~ShaderCompiler();

		[[nodiscard]] static CompilationResultData TryCompile(const Specification& specification);
		static void AddMacro(const std::string& macroName);
		static void RemoveMacro(std::string_view macroName);
		
		static RefPtr<ShaderCompiler> Create(const ShaderCompilerCreateInfo& createInfo);

	protected:
		ShaderCompiler();

		// Should compile shader using shader source files, result is stored in shaders internal storage
		virtual CompilationResultData TryCompileImpl(const Specification& specification) = 0;
		virtual void AddMacroImpl(const std::string& macroName) = 0;
		virtual void RemoveMacroImpl(std::string_view macroName) = 0;

	private:
		inline static ShaderCompiler* s_instance = nullptr;
	};
}
