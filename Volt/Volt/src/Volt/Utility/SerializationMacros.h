#pragma once

#include <vector>

#define VT_SERIALIZE_PROPERTY(propName, propVal, outputNode) outputNode << YAML::Key << #propName << YAML::Value << propVal
#define VT_SERIALIZE_PROPERTY_STRING(propName, propVal, outputNode) outputNode << YAML::Key << propName << YAML::Value << propVal

#define VT_DESERIALIZE_PROPERTY_ANY(propName, destination, node, defaultValue) destination = node[#propName] ? std::any_cast<decltype(defaultValue)>(node[#propName].as<decltype(defaultValue)>()) : defaultValue

#define VT_DESERIALIZE_PROPERTY(propName, destination, node, defaultValue) destination = node[#propName] ? node[#propName].as<decltype(defaultValue)>() : defaultValue
#define VT_DESERIALIZE_PROPERTY_STRING(propName, destination, node, defaultValue) destination = node[propName] ? node[propName].as<decltype(defaultValue)>() : defaultValue																			\