#pragma once

#include "WireGUID.h"
#include "Registry.h"
#include "Destructor.h"
#include "ComponentPool.hpp"

#include <Volt/Asset/Asset.h>

#include <unordered_map>
#include <filesystem>
#include <any>

#define CREATE_COMPONENT_GUID(guid) inline static constexpr WireGUID comp_guid = guid;


#define UNPACK(...) __VA_ARGS__
#define SERIALIZE_COMPONENT(definition, type) UNPACK definition; \
inline static std::shared_ptr<Wire::ComponentPoolBase> Create##type() { return std::make_shared<Wire::ComponentPool<type>>(); } \
inline static bool type##_reg = Wire::ComponentRegistry::Register(#type, #definition, { type::comp_guid, sizeof(type), Create##type });

#define SERIALIZE_ENUM(definition, type) UNPACK definition; \
inline static bool type##_reg = Wire::ComponentRegistry::RegisterEnum(#type, #definition);

/*
* PROPERTY values:
* Serializable: true/false
* Visible: true/false
* Name: string
* Category: string
* SpecialType: string
*/
#define PROPERTY(...)

namespace Wire
{
	namespace Utility
	{
		inline std::string ToLower(const std::string& str)
		{
			std::string newStr(str);
			std::transform(str.begin(), str.end(), newStr.begin(), [](unsigned char c) { return (uint8_t)std::tolower((int32_t)c); });

			return newStr;
		}
	}

	struct SerializationData
	{
		struct EntityHeader
		{
			EntityId id;
			uint32_t componentCount;
			std::vector<WireGUID> componentGUIDs;
			std::vector<uint32_t> componentOffsets;
		};

		struct ComponentHeader
		{
			uint32_t componentStringSize;
			uint32_t componentSize;
			std::string componentString;
		};

		EntityHeader entityHeader;
		std::vector<ComponentHeader> componentHeaders;
	};

	class ComponentRegistry
	{
	public:
		enum class PropertyType : uint32_t
		{
			Bool = 0,
			Int = 1,
			UInt = 2,
			Short = 3,
			UShort = 4,
			Char = 5,
			UChar = 6,
			Float = 7,
			Double = 8,
			Vector2 = 9,
			Vector3 = 10,
			Vector4 = 11,
			String = 12,
			Unknown = 13,

			Int64 = 14,
			UInt64 = 15,
			AssetHandle = 16,
			Color3 = 17,
			Color4 = 18,
			Folder = 19,
			Path = 20,
			Vector = 21,
			EntityId = 22,
			GUID = 23,
			Enum = 24,
			Quaternion = 25
		};

		inline static const Volt::AssetType GetAssetTypeFromSting(const std::string& s)
		{
			static std::unordered_map<std::string, Volt::AssetType> assetsNameMap =
			{
				{ "MeshSource", Volt::AssetType::MeshSource },
				{ "Mesh", Volt::AssetType::Mesh },

				{ "Skeleton", Volt::AssetType::Skeleton },
				{ "Animation", Volt::AssetType::Animation },
				{ "AnimatedCharacter", Volt::AssetType::AnimatedCharacter },

				{ "Texture", Volt::AssetType::Texture },

				{ "Shader", Volt::AssetType::Shader },
				{ "ShaderSource", Volt::AssetType::ShaderSource },

				{ "Material", Volt::AssetType::Material },
				{ "PhysicsMaterial", Volt::AssetType::PhysicsMaterial },

				{ "Scene", Volt::AssetType::Scene },
				{ "Prefab", Volt::AssetType::Prefab },
				{ "ParticlePreset", Volt::AssetType::ParticlePreset },
				{ "Font", Volt::AssetType::Font },
				{ "Video", Volt::AssetType::Video },
				{ "RenderPipeline", Volt::AssetType::RenderPipeline }
			};

			for (const auto& [name, type] : assetsNameMap)
			{
				if (Utility::ToLower(name) == s)
				{
					return type;
				}
			}

			return Volt::AssetType::None;
		}

