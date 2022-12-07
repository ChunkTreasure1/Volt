#pragma once

#include <string>
#include <algorithm>
#include <filesystem>
#include <codecvt>

namespace Utils
{
	inline std::string ToLower(const std::string& str)
	{
		std::string newStr(str);
		std::transform(str.begin(), str.end(), newStr.begin(), [](unsigned char c) { return (uint8_t)std::tolower((int32_t)c); });

		return newStr;
	}

	inline std::string RemoveTrailingZeroes(const std::string& string)
	{
		std::string newStr = string;
		newStr.erase(newStr.find_last_not_of('0') + 1, std::string::npos);

		if (!newStr.empty() && newStr.back() == '.')
		{
			newStr.pop_back();
		}

		return newStr;
	}

#pragma warning(disable : 4996)

	// From https://stackoverflow.com/questions/31302506/stdu32string-conversion-to-from-stdstring-and-stdu16string
	inline std::u32string To_UTF32(const std::string& s)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
		return conv.from_bytes(s);
	}

#pragma warning(default : 4996)
}