#include "vtpch.h"
#include "AnimationTreeImporter.h"

#include <yaml-cpp/yaml.h>

//bool Volt::AnimationTreeImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
//{
//
//	//if (!std::filesystem::exists(path)) [[unlikely]]
//	//{
//	//	VT_CORE_ERROR("File {0} not found!", path.string().c_str());
//	//	asset->SetFlag(AssetFlag::Missing, true);
//	//	return false;
//	//}
//
//	//std::ifstream file(path);
//	//if (!file.is_open()) [[unlikely]]
//	//{
//	//	VT_CORE_ERROR("Failed to open file: {0}!", path.string().c_str());
//	//	asset->SetFlag(AssetFlag::Invalid, true);
//	//	return false;
//	//}
//
//	//std::stringstream sstream;
//	//sstream << file.rdbuf();
//	//file.close();
//
//	//YAML::Node root;
//
//	//try
//	//{
//	//	root = YAML::Load(sstream.str());
//	//}
//	//catch (std::exception& e)
//	//{
//	//	VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", path, e.what());
//	//	asset->SetFlag(AssetFlag::Invalid, true);
//	//	return false;
//	//}
//
//
//
//
//    return false;
//}