#include "Serialization.h"

#include <fstream>
#include <string>

namespace Wire
{
	namespace Utility
	{
		static ComponentRegistry::PropertyType PropertyFromString(const std::string& string)
		{
			if (string == "std::string") return ComponentRegistry::PropertyType::String;
			if (string == "bool") return ComponentRegistry::PropertyType::Bool;

			if (string == "int") return ComponentRegistry::PropertyType::Int;
			if (string == "int32_t") return ComponentRegistry::PropertyType::Int;

			if (string == "unsigned int") return ComponentRegistry::PropertyType::UInt; // TODO: Implement this
			if (string == "uint32_t") return ComponentRegistry::PropertyType::UInt;

			if (string == "short") return ComponentRegistry::PropertyType::Short;
			if (string == "int16_t") return ComponentRegistry::PropertyType::Short;

			if (string == "unsigned short") return ComponentRegistry::PropertyType::UShort; // TODO: Implement this
			if (string == "uint16_t") return ComponentRegistry::PropertyType::UShort;

			if (string == "char") return ComponentRegistry::PropertyType::Char;
			if (string == "int8_t") return ComponentRegistry::PropertyType::Char;

			if (string == "unsigned char") return ComponentRegistry::PropertyType::UChar; // TODO: Implement this
			if (string == "uint8_t") return ComponentRegistry::PropertyType::UChar;

			if (string == "float") return ComponentRegistry::PropertyType::Float;
			if (string == "double") return ComponentRegistry::PropertyType::Double;

			if (string == "gem::vec2") return ComponentRegistry::PropertyType::Vector2;
			if (string == "gem::vec3") return ComponentRegistry::PropertyType::Vector3;
			if (string == "gem::vec4") return ComponentRegistry::PropertyType::Vector4;
			if (string == "gem::quat") return ComponentRegistry::PropertyType::Quaternion;

			if (string == "int64_t") return ComponentRegistry::PropertyType::Int64;
			if (string == "uint64_t") return ComponentRegistry::PropertyType::UInt64;

			if (string == "AssetHandle") return ComponentRegistry::PropertyType::AssetHandle;
			if (string == "Volt::AssetHandle") return ComponentRegistry::PropertyType::AssetHandle;
			if (string == "std::filesystem::path") return ComponentRegistry::PropertyType::Path;

			if (string == "std::vector") return ComponentRegistry::PropertyType::Vector;
			if (string == "Wire::EntityId") return ComponentRegistry::PropertyType::EntityId;
			if (string == "WireGUID") return ComponentRegistry::PropertyType::GUID;

			return ComponentRegistry::PropertyType::Unknown;
		}
	}

	bool ComponentRegistry::Register(const std::string& name, const std::string& definitionData, const RegistrationInfo& guid)
	{
		if (auto it = ComponentGUIDs().find(name); it == ComponentGUIDs().end())
		{
			ComponentGUIDs()[name] = guid;
			ComponentGUIDs()[name].name = name;

			ComponentGUIDs()[name].definitionData = definitionData;

			ParseDefinition(definitionData, ComponentGUIDs()[name]);
			return true;
		}

		return false;
	}

	bool ComponentRegistry::RegisterEnum(const std::string& name, const std::string& definitionData)
	{
		if (auto it = EnumData().find(name); it != EnumData().end())
		{
			return false;
		}

		ParseEnumDefinition(definitionData, EnumData()[name]);

		return true;
	}

	const std::string ComponentRegistry::GetNameFromGUID(const WireGUID& aGuid)
	{
		for (const auto& it : ComponentGUIDs())
		{
			if (it.second.guid == aGuid)
			{
				return it.first;
			}
		}

		return "Null";
	}

	const ComponentRegistry::RegistrationInfo& ComponentRegistry::GetRegistryDataFromName(const std::string& aName)
	{
		if (auto it = ComponentGUIDs().find(aName); it != ComponentGUIDs().end())
		{
			return it->second;
		}

		static RegistrationInfo empty;
		return empty;
	}

	const ComponentRegistry::RegistrationInfo& ComponentRegistry::GetRegistryDataFromGUID(const WireGUID& aGuid)
	{
		for (const auto& [name, info] : ComponentGUIDs())
		{
			if (info.guid == aGuid)
			{
				return info;
			}
		}

		static RegistrationInfo empty;
		return empty;
	}

	std::unordered_map<std::string, ComponentRegistry::RegistrationInfo>& ComponentRegistry::ComponentGUIDs()
	{
		static std::unordered_map<std::string, RegistrationInfo> impl;
		return impl;
	}

