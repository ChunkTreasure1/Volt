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

		static Ref<RHI::Shader> Get(const std::string& name);

	private:
		static void LoadShaders();
		static void RegisterShader(const std::string& name, Ref<RHI::Shader> shader);

		inline static std::unordered_map<std::string, Ref<RHI::Shader>> m_shaderMap;
		inline static std::mutex m_registerMutex;
	};
}
