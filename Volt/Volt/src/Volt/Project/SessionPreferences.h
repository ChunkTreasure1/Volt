#pragma once

#include <unordered_map>
#include <string>
#include <any>

namespace Volt
{
	class SessionPreferences
	{
	public:
		static void Initialize();

		static void DeleteAll();
		static void DeleteKey(const std::string& key);

		static float GetFloat(const std::string& key);
		static int32_t GetInt(const std::string& key);
		static std::string GetString(const std::string& key);

		static void SetFloat(const std::string& key, float value);
		static void SetInt(const std::string& key, int32_t value);
		static void SetString(const std::string& key, const std::string& value);

		static bool HasKey(const std::string& key);
		static void Save();

	private:
		static const std::string GetRegistryPath();

		inline static std::unordered_map<std::string, std::any> myRegistry;
	};
}
