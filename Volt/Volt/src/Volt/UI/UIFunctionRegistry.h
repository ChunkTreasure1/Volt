#pragma once
#include "Volt/Core/Base.h"
#include <unordered_map>
#include <functional>

namespace Volt
{
	class UIFunctionRegistry
	{
	public:
		UIFunctionRegistry() = default;

		static bool AddFunc(const std::string& funcName, std::function<void()> aFunction)
		{
			auto map = func_registry;

			if (map.find(funcName) != map.end())
			{
				return false;
			}

			func_registry[funcName] = aFunction;
			return true;
		}

		static bool Execute(const std::string& funcName)
		{
			auto map = func_registry;
			if (map.find(funcName) == map.end())
			{
				return false;
			}

			func_registry[funcName]();
			return true;
		}

		inline static const std::unordered_map<std::string, std::function<void()>>& GetRegistry() { return func_registry; }

	private:
		inline static std::unordered_map<std::string, std::function<void()>> func_registry;
	};
};