	std::unordered_map<std::string, std::vector<std::string>>& ComponentRegistry::EnumData()
	{
		static std::unordered_map<std::string, std::vector<std::string>> impl;
		return impl;
	}

	void ComponentRegistry::ParseDefinition(const std::string& definitionData, RegistrationInfo& outInfo)
	{
		constexpr size_t propertyTextLength = 9;

		auto propertyOffsets = GetMemberOffsetsFromString(definitionData);

		// Find first {, which is the start of the component
		size_t offset = definitionData.find_first_of('{');
		offset = definitionData.find("PROPERTY(", offset);

		while (offset != std::string::npos)
		{
			const size_t propertyBegin = offset;
			const size_t propertyEnd = definitionData.find(')', offset);

			if (propertyBegin == std::string::npos)
			{
				break;
			}

			std::string propertySubStr = definitionData.substr(offset + propertyTextLength, propertyEnd - (propertyBegin + propertyTextLength));

			// Handle property options
			{
				std::string propertyName;
				std::string propertyCategory;
				std::string propertySpecialType;
				std::string propertyEnumName;

				bool propertySerializable = true;
				bool propertyVisible = true;

				uint32_t propertyMemOffset = 0;
				PropertyType propertyType;
				PropertyType vectorType = PropertyType::Unknown;
				std::any specialTypeValue;

				propertyName = FindValueInString(propertySubStr, "Name");
				propertyCategory = FindValueInString(propertySubStr, "Category");
				propertySpecialType = Utility::ToLower(FindValueInString(propertySubStr, "SpecialType"));

				{
					std::string value = Utility::ToLower(FindValueInString(propertySubStr, "Serializable"));
					propertySerializable = value == "false" ? false : true;
				}

				{
					std::string value = Utility::ToLower(FindValueInString(propertySubStr, "Visible"));
					propertyVisible = value == "false" ? false : true;
				}

				// Find property name and get it's offset
				{
					const size_t propertyOffset = definitionData.find_first_of(';', propertyBegin);
					std::string propertyString = definitionData.substr(propertyBegin, propertyOffset - propertyBegin);

					const auto propInfo = FindNameAndTypeFromString(propertyString);
					if (propertyName.empty())
					{
						propertyName = propInfo.name;
						propertyName[0] = toupper(propertyName[0]);
					}

					propertyMemOffset = (uint32_t)propertyOffsets[propInfo.name];
					propertyType = propInfo.type;
					vectorType = propInfo.vectorType;

					if (propertySpecialType == "enum")
					{
						propertyType = PropertyType::Enum;

						const size_t propertyEnd = propertyString.find_first_of(')');
						const size_t firstLetter = propertyString.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890", propertyEnd);
						const size_t lastLetter = propertyString.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890", firstLetter);

						propertyEnumName = propertyString.substr(firstLetter, lastLetter - firstLetter);
					}

					if (propertySpecialType == "asset")
					{
						std::string assetType = Utility::ToLower(FindValueInString(propertySubStr, "AssetType"));
						if (!assetType.empty())
						{
							specialTypeValue = GetAssetTypeFromSting(assetType);
						}
					}
				}


				if (propertyType != PropertyType::Unknown)
				{
					auto& propertyValues = outInfo.properties.emplace_back();
					propertyValues.category = propertyCategory;
					propertyValues.name = propertyName;
					propertyValues.offset = propertyMemOffset;
					propertyValues.serializable = propertySerializable;
					propertyValues.visible = propertyVisible;
					propertyValues.type = propertyType;
					propertyValues.vectorType = vectorType;
					propertyValues.enumName = propertyEnumName;
					propertyValues.specialType = specialTypeValue;

					if (propertySpecialType == "color" && (propertyType == PropertyType::Vector3 || propertyType == PropertyType::Vector4))
					{
						propertyValues.type = propertyType == PropertyType::Vector3 ? PropertyType::Color3 : PropertyType::Color4;
					}
					else if (propertySpecialType == "folder")
					{
						propertyValues.type = PropertyType::Folder;
					}
				}
			}

			offset = definitionData.find("PROPERTY(", propertyEnd);
		}
	}

	void ComponentRegistry::ParseEnumDefinition(const std::string& definitionData, std::vector<std::string>& outMembers)
	{
		size_t stringOffset = definitionData.find_first_of('{');

		while (stringOffset != std::string::npos)
		{
			const size_t nextMemberEndOffset = definitionData.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", stringOffset);

			std::string memberName = definitionData.substr(stringOffset, nextMemberEndOffset - stringOffset);
			if (!memberName.empty())
			{
				outMembers.emplace_back(memberName);
			}
			stringOffset = definitionData.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", nextMemberEndOffset);
		}
	}

