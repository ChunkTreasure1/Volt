#pragma once

#include "Volt/Rendering/RenderPipeline/RenderPipeline.h"

namespace Volt
{
	class RenderPipelineAsset;
	class Shader;

	class ShaderRegistry
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Register(const std::string& name, Ref<Shader> shader);
		static void Unregister(const std::string& name);

		static bool IsShaderRegistered(const std::string& name);

		static Ref<Shader> GetShader(const std::string& name);

		inline static const std::unordered_map<std::string, Ref<Shader>>& GetShaderRegistry() { return myShaderRegistry; }

	private:
		ShaderRegistry() = delete;

		static void LoadAllShaders();

		inline static std::unordered_map<std::string, Ref<Shader>> myShaderRegistry;
	};
}
