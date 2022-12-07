#pragma once

#include "Volt/Core/Base.h"

#include <unordered_map>

namespace Volt
{
	class MultiMaterial;
	class MaterialRegistry
	{
	public:
		static void Initialize();
		static void Shutdown();

		static const std::filesystem::path Get(const std::string& name);
		static void Register(const std::string& name, const std::filesystem::path& path);

		inline static const std::unordered_map<std::string, std::filesystem::path>& GetMaterials() { return s_registry; }

	private:
		static void FindAllMaterials();

		MaterialRegistry() = delete;

		inline static std::unordered_map<std::string, std::filesystem::path> s_registry;
	};
}