	std::string ComponentRegistry::FindValueInString(const std::string& srcString, const std::string& key)
	{
		std::string result;
		std::string lowerKey = Utility::ToLower(key);

		if (size_t offset = srcString.find(key); offset != std::string::npos)
		{
			const size_t nextPropertyOffset = srcString.find(",", offset);
			const size_t propertyLength = (nextPropertyOffset != std::string::npos ? nextPropertyOffset : srcString.size()) - offset;

			std::string propertySubStr = srcString.substr(offset, propertyLength);

			const size_t equalSignOffset = propertySubStr.find("=");
			const size_t firstLetter = propertySubStr.find_first_not_of(" =", equalSignOffset);

			result = propertySubStr.substr(firstLetter, propertySubStr.size() - firstLetter);
		}

		return result;
	}

	std::unordered_map<std::string, size_t> ComponentRegistry::GetMemberOffsetsFromString(const std::string& srcString)
	{
		std::unordered_map<std::string, size_t> properties;
		size_t memoryOffset = 0;
		size_t stringOffset = srcString.find_first_of('{');

		while (stringOffset != std::string::npos)
		{
			const size_t propertyOffset = srcString.find_first_of(';', stringOffset);
			std::string propertyString = srcString.substr(stringOffset, propertyOffset - stringOffset);

			const auto propInfo = FindNameAndTypeFromString(propertyString);

			properties.emplace(propInfo.name, memoryOffset);

			memoryOffset += GetSizeFromType(propInfo.type);
			stringOffset = srcString.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890", propertyOffset);

			// Check for GUID definition
			{
				size_t nextSemiColon = srcString.find_first_of(';', stringOffset);
				if (nextSemiColon != std::string::npos)
				{
					std::string guidSubString = srcString.substr(stringOffset, nextSemiColon - stringOffset);
					if (guidSubString.find("CREATE_COMPONENT_GUID(") != std::string::npos)
					{
						stringOffset = srcString.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890", nextSemiColon);
					}
				}
			}
		}

		return properties;
	}

	ComponentRegistry::PropertyInfo ComponentRegistry::FindNameAndTypeFromString(const std::string& srcString)
	{
		bool hasProperty = srcString.find("PROPERTY(") != std::string::npos;



		size_t propDefEnd = srcString.find_first_of(')');
		const std::string propertySpecialType = Utility::ToLower(FindValueInString(srcString.substr(0, propDefEnd), "SpecialType"));

		if (propDefEnd == std::string::npos || !hasProperty)
		{
			propDefEnd = 0;
		}

		PropertyType type;
		PropertyType vectorType = PropertyType::Unknown;
		std::string name;

		size_t nameOffset = 0;

		{
			const size_t nextLetter = srcString.find_first_not_of(' ', propDefEnd + 1);
			const size_t nextSpace = srcString.find_first_of(' ', nextLetter);
			const size_t nextLessThan = srcString.find_first_of('<', nextLetter);

			size_t nextCharacter = nextSpace;

			if (nextLessThan < nextSpace)
			{
				nextCharacter = nextLessThan;
			}

			const std::string typeString = srcString.substr(nextLetter, nextCharacter - nextLetter);

			if (propertySpecialType == "enum")
			{
				type = PropertyType::Enum;
			}
			else
			{
				type = Utility::PropertyFromString(typeString);
			}

			if (type == PropertyType::Vector)
			{
				const size_t nextGreaterThan = srcString.find_first_of('>', nextLessThan);
				const std::string vectorTypeString = srcString.substr(nextLessThan + 1, nextGreaterThan - nextLessThan - 1);

				vectorType = Utility::PropertyFromString(vectorTypeString);
			}

			nameOffset = nextSpace;
		}

		{
			size_t nextLetter = srcString.find_first_not_of(' ', nameOffset);
			if (nextLetter == std::string::npos)
			{
				nextLetter = srcString.length();
			}

			size_t endOfName = srcString.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890", nextLetter);
			if (endOfName == std::string::npos)
			{
				endOfName = srcString.length();
			}

			name = srcString.substr(nextLetter, endOfName - nextLetter + 1);
		}

		PropertyInfo info{};
		info.name = name;
		info.type = type;
		info.vectorType = vectorType;

		return info;
	}

