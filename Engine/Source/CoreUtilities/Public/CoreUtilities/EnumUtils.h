#pragma once

#include "Containers/Vector.h"
#include <CoreUtilities/Concepts.h>


#include <unordered_map>
#include <map>
#include <string>
#include <cassert>

namespace Utils
{
	/// <summary>
	/// Class for automatically generating enum to string and string to enum functions. DO NOT CALL THESE FUNCTIONS DIRECTLY.
	/// </summary>
	class EnumUtil
	{
	public:
		template<Enum EnumType>
		static std::string ToString(uint64_t aEnumValue)
		{
			assert(myRegistry.contains(typeid(EnumType).name()) && "Tried to Convert enum to string with an enum that is not registered!");

			return myRegistry[typeid(EnumType).name()][aEnumValue];
		}

		//to enum
		template<Enum EnumType>
		static EnumType ToEnum(std::string aEnumValue)
		{
			assert(myRegistry.contains(typeid(EnumType).name()) && "Tried to convert string to enum that has not been registered!");
			const auto& enumMap = myRegistry[typeid(EnumType).name()];
			for (const auto& pair : enumMap)
			{
				if (pair.second == aEnumValue)
				{
					return static_cast<EnumType>(pair.first);
				}
			}
			//VT_LOG(LogSeverity::Error, "Tried to convert string to enum that does not exist! EnumName: {0}, EnumValue: {1}", typeid(EnumType).name(), aEnumValue);
			return static_cast<EnumType>(0);
		}

		static bool RegisterEnum(const std::string& name, const std::string& definitionData)
		{
			const std::string allLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

			size_t stringOffset = definitionData.find_first_of(allLetters);
			Vector<std::string> enumNames;
			Vector<uint64_t> enumValues;
			while (stringOffset != std::string::npos)
			{
				size_t memberNameEndOffset = definitionData.find_first_not_of(allLetters, stringOffset);
				if (memberNameEndOffset == std::string::npos)
				{
					memberNameEndOffset = definitionData.size();
				}

				std::string memberName = definitionData.substr(stringOffset, memberNameEndOffset - stringOffset);
				uint64_t memberValue = 0;

				assert(!memberName.empty() && "When Registering an enum, you must provide a name for each member");

				//find the comma
				size_t endOfMemberOffset = definitionData.find_first_of(',', memberNameEndOffset);
				if (endOfMemberOffset == std::string::npos)
				{
					//this is the last member so instead take the end of the string
					endOfMemberOffset = definitionData.size();
				}

				std::string memberValueString = definitionData.substr(memberNameEndOffset, endOfMemberOffset - memberNameEndOffset);

				//if the value string contains a = then we need to parse it else we just increment the highest value so far
				if (memberValueString.find('=') != std::string::npos)
				{
					//remove whitespace
					memberValueString.erase(std::remove_if(memberValueString.begin(), memberValueString.end(), isspace), memberValueString.end());

					//remove the =
					memberValueString.erase(std::remove(memberValueString.begin(), memberValueString.end(), '='), memberValueString.end());

					//if the value is hex
					if (memberValueString.find('x') != std::string::npos)
					{
						memberValue = std::stoull(memberValueString, nullptr, 16);
					}
					//if the value is binary
					else if (memberValueString.find('b') != std::string::npos)
					{
						memberValue = std::stoull(memberValueString, nullptr, 2);
					}
					//if the value is bitshifted to the left
					else if (memberValueString.find('<') != std::string::npos)
					{
						//split string along "<<"
						const size_t bitshiftOffset = memberValueString.find_first_of('<');
						std::string base = memberValueString.substr(0, bitshiftOffset);
						std::string shift = memberValueString.substr(bitshiftOffset + 2, memberValueString.size() - bitshiftOffset - 2);

						memberValue = std::stoull(base, nullptr, 10) << std::stoull(shift, nullptr, 10);
					}
					//if the value is bitshifted to the right
					else if (memberValueString.find('>') != std::string::npos)
					{
						//split string along ">>"
						const size_t bitshiftOffset = memberValueString.find_first_of('>');
						std::string base = memberValueString.substr(0, bitshiftOffset);
						std::string shift = memberValueString.substr(bitshiftOffset + 2, memberValueString.size() - bitshiftOffset - 2);

						memberValue = std::stoull(base, nullptr, 10) >> std::stoull(shift, nullptr, 10);
					}
					//if the value is a number
					else
					{
						memberValue = std::stoull(memberValueString);
					}
				}
				else
				{
					memberValue = std::numeric_limits<uint64_t>::max();
				}

				stringOffset = definitionData.find_first_of(allLetters, endOfMemberOffset);
				enumNames.push_back(memberName);
				enumValues.push_back(memberValue);
			}
			for (size_t i = 0; i < enumValues.size(); i++)
			{
				if (enumValues[i] == std::numeric_limits<uint64_t>::max())
				{
					if (i == 0)
					{
						enumValues[i] = 0;
					}
					else
					{
						enumValues[i] = enumValues[i - 1] + 1;
					}
				}
			}
			auto& enumMap = myRegistry[name];
			for (size_t i = 0; i < enumNames.size(); i++)
			{
				enumMap.insert({ enumValues[i], enumNames[i] });
			}

			return true;
		}



	private:
		//<EnumName, <EnumValue, EnumString>>
		inline static std::unordered_map<std::string, std::map<uint64_t, std::string>> myRegistry;
	};
}

template<Enum T> inline static T ToEnum(const std::string& aEnumString)
{
	return Utils::EnumUtil::ToEnum<T>(aEnumString);
};

#define CREATE_ENUM_TYPED(enumName, type, ...)\
enum class enumName : type \
{ \
	__VA_ARGS__ \
}; \
inline static bool enumName##_enum_reg = Utils::EnumUtil::RegisterEnum(typeid(enumName).name(), #__VA_ARGS__); \
inline static std::string ToString(enumName aEnumValue) \
{ \
	return Utils::EnumUtil::ToString<enumName>(static_cast<uint64_t>(aEnumValue)); \
}

#define CREATE_ENUM(enumName, ...) CREATE_ENUM_TYPED(enumName, uint32_t, __VA_ARGS__)



enum class yoo
{
	apa = 2,
	banan
};

//CREATE_ENUM(Test, uint64_t,
//	One = 10, Two, Three = 0x1000000000010011, Four);

//enum class Test : uint64_t
//{
//	One = 10, Two, Three = 0x1000000000010011, Four
//}; inline static bool Test_reg = Utils::EnumUtil::RegisterEnum("Test", "One = 10, Two, Three = 0x1000000000010011, Four"); inline static std::string ToString(Test aEnumValue)
//{
//	return Utils::EnumUtil::ToString("Test", static_cast<uint64_t>(aEnumValue));
//}
//template<> inline static Test ToEnum(const std::string& aEnumString)
//{
//	return Utils::EnumUtil::ToEnum<Test>("Test", aEnumString);
//};



/*
enum class Test : uint32_t {
	One, Two, Three
};inline static bool Test_reg = EnumUtil::RegisterEnum("Test", "One, Two, Three"); inline static std::string ToString(Test aEnumValue) {
	return EnumUtil::ToString("Test", static_cast<uint64_t>(aEnumValue));
}
*/

template<Enum T>
inline static constexpr bool EnumValueContainsFlag(const T& value, const T& flag)
{
	return (value & flag) != static_cast<T>(0);
}
