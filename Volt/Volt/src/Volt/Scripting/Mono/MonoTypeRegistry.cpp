#include "vtpch.h"
#include "MonoTypeRegistry.h"

#include <entt.hpp>

namespace Volt
{
	template<typename T, AssetType assetType = AssetType::None, MonoTypeFlags typeFlags = MonoTypeFlags::None>
	void RegisterMonoType(const std::string& monoTypeName, std::unordered_map<std::string, MonoTypeInfo>& outTypeMap)
	{
		auto& newType = outTypeMap[monoTypeName];
		newType.typeIndex = std::type_index{ typeid(T) };
		newType.assetType = assetType;
		newType.typeFlags = typeFlags;
		newType.typeSize = sizeof(T);
		newType.typeName = std::string(monoTypeName);

		newType.equalFunc = [](const void* lhs, const void* rhs) -> bool
		{
			if (!lhs || !rhs)
			{
				return false;
			}

			return (*reinterpret_cast<const T*>(lhs)) == (*reinterpret_cast<const T*>(rhs));
		};
	}

	void MonoTypeRegistry::Initialize()
	{
		// System types
		RegisterMonoType<bool>("System.Boolean", s_monoToNativeTypeMap);
		RegisterMonoType<int64_t>("System.Int64", s_monoToNativeTypeMap);
		RegisterMonoType<uint64_t>("System.UInt64", s_monoToNativeTypeMap);
		RegisterMonoType<int32_t>("System.Int32", s_monoToNativeTypeMap);
		RegisterMonoType<uint32_t>("System.UInt32", s_monoToNativeTypeMap);
		RegisterMonoType<int16_t>("System.Int16", s_monoToNativeTypeMap);
		RegisterMonoType<uint16_t>("System.UInt16", s_monoToNativeTypeMap);
		RegisterMonoType<int8_t>("System.Char", s_monoToNativeTypeMap);
		RegisterMonoType<uint8_t>("System.Byte", s_monoToNativeTypeMap);
		RegisterMonoType<float>("System.Single", s_monoToNativeTypeMap);
		RegisterMonoType<double>("System.Double", s_monoToNativeTypeMap);
		RegisterMonoType<std::string>("System.String", s_monoToNativeTypeMap);

		// Volt types
		RegisterMonoType<glm::vec2>("Volt.Vector2", s_monoToNativeTypeMap);
		RegisterMonoType<glm::vec3>("Volt.Vector3", s_monoToNativeTypeMap);
		RegisterMonoType<glm::vec4>("Volt.Vector4", s_monoToNativeTypeMap);
		RegisterMonoType<glm::quat>("Volt.Quaternion", s_monoToNativeTypeMap);
		RegisterMonoType<entt::entity>("Volt.Entity", s_monoToNativeTypeMap);
		RegisterMonoType<glm::vec4, AssetType::None, MonoTypeFlags::Color>("Volt.Color", s_monoToNativeTypeMap);
		RegisterMonoType<AssetHandle, AssetType::Animation>("Volt.Animation", s_monoToNativeTypeMap);
		RegisterMonoType<AssetHandle, AssetType::Prefab>("Volt.Prefab", s_monoToNativeTypeMap);
		RegisterMonoType<AssetHandle, AssetType::Scene>("Volt.Scene", s_monoToNativeTypeMap);
		RegisterMonoType<AssetHandle, AssetType::Mesh>("Volt.Mesh", s_monoToNativeTypeMap);
		RegisterMonoType<AssetHandle, AssetType::Font>("Volt.Font", s_monoToNativeTypeMap);
		RegisterMonoType<AssetHandle, AssetType::Material>("Volt.Material", s_monoToNativeTypeMap);
		RegisterMonoType<AssetHandle, AssetType::Texture>("Volt.Texture", s_monoToNativeTypeMap);
		RegisterMonoType<AssetHandle, AssetType::PostProcessingMaterial>("Volt.PostProcessingMaterial", s_monoToNativeTypeMap);
		RegisterMonoType<AssetHandle, AssetType::Video>("Volt.Video", s_monoToNativeTypeMap);
		RegisterMonoType<AssetHandle, AssetType::AnimationGraph>("Volt.AnimationGraph", s_monoToNativeTypeMap);
	}

	const MonoTypeInfo MonoTypeRegistry::GetTypeInfo(std::string_view monoTypeName)
	{
		const std::string strTypeName = std::string(monoTypeName);

		if (!s_monoToNativeTypeMap.contains(strTypeName))
		{
			return {};
		}

		return s_monoToNativeTypeMap.at(strTypeName);
	}

	const MonoTypeInfo MonoTypeRegistry::GetTypeInfo(const std::type_index& typeIndex)
	{
		for (const auto& [monoTypeName, typeInfo] : s_monoToNativeTypeMap)
		{
			if (typeInfo.typeIndex == typeIndex)
			{
				return typeInfo;
			}
		}

		return {};
	}

	void MonoTypeRegistry::RegisterEnum(const std::string& typeName)
	{
		RegisterMonoType<int32_t, AssetType::None, MonoTypeFlags::Enum>(typeName, s_monoToNativeTypeMap);
	}
}