		inline static const size_t GetSizeFromType(PropertyType type)
		{
			switch (type)
			{
				case PropertyType::Bool: return sizeof(bool);
				case PropertyType::Int: return sizeof(int32_t);
				case PropertyType::UInt: return sizeof(uint32_t);
				case PropertyType::Short: return sizeof(int16_t);
				case PropertyType::UShort: return sizeof(uint16_t);
				case PropertyType::Char: return sizeof(int8_t);
				case PropertyType::UChar: return sizeof(uint8_t);
				case PropertyType::Float: return sizeof(float);
				case PropertyType::Double: return sizeof(double);
				case PropertyType::Vector2: return sizeof(float) * 2;
				case PropertyType::Vector3: return sizeof(float) * 3;
				case PropertyType::Vector4: return sizeof(float) * 4;
				case PropertyType::String: return sizeof(std::string);

				case PropertyType::Int64: return sizeof(int64_t);
				case PropertyType::UInt64: return sizeof(uint64_t);
				case PropertyType::AssetHandle: return sizeof(uint64_t);
				case PropertyType::Color3: return sizeof(float) * 3;
				case PropertyType::Color4: return sizeof(float) * 4;
				case PropertyType::Folder: return sizeof(std::filesystem::path);
				case PropertyType::Path: return sizeof(std::filesystem::path);
				case PropertyType::Vector: return sizeof(std::vector<uint8_t>);
				case PropertyType::EntityId: return sizeof(Wire::EntityId);
				case PropertyType::GUID: return sizeof(WireGUID);
				case PropertyType::Enum: return sizeof(uint32_t);
			}

			assert(type == PropertyType::Unknown && "Type is not supported!");
			return 0;
		}

		struct ComponentProperty
		{
			std::string name;
			std::string category;
			std::string enumName;

			bool serializable = true;
			bool visible = true;

			PropertyType type;
			PropertyType vectorType = PropertyType::Unknown;
			uint32_t offset = 0;
		
			// Used to hold special types, such as AssetType
			std::any specialType;
		};

		struct RegistrationInfo
		{
			using CreateMethod = std::shared_ptr<ComponentPoolBase>(*)();

			WireGUID guid = WireGUID::Null();
			size_t size = 0;

			CreateMethod poolCreateMethod;

			std::string name;
			std::string definitionData;
			std::vector<ComponentProperty> properties;
		};

		static bool Register(const std::string& name, const std::string& definitionData, const RegistrationInfo& guid);
		static bool RegisterEnum(const std::string& name, const std::string& definitionData);
		static const std::string GetNameFromGUID(const WireGUID& aGuid);
		static const RegistrationInfo& GetRegistryDataFromName(const std::string& aName);
		static const RegistrationInfo& GetRegistryDataFromGUID(const WireGUID& aGuid);

		static std::unordered_map<std::string, RegistrationInfo>& ComponentGUIDs();
		static std::unordered_map<std::string, std::vector<std::string>>& EnumData();

	private:
		struct PropertyInfo
		{
			PropertyType type;
			PropertyType vectorType;
			std::string enumName;
			std::string name;
		};
		
		ComponentRegistry() = delete;

		friend class Serializer;

		static void ParseDefinition(const std::string& definitionData, RegistrationInfo& outInfo);
		static void ParseEnumDefinition(const std::string& definitionData, std::vector<std::string>& members);

		static std::string FindValueInString(const std::string& srcString, const std::string& key);
		static std::unordered_map<std::string, size_t> GetMemberOffsetsFromString(const std::string& srcString);
		static PropertyInfo FindNameAndTypeFromString(const std::string& srcString);
	};

	class Serializer
	{
	public:
		static void SerializeEntityToFile(EntityId aId, const Registry& aRegistry, const std::filesystem::path& aSceneFolder);
		static EntityId DeserializeEntityToRegistry(const std::filesystem::path& aPath, Registry& aRegistry);

	private:
		Serializer() = delete;

		static bool ValidateComponentData(const std::string& definitionData, std::vector<uint8_t>& inOutData, WireGUID componentGUID);
	};
}