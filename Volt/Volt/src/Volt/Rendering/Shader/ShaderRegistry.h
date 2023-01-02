#pragma once

#include "Volt/Core/Base.h"

#include <map>
#include <string>

namespace Volt
{
	class Shader;
	class ShaderRegistry
	{
	public:
		static void Initialize();
		static void Shutdown();

		static Ref<Shader> Get(const std::string& name);
		static void Register(const std::string& name, Ref<Shader> shader);

		static const std::map<std::string, Ref<Shader>>& GetAllShaders();

	private:
		ShaderRegistry() = delete;

		static void LoadAllShaders();

		inline static std::map<std::string, Ref<Shader>> myRegistry;
	};
}