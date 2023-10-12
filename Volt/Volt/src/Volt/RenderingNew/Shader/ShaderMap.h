#pragma once

#include "Volt/Core/Base.h"

#include <unordered_map>
#include <string>

namespace Volt
{
	namespace RHI
	{
		class Shader;
	}

	class ShaderMap
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void ReloadAll();

		static Ref<RHI::Shader> Get(const std::string& name);

	private:
		static void LoadShaders();
		static void RegisterShader(const std::string& name, Ref<RHI::Shader> shader);

		inline static std::unordered_map<std::string, Ref<RHI::Shader>> s_shaderMap;
		inline static std::mutex s_registerMutex;
	};
}
