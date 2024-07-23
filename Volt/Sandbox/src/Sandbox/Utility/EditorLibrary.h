#pragma once

#include <Volt/Asset/Asset.h>
#include <unordered_map>
#include <typeindex>

class EditorWindow;

class EditorLibrary
{
public:
	struct PanelInfo
	{
		Volt::AssetType assetType;
		std::type_index editorType;
		std::string category;

		Ref<EditorWindow> editorWindow;
	};

	static void Clear();
	static void Sort();
	static Ref<EditorWindow> GetPanel(const std::string& panelName);

	template<typename T, typename ...Args>
	static Ref<T> RegisterWithType(const std::string& category, Volt::AssetType assetType, Args&& ...args);

	template<typename T, typename ...Args>
	static Ref<T> Register(const std::string& category, Args&& ...args);
	
	static bool OpenAsset(Ref<Volt::Asset> asset);
	static Ref<EditorWindow> Get(Volt::AssetType type);

	template<typename T>
	static Ref<T> Get();

	inline static const Vector<PanelInfo>& GetPanels() { return s_editors; }

private:
	inline static Vector<PanelInfo> s_editors;
};

template<typename T, typename ...Args>
inline Ref<T> EditorLibrary::RegisterWithType(const std::string& category, Volt::AssetType assetType, Args && ...args)
{
	s_editors.emplace_back(assetType, typeid(T), category, CreateRef<T>(std::forward<Args>(args)...));
	return std::reinterpret_pointer_cast<T>(s_editors.back().editorWindow);
}

template<typename T, typename ...Args>
inline Ref<T> EditorLibrary::Register(const std::string& category, Args && ...args)
{
	s_editors.emplace_back(Volt::AssetType::None, typeid(T), category, CreateRef<T>(std::forward<Args>(args)...));
	return std::reinterpret_pointer_cast<T>(s_editors.back().editorWindow);
}

template<typename T>
inline Ref<T> EditorLibrary::Get()
{
	auto it = std::find_if(s_editors.begin(), s_editors.end(), [&](const auto& lhs) { return lhs.editorType == typeid(T); });
	if (it == s_editors.end())
	{
		VT_CORE_ERROR("Editor with type not registered!");
		return nullptr;
	}

	return std::reinterpret_pointer_cast<T>(it->editorWindow);
}
