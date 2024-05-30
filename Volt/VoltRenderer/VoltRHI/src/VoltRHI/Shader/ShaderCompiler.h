#pragma once

#include "VoltRHI/Core/RHIInterface.h"

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
		std::vector<std::filesystem::path> includeDirectories;
		std::vector<std::string> initialMacros;
	
		ShaderCompilerFlags flags = ShaderCompilerFlags::None;
		std::filesystem::path cacheDirectory = "Engine/Shaders/Cache/";
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

		struct Specification
		{
			OptimizationLevel optimizationLevel = OptimizationLevel::Disable;
		
			std::string entryPoint = "main";
			bool forceCompile = false;
		};

		virtual ~ShaderCompiler();;

		[[nodiscard]] static CompilationResult TryCompile(const Specification& specification, Shader& shader);
		static void AddMacro(const std::string& macroName);
		static void RemoveMacro(std::string_view macroName);
		
		static RefPtr<ShaderCompiler> Create(const ShaderCompilerCreateInfo& createInfo);

	protected:
		ShaderCompiler();

		// Should compile shader using shader source files, result is stored in shaders internal storage
		virtual CompilationResult TryCompileImpl(const Specification& specification, Shader& shader) = 0;
		virtual void AddMacroImpl(const std::string& macroName) = 0;
		virtual void RemoveMacroImpl(std::string_view macroName) = 0;

	private:
		inline static ShaderCompiler* s_instance = nullptr;
	};
}