	void Serializer::SerializeEntityToFile(EntityId aId, const Registry& aRegistry, const std::filesystem::path& aSceneFolder)
	{
		//SerializationData serializationData{};
		//serializationData.entityHeader.id = aId;

		//// Get component guids, and strings
		//for (const auto& [guid, pool] : aRegistry.m_pools)
		//{
		//	if (pool.HasComponent(aId))
		//	{
		//		const auto& info = ComponentRegistry::GetRegistryDataFromGUID(guid);
		//		serializationData.entityHeader.componentGUIDs.emplace_back(guid);
		//		auto& componentHeader = serializationData.componentHeaders.emplace_back();

		//		componentHeader.componentString = info.definitionData;
		//		componentHeader.componentStringSize = info.definitionData.size();

		//		componentHeader.componentSize = info.size;

		//		serializationData.entityHeader.componentCount++;
		//	}
		//}

		//// Start component byte collection
		//std::vector<uint8_t> outComponentData;

		//{
		//	size_t offset = 0;

		//	uint32_t index = 0;
		//	for (const auto& guid : serializationData.entityHeader.componentGUIDs)
		//	{
		//		// Add offset
		//		serializationData.entityHeader.componentOffsets.emplace_back(outComponentData.size());

		//		const auto& pool = aRegistry.m_pools.at(guid);
		//		const auto& componentHeader = serializationData.componentHeaders.at(index);

		//		const auto componentData = pool.GetComponentData(aId);

		//		// String size
		//		{
		//			outComponentData.resize(outComponentData.size() + sizeof(uint32_t));
		//			memcpy_s(&outComponentData[offset], sizeof(uint32_t), &componentHeader.componentStringSize, sizeof(uint32_t));
		//			offset += sizeof(uint32_t);
		//		}

		//		// Size
		//		{
		//			outComponentData.resize(outComponentData.size() + sizeof(uint32_t));
		//			memcpy_s(&outComponentData[offset], sizeof(uint32_t), &componentHeader.componentSize, sizeof(uint32_t));
		//			offset += sizeof(uint32_t);
		//		}

		//		// String
		//		{
		//			outComponentData.resize(outComponentData.size() + componentHeader.componentStringSize);
		//			memcpy_s(&outComponentData[offset], componentHeader.componentStringSize, componentHeader.componentString.data(), componentHeader.componentStringSize);
		//			offset += componentHeader.componentStringSize;
		//		}

		//		// Data
		//		{
		//			outComponentData.resize(outComponentData.size() + componentHeader.componentSize);
		//			memcpy_s(&outComponentData[offset], componentHeader.componentSize, componentData.data(), componentData.size());
		//			offset += componentData.size();
		//		}

		//		index++;
		//	}
		//}

		//std::vector<uint8_t> outData;
		//size_t offset = 0;

		//// Entity header
		//{
		//	outData.resize(outData.size() + sizeof(EntityId));
		//	memcpy_s(&outData[offset], sizeof(EntityId), &serializationData.entityHeader.id, sizeof(EntityId));
		//	offset += sizeof(EntityId);

		//	outData.resize(outData.size() + sizeof(uint32_t));
		//	memcpy_s(&outData[offset], sizeof(uint32_t), &serializationData.entityHeader.componentCount, sizeof(uint32_t));
		//	offset += sizeof(uint32_t);

		//	for (const auto& guid : serializationData.entityHeader.componentGUIDs)
		//	{
		//		outData.resize(outData.size() + sizeof(WireGUID));
		//		memcpy_s(&outData[offset], sizeof(WireGUID), &guid, sizeof(WireGUID));
		//		offset += sizeof(WireGUID);
		//	}

		//	for (const auto& componentOffset : serializationData.entityHeader.componentOffsets)
		//	{
		//		outData.resize(outData.size() + sizeof(uint32_t));
		//		memcpy_s(&outData[offset], sizeof(uint32_t), &componentOffset, sizeof(uint32_t));
		//		offset += sizeof(uint32_t);
		//	}
		//}

		//// Component data
		//{
		//	outData.resize(outData.size() + outComponentData.size());
		//	memcpy_s(&outData[offset], outComponentData.size(), outComponentData.data(), outComponentData.size());
		//}

		//std::ofstream output(aSceneFolder / ("ent_" + std::to_string(aId) + ".ent"), std::ios::binary | std::ios::out);
		//output.write(reinterpret_cast<const char*>(outData.data()), outData.size());
		//output.close();
	}

