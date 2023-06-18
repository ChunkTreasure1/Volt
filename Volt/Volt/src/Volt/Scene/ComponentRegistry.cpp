#include "vtpch.h"
#include "ComponentRegistry.h"

#include "EnTTTesting.h"

#include "Volt/Utility/StringUtility.h"

namespace Volt
{
	inline static SpecialType SpecialTypeFromString(std::string_view value)
	{
		if (value == "est_none") return eST_None;
		if (value == "est_color") return eST_Color;
		if (value == "est_asset") return eST_Asset;
		if (value == "est_directory") return eST_Directory;

		return eST_None;
	}

	bool ComponentRegistry::RegisterPropertyToComponent(std::string_view componentName, std::string_view propertyName, std::string_view propertyString)
	{
		if (!s_componentInfo.contains(componentName))
		{
			return false;
		}

		auto& info = s_componentInfo.at(componentName);

		auto& newProperty = info.properties.emplace_back();
		newProperty.name = propertyName;

		ParsePropertyInfo(propertyString, newProperty);
		return true;
	}

	void ComponentRegistry::ParsePropertyInfo(std::string_view propertyString, ComponentProperty& outProperty)
	{
		const auto propertyValues = Utility::SplitStringsByCharacter(std::string(propertyString), ',');

		for (const auto& val : propertyValues)
		{
			const auto valLowerCase = Utility::ToLower(val);

			if (Utility::StringContains(valLowerCase, SPECIAL_TYPE_NAME))
			{
				ParseSpecialType(valLowerCase, propertyValues, outProperty);
			}
			else if (Utility::StringContains(valLowerCase, VISIBLE_NAME))
			{
				if (Utility::StringContains(valLowerCase, "true"))
				{
					outProperty.visible = eV_True;
				}
				else
				{
					outProperty.visible = eV_False;
				}
			}
			else if (Utility::StringContains(valLowerCase, SERIALIZABLE_NAME))
			{
				if (Utility::StringContains(valLowerCase, "true"))
				{
					outProperty.serializable = eS_True;
				}
				else
				{
					outProperty.serializable = eS_False;
				}
			}
		}
	}

	void ComponentRegistry::ParseSpecialType(std::string_view specialTypeString, const std::vector<std::string>& propertyValues, ComponentProperty& outProperty)
	{
		std::string specialTypeValue = FindValueInString(specialTypeString, SPECIAL_TYPE_NAME);
		SpecialType specType = SpecialTypeFromString(specialTypeValue);

		outProperty.specialType = specType;

		if (specType == eST_Asset)
		{
			for (const auto& propVal : propertyValues)
			{
				std::string propValLower = Utility::ToLower(propVal);

				if (Utility::StringContains(propValLower, ASSET_TYPE_NAME))
				{
					const std::string assetTypeVal = Utility::ToLower(FindValueInString(propVal, ASSET_TYPE_NAME));
				
					for (const auto& [name, assetType] : GetAssetNames())
					{
						std::string correctedName = Utility::ToLower(name);
						correctedName.erase(std::remove_if(correctedName.begin(), correctedName.end(), ::isspace), correctedName.end());

						if (assetTypeVal == name)
						{
							outProperty.assetType = assetType;
						}
					}
				}
			}
		}
	}

	std::string ComponentRegistry::FindValueInString(std::string_view srcString, std::string_view key)
	{
		std::string result;
		std::string lowerKey = Utility::ToLower(std::string(key));
		std::string srcLower = Utility::ToLower(std::string(srcString));

		if (size_t offset = srcLower.find(key); offset != std::string::npos)
		{
			const size_t nextPropertyOffset = srcLower.find(",", offset);
			const size_t propertyLength = (nextPropertyOffset != std::string::npos ? nextPropertyOffset : srcLower.size()) - offset;

			std::string propertySubStr = std::string(srcLower).substr(offset, propertyLength);

			const size_t equalSignOffset = propertySubStr.find('=');
			const size_t firstLetter = propertySubStr.find_first_not_of(" =", equalSignOffset);

			result = propertySubStr.substr(firstLetter, propertySubStr.size() - firstLetter);
		}

		return result;
	}
}
