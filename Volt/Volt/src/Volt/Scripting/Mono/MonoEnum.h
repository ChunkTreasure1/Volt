#pragma once

#include <string>
#include <vector>

extern "C"
{
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClass MonoClass;
}

namespace Volt
{
	class MonoEnum
	{
	public:
		MonoEnum() = default;
		MonoEnum(MonoImage* assemblyImage, const std::string& classNamespace, const std::string& enumName);

		inline const Vector<std::pair<std::string, uint32_t>>& GetValues() const { return m_enumValues; }

	private:
		void LoadEnumValues();

		Vector<std::pair<std::string, uint32_t>> m_enumValues;

		std::string m_namespace;
		std::string m_enumName;

		MonoClass* m_monoClass = nullptr;
	};
}
