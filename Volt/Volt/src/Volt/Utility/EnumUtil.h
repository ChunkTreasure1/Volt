#pragma once
#include "Volt/Core/Base.h"
#include <unordered_map>
namespace Utils
{
	/// <summary>
	/// Class for automatically generating enum to string and string to enum functions.
	/// </summary>
	class EnumUtil
	{
	public:
		static std::string ToString(std::string aEnumName, uint64_t aEnumValue)
		{
			return myRegistry[aEnumName][aEnumValue];
		}

		//to enum
		template<typename T>
		static T ToEnum(std::string aEnumName, std::string aEnumValue)
		{
			auto enumPair = myRegistry[aEnumName];
		}

		static bool RegisterEnum(const std::string& name, const std::string& definitionData)
		{
			//parse definition data into umap
			std::unordered_map<uint64_t, std::string> enumMap;

			const std::string allLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

			/*

			enum class Test : uint64_t
			{
				One = 10, Two = 1 << 2, Three = 0x1000000000010011
			}; inline static bool Test_reg = Utils::EnumUtil::RegisterEnum("Test", "One = 10, Two = 1 << 2, Three = 0x1000000000010011"); inline static std::string ToString(Test aEnumValue)
			{
				return Utils::EnumUtil::ToString("Test", static_cast<uint64_t>(aEnumValue));
			};

			*/
			size_t stringOffset = definitionData.find_first_of(allLetters);
			uint64_t highestValueSoFar = 0;
			while (stringOffset != std::string::npos)
			{
				const size_t memberNameEndOffset = definitionData.find_first_not_of(allLetters, stringOffset);

				std::string memberName = definitionData.substr(stringOffset, memberNameEndOffset - stringOffset);
				uint64_t memberValue = 0;

				VT_CORE_ASSERT(!memberName.empty(), "When Registering an enum, you must provide a name for each member");

				//find the comma
				size_t endOfMemberOffset = definitionData.find_first_of(',', memberNameEndOffset);
				if (endOfMemberOffset == std::string::npos)
				{
					//this is the last member so instead take the end of the string
					endOfMemberOffset = definitionData.size();
				}

				std::string memberValueString = definitionData.substr(memberNameEndOffset, endOfMemberOffset - memberNameEndOffset);

				//if the value string contains a = then we need to parse it else we just increment the highest value so far
				if (memberValueString.contains('='))
				{
					//remove whitespace
					memberValueString.erase(std::remove_if(memberValueString.begin(), memberValueString.end(), isspace), memberValueString.end());

					//remove the =
					memberValueString.erase(std::remove(memberValueString.begin(), memberValueString.end(), '='), memberValueString.end());

					//if the value is hex
					if (memberValueString.contains('x'))
					{
						memberValue = std::stoull(memberValueString, nullptr, 16);
					}
					//if the value is binary
					else if (memberValueString.contains('b'))
					{
						memberValue = std::stoull(memberValueString, nullptr, 2);
					}
					//if the value is bitshifted to the left
					else if (memberValueString.contains('<'))
					{
						//split string along "<<"
						const size_t bitshiftOffset = memberValueString.find_first_of('<');
						std::string base = memberValueString.substr(0, bitshiftOffset);
						std::string shift = memberValueString.substr(bitshiftOffset + 2, memberValueString.size() - bitshiftOffset - 2);
						
						memberValue = std::stoull(base, nullptr, 10) << std::stoull(shift, nullptr, 10);
					}
					//if the value is bitshifted to the right
					else if (memberValueString.contains('>'))
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
					memberValue = highestValueSoFar + 1;
				}
				
				highestValueSoFar = memberValue;

				enumMap[memberValue] = memberName;

				stringOffset = definitionData.find_first_of(allLetters, endOfMemberOffset);
			}


			myRegistry.insert({ name,  enumMap });


			//parse definition data

			//add to registry
		}



	private:
		//<EnumName, <EnumValue, EnumString>>
		static std::unordered_map<std::string, std::unordered_map<uint64_t, std::string>> myRegistry;
	};
}

#define CREATE_ENUM(enumName, type, ...)\
enum class enumName : type\
{\
	__VA_ARGS__\
};\
inline static bool enumName##_reg = Utils::EnumUtil::RegisterEnum(#enumName, #__VA_ARGS__); \
inline static std::string ToString(enumName aEnumValue) { return Utils::EnumUtil::ToString(#enumName, static_cast<uint64_t>(aEnumValue)); }

//implicit cast to enum
template<typename T>
inline T operator|(T aLeft, T aRight)
{
	return static_cast<T>(static_cast<uint64_t>(aLeft) | static_cast<uint64_t>(aRight));
}

enum class Test : uint64_t
{
	One = 10, Two, Three = 0x1000000000010011, Four
}; inline static bool Test_reg = Utils::EnumUtil::RegisterEnum("Test", "One = 10, Two = 1 << 2, Three = 0x1000000000010011"); inline static std::string ToString(Test aEnumValue)
{
	return Utils::EnumUtil::ToString("Test", static_cast<uint64_t>(aEnumValue));
};

/*
enum class Test : uint32_t {
	One, Two, Three
};inline static bool Test_reg = EnumUtil::RegisterEnum("Test", "One, Two, Three"); inline static std::string ToString(Test aEnumValue) {
	return EnumUtil::ToString("Test", static_cast<uint64_t>(aEnumValue));
}
*/
