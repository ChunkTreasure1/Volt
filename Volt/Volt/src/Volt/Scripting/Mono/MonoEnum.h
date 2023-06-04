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

		inline const std::vector<std::pair<std::string, uint32_t>>& GetValues() const { return myEnumValues; }

	private:
		void LoadEnumValues();

		std::vector<std::pair<std::string, uint32_t>> myEnumValues;

		std::string myNamespace;
		std::string myEnumName;

		MonoClass* myMonoClass = nullptr;
	};
}