	EntityId Serializer::DeserializeEntityToRegistry(const std::filesystem::path& aPath, Registry& aRegistry)
	{
		//std::ifstream file(aPath.string(), std::ios::binary | std::ios::in);
		//if (!file.is_open())
		//{
		//	return Wire::NullID;
		//}

		//std::vector<uint8_t> totalData;
		//totalData.resize(file.seekg(0, std::ios::end).tellg());
		//file.seekg(0, std::ios::beg);
		//file.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
		//file.close();

		//SerializationData serializationData{};
		//size_t offset = 0;

		//// Entity header
		//{
		//	serializationData.entityHeader.id = *(EntityId*)&totalData[offset];
		//	offset += sizeof(EntityId);

		//	serializationData.entityHeader.componentCount = *(uint32_t*)&totalData[offset];
		//	offset += sizeof(uint32_t);

		//	for (uint32_t i = 0; i < serializationData.entityHeader.componentCount; i++)
		//	{
		//		serializationData.entityHeader.componentGUIDs.emplace_back(*(WireGUID*)&totalData[offset]);
		//		offset += sizeof(WireGUID);
		//	}

		//	for (uint32_t i = 0; i < serializationData.entityHeader.componentCount; i++)
		//	{
		//		serializationData.entityHeader.componentOffsets.emplace_back(*(uint32_t*)&totalData[offset]);
		//		offset += sizeof(uint32_t);
		//	}
		//}

		//aRegistry.AddEntity(serializationData.entityHeader.id);

		//// Components
		//{
		//	for (uint32_t i = 0; i < serializationData.entityHeader.componentCount; i++)
		//	{
		//		const uint32_t componentOffset = serializationData.entityHeader.componentOffsets[i] + offset;
		//		const WireGUID guid = serializationData.entityHeader.componentGUIDs[i];
		//		auto& componentHeader = serializationData.componentHeaders.emplace_back();

		//		uint32_t internalOffset = 0;

		//		const uint32_t stringSize = *(uint32_t*)&totalData[componentOffset + internalOffset];
		//		internalOffset += sizeof(uint32_t);

		//		const uint32_t size = *(uint32_t*)&totalData[componentOffset + internalOffset];
		//		internalOffset += sizeof(uint32_t);

		//		std::string componentString;
		//		componentString.resize(stringSize);
		//		memcpy_s(componentString.data(), stringSize, &totalData[componentOffset + internalOffset], stringSize);
		//		internalOffset += stringSize;

		//		// Component data
		//		std::vector<uint8_t> componentData;
		//		componentData.resize(size);
		//		memcpy_s(componentData.data(), size, &totalData[componentOffset + internalOffset], size);

		//		if (ValidateComponentData(componentString, componentData, guid))
		//		{
		//			aRegistry.AddComponent(componentData, guid, serializationData.entityHeader.id);
		//		}
		//	}
		//}

		//return serializationData.entityHeader.id;

		return 0;
	}

	bool Serializer::ValidateComponentData(const std::string& definitionData, std::vector<uint8_t>& inOutData, WireGUID componentGUID)
	{
		auto it = std::find_if(ComponentRegistry::ComponentGUIDs().begin(), ComponentRegistry::ComponentGUIDs().end(), [&componentGUID](const std::pair<std::string, ComponentRegistry::RegistrationInfo>& pair)
			{
				return pair.second.guid == componentGUID;
			});

		if (it == ComponentRegistry::ComponentGUIDs().end())
		{
			return false;
		}

		ComponentRegistry::RegistrationInfo loadedInfo{};
		loadedInfo.definitionData = definitionData;
		loadedInfo.guid = componentGUID;

		ComponentRegistry::ParseDefinition(definitionData, loadedInfo);
		const ComponentRegistry::RegistrationInfo& existingInfo = ComponentRegistry::GetRegistryDataFromGUID(componentGUID);

		const size_t loadedComponentSize = inOutData.size();
		std::vector<uint8_t> tempCompStorage;
		tempCompStorage.resize(existingInfo.size);

		for (uint32_t i = 0; i < (uint32_t)loadedInfo.properties.size(); i++)
		{
			const auto& loadedProperty = loadedInfo.properties.at(i);

			auto existingPropertyIt = std::find_if(existingInfo.properties.begin(), existingInfo.properties.end(), [&loadedProperty](const ComponentRegistry::ComponentProperty& existingProp)
				{
					return loadedProperty.name == existingProp.name && loadedProperty.type == existingProp.type; // if it's not the same type anymore, skip serialization
				});

			if (existingPropertyIt != existingInfo.properties.end())
			{
				// Move property from in data vector to temp vector
				const size_t propSize = ComponentRegistry::GetSizeFromType(existingPropertyIt->type);
				memmove_s(&tempCompStorage[existingPropertyIt->offset], propSize, &inOutData[loadedProperty.offset], propSize);
			}
		}

		inOutData = tempCompStorage;
		return true;
	}
}
