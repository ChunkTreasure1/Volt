#pragma once

#include <string>
#include <algorithm>
#include <filesystem>
#include <codecvt>

namespace Utility
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

	inline const Vector<std::string> SplitStringsByCharacter(const std::string& src, const char character)
	{
		std::istringstream iss(src);

		Vector<std::string> result;
		std::string token;

		while (std::getline(iss, token, character))
		{
			if (!token.empty())
			{
				result.emplace_back(token);
			}
		}

		return result;
	}

	inline const std::string ReplaceCharacter(const std::string& src, const char oldCharacter, const char newCharacter)
	{
		std::string temp = src;
		std::replace(temp.begin(), temp.end(), oldCharacter, newCharacter);

		return temp;
	}

	inline const bool StringContains(const std::string& srcString, const std::string& token)
	{
		return srcString.find(token) != std::string::npos;
	}

	template<typename T, typename std::enable_if_t<std::is_integral<T>::value, bool> = true>
	inline const std::string ToStringWithMetricPrefixCharacterForBytes(const T aValue)
	{
		if (aValue > 1000000000000)
		{
			return std::format("{:.1f}", aValue / 1000000000000.f) + " TB";
		}
		else if (aValue > 1000000000)
		{
			return std::format("{:.1f}", aValue / 1000000000.f) + " GB";
		}
		else if (aValue > 1000000)
		{
			return std::format("{:.1f}", aValue / 1000000.f) + " MB";
		}
		else if (aValue > 1000)
		{
			return std::format("{: .1f}", aValue / 1000.f) + " kB";
		}
		else
		{
			return std::to_string(aValue) + " B";
		}
	}

	template<typename T, typename std::enable_if_t<std::is_integral<T>::value, bool> = true>
	inline const std::string ToStringWithThousandSeparator(const T aValue)
	{
		std::string result = "";

		std::string valueAsString = std::to_string(aValue);

		//traverse the string backwards
		int counter = 0;
		for (int32_t i = static_cast<int32_t>(valueAsString.size()) - 1; i >= 0; i--)
		{
			result.push_back(valueAsString[i]);
			counter++;
			if (counter >= 3 && i != 0)
			{
				counter = 0;
				result.push_back(',');
			}
		}
		std::reverse(result.begin(), result.end());

		return result;
	}

#pragma warning(disable : 4996)

	inline std::string ToString(std::wstring_view str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(str.data());
	}

	inline std::wstring ToWString(std::string_view str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter; //#TODO_Ivar: Might be taking a long time (over 0.1ms)
		return converter.from_bytes(str.data());
	}

	// From https://stackoverflow.com/questions/31302506/stdu32string-conversion-to-from-stdstring-and-stdu16string
	inline std::u32string To_UTF32(const std::string& s)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
		return conv.from_bytes(s);
	}

#pragma warning(default : 4996)
}
