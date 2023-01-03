#pragma once

#include <Volt/Asset/Asset.h>
#include <unordered_map>

class EditorWindow;

class EditorLibrary
{
public:
	static void Clear();

	static void Register(Volt::AssetType type, Ref<EditorWindow> editor);
	static void OpenAsset(Ref<Volt::Asset> asset);
	static Ref<EditorWindow> Get(Volt::AssetType type);

private:
	inline static std::unordered_map<Volt::AssetType, Ref<EditorWindow>> s_editors;
};