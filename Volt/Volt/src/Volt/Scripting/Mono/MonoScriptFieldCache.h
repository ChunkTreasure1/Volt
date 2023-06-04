#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/UUID.h"

#include <unordered_map>

namespace Volt
{
	struct MonoScriptFieldInstance;

	using MonoScriptFieldMap = std::unordered_map<std::string, Ref<MonoScriptFieldInstance>>;

	class MonoScriptFieldCache
	{
	public:
		MonoScriptFieldCache() = default;
		~MonoScriptFieldCache() = default;

		inline std::unordered_map<UUID, MonoScriptFieldMap>& GetCache() { return myScriptFields; }
		inline const std::unordered_map<UUID, MonoScriptFieldMap>& GetCache() const { return myScriptFields; }

	private:
		std::unordered_map<UUID, MonoScriptFieldMap> myScriptFields;
	};
}
