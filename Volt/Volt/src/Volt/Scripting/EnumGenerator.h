#pragma once

#include <CoreUtilities/Containers/Vector.h>

#include <string>

namespace Volt
{
	class EnumGenerator
	{
	public:
		EnumGenerator(const std::string& enumName, const std::string& nameSpace = "Volt");
		void AddEnumValue(const std::string& name, int32_t value = -1);
		void WriteToFile(const std::filesystem::path& destination);

	private:
		struct EnumValue
		{
			std::string name;
			uint32_t value;
		};
	
		std::string myName;
		std::string myNamespace;

		uint32_t myEnumValueCounter = 0;
		Vector<EnumValue> myEnumValues;
	};
}